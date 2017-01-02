#include "wait_multiplexer.hpp"
#include <windows.h>
#include <chrono>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <utility>
#include <functional>
#include "easylogging/easylogging++.hpp"
#include "windows_interface.hpp"
#include "handle_wrapper.hpp"
#include "event_wrapper.hpp"
#include "map_value_iterator.hpp"

bool winss::WaitTimeoutItem::operator<(const winss::WaitTimeoutItem& rhs)
    const {
    return timeout < rhs.timeout;
}

void winss::WaitMultiplexer::AddInitCallback(winss::Callback callback) {
    if (callback) {
        init_callbacks.push_back(callback);
    }
}

void winss::WaitMultiplexer::AddTriggeredCallback(
    const winss::HandleWrapper& handle, winss::TriggeredCallback callback) {
    if (handle.HasHandle() && callback) {
        trigger_callbacks.emplace(handle, callback);
    }
}

void winss::WaitMultiplexer::AddTimeoutCallback(DWORD timeout,
    winss::Callback callback, std::string group) {
    if (callback && timeout != INFINITE) {
        auto now = std::chrono::system_clock::now();
        auto timeout_time = now + std::chrono::milliseconds(timeout);
        timeout_callbacks.insert(std::move(
            winss::WaitTimeoutItem{ group, timeout_time, callback }));
    }
}

void winss::WaitMultiplexer::AddStopCallback(winss::Callback callback) {
    if (callback) {
        stop_callbacks.push_back(callback);
    }
}

bool winss::WaitMultiplexer::RemoveTriggeredCallback(
    const winss::HandleWrapper& handle) {
    auto mapping_it = trigger_callbacks.find(handle);
    if (mapping_it != trigger_callbacks.end()) {
        trigger_callbacks.erase(mapping_it);
        return true;
    }

    return false;
}

bool winss::WaitMultiplexer::RemoveTimeoutCallback(std::string group) {
    bool found = false;

    auto it = timeout_callbacks.begin();
    while (it != timeout_callbacks.end()) {
        if (it->group == group) {
            it = timeout_callbacks.erase(it);
            found = true;
        } else {
            ++it;
        }
    }

    return found;
}

DWORD winss::WaitMultiplexer::GetTimeout() const {
    auto it = timeout_callbacks.begin();
    if (it != timeout_callbacks.end()) {
        auto now = std::chrono::system_clock::now();
        auto timeout = std::chrono::duration_cast<std::chrono::milliseconds>(
            it->timeout - now);

        auto timeout_count = timeout.count();
        if (timeout_count > 0) {
            VLOG(7) << "Using timeout of " << timeout_count;
            return (DWORD) timeout_count;
        } else {
            return 0;
        }
    }

    return INFINITE;
}

winss::Callback winss::WaitMultiplexer::GetTimeoutCallback() {
    Callback func;

    auto it = timeout_callbacks.begin();
    if (it != timeout_callbacks.end()) {
        func = std::move(it->callback);
        timeout_callbacks.erase(it);
    }

    return func;
}

int winss::WaitMultiplexer::Start() {
    if (started || stopping) {
        return return_code;
    }

    VLOG(7) << "Starting multiplexer";
    started = true;

    for (auto callback : init_callbacks) {
        callback(*this);
    }

    while (!trigger_callbacks.empty()) {
        VLOG(7)
            << "Multiplexer waiting with "
            << trigger_callbacks.size()
            << " handles";

        DWORD timeout = GetTimeout();
        winss::WaitResult result{ TIMEOUT };
        if (timeout > 0) {
            auto begin = winss::MapKeyIterator<winss::HandleWrapper,
                winss::TriggeredCallback>(
                    trigger_callbacks.begin());

            auto end = winss::MapKeyIterator<winss::HandleWrapper,
                winss::TriggeredCallback>(
                    trigger_callbacks.end());

            result = winss::HandleWrapper::Wait(timeout, begin, end);
        }

        if (result.state == TIMEOUT) {
            auto callback = GetTimeoutCallback();
            if (callback) {
                callback(*this);
                continue;
            }
        }

        if (result.state == FAILED) {
            VLOG(1)
                << "Failed to wait on handles: "
                << WINDOWS.GetLastError();
            break;
        }

        auto callback = trigger_callbacks.at(result.handle);
        RemoveTriggeredCallback(result.handle);
        callback(*this, result.handle);
    }

    if (trigger_callbacks.empty()) {
        VLOG(7)
            << "No more callbacks to wait on (exiting: "
            << return_code
            << ")";
    }

    started = false;
    stopping = false;

    return return_code;
}

void winss::WaitMultiplexer::Stop(int code) {
    if (!stopping) {
        VLOG(7) << "Stopping multiplexer with code: " << code;
        stopping = true;
        return_code = code;

        for (auto callback : stop_callbacks) {
            callback(*this);
        }
    }
}

bool winss::WaitMultiplexer::IsStopping() const {
    return stopping;
}

bool winss::WaitMultiplexer::HasStarted() const {
    return started;
}

int winss::WaitMultiplexer::GetReturnCode() const {
    return return_code;
}

void winss::WaitMultiplexer::AddCloseEvent(
    const winss::EventWrapper& close_event, DWORD return_code) {

    auto handle = close_event.GetHandle();

    AddStopCallback([handle](winss::WaitMultiplexer& m) {
        m.RemoveTriggeredCallback(handle);
    });

    AddTriggeredCallback(handle, [return_code](winss::WaitMultiplexer& m,
        const winss::HandleWrapper&) {
        m.Stop(return_code);
    });
}
