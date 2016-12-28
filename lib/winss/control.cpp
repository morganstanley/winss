#include "control.hpp"
#include <vector>
#include <set>
#include "pipe_client.hpp"
#include "wait_multiplexer.hpp"
#include "not_owning_ptr.hpp"

winss::ControlItem::ControlItem(std::string name) : name(name) {}

const std::string&winss::ControlItem::GetName() const {
    return name;
}

const int winss::Control::kTimeoutExitCode = 1;
const char winss::Control::kTimeoutGroup[] = "control";

winss::Control::Control(
    winss::NotOwningPtr<winss::WaitMultiplexer> multiplexer,
    DWORD timeout, bool finish_all) : multiplexer(multiplexer),
    timeout(timeout), finish_all(finish_all) {
    if (timeout != INFINITE) {
        multiplexer->AddInitCallback([timeout](winss::WaitMultiplexer& m) {
            m.AddTimeoutCallback(timeout, [](
                winss::WaitMultiplexer& m) {
                VLOG(3) << "Control timeout!";
                m.Stop(winss::Control::kTimeoutExitCode);
            }, kTimeoutGroup);
        });

        multiplexer->AddStopCallback([timeout](winss::WaitMultiplexer& m) {
            m.RemoveTimeoutCallback(kTimeoutGroup);
        });
    }
}

bool winss::Control::IsStarted() const {
    return started;
}

void winss::Control::Add(winss::NotOwningPtr<winss::ControlItem> item) {
    items.emplace(item->GetName(), item);
    multiplexer->AddInitCallback([item](winss::WaitMultiplexer&) {
        item->Init();
    });
}

void winss::Control::Ready(std::string name) {
    if (started) {
        return;
    }

    auto it = items.find(name);
    if (it != items.end()) {
        ready.insert(name);

        if (items.size() == ready.size()) {
            started = true;
            for (auto& kv : items) {
                kv.second->Start();
            }
        }
    }
}

void winss::Control::Remove(std::string name) {
    if (items.erase(name) && (items.empty() || !finish_all)) {
        multiplexer->Stop(0);
    }
}

int winss::Control::Start() {
    if (items.empty()) {
        return 0;
    }

    return multiplexer->Start();
}

const int winss::OutboundControlItem::kNoSendExitCode = 111;

winss::OutboundControlItem::OutboundControlItem(
    winss::NotOwningPtr<WaitMultiplexer> multiplexer,
    winss::NotOwningPtr<winss::Control> control,
    winss::NotOwningPtr<winss::OutboundPipeClient> client,
    const std::vector<char>& commands, std::string name) :
    multiplexer(multiplexer), control(control),
    client(client), commands(commands),
    winss::ControlItem::ControlItem("out_" + name) {
    client->AddListener(winss::NotOwned(this));

    if (!commands.empty()) {
        control->Add(winss::NotOwned(this));
    }
}

void winss::OutboundControlItem::Init() {
    client->Connect();
}

void winss::OutboundControlItem::Start() {
    client->Send(commands);
}

bool winss::OutboundControlItem::Completed() const {
    return written;
}

bool winss::OutboundControlItem::Connected() {
    control->Ready(name);
    return true;
}

bool winss::OutboundControlItem::WriteComplete() {
    VLOG(3) << "Completed sending commands";
    written = true;
    client->Stop();
    return true;
}

bool winss::OutboundControlItem::Disconnected() {
    if (!written) {
        multiplexer->Stop(kNoSendExitCode);
    } else {
        control->Remove(name);
    }
    return true;
}

winss::InboundControlItem::InboundControlItem(
    winss::NotOwningPtr<WaitMultiplexer> multiplexer,
    winss::NotOwningPtr<Control> control,
    winss::NotOwningPtr<winss::InboundPipeClient> client,
    winss::NotOwningPtr<InboundControlItemListener> listener,
    std::string name) : multiplexer(multiplexer),
    control(control), client(client), listener(listener),
    winss::ControlItem::ControlItem("in_" + name) {
    client->AddListener(winss::NotOwned(this));

    if (listener->IsEnabled()) {
        control->Add(winss::NotOwned(this));
    }
}

void winss::InboundControlItem::Init() {
    client->Connect();
}

void winss::InboundControlItem::Start() {
    if (!listener->CanStart()) {
        complete = true;
        client->Stop();
    }
}

bool winss::InboundControlItem::Completed() const {
    return complete;
}

bool winss::InboundControlItem::Connected() {
    listener->HandleConnected();
    control->Ready(name);
    return true;
}

bool winss::InboundControlItem::Recieved(const std::vector<char>& message) {
    if (!listener->HandleRecieved(message)) {
        complete = true;
        client->Stop();
    }

    return true;
}

bool winss::InboundControlItem::Disconnected() {
    control->Remove(name);
    return true;
}
