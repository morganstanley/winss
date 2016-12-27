#ifndef LIB_WINSS_CONTROL_HPP_
#define LIB_WINSS_CONTROL_HPP_

#include <string>
#include <vector>
#include <map>
#include <set>
#include "pipe_client.hpp"
#include "wait_multiplexer.hpp"
#include "not_owning_ptr.hpp"

namespace winss {
class ControlItem {
 protected:
    const std::string name;

    explicit ControlItem(std::string name);

 public:
    virtual const std::string& GetName() const;

    virtual void Init() {}
    virtual void Start() {}
    virtual bool Completed() const = 0;
    virtual ~ControlItem() {}
};
class Control {
 private:
    static const int kTimeoutExitCode;
    static const char kTimeoutGroup[];
    winss::NotOwningPtr<winss::WaitMultiplexer> multiplexer;
    std::map<std::string, winss::NotOwningPtr<ControlItem>> items;
    std::set<std::string> ready;
    bool started = false;
    const DWORD timeout;
    const bool finish_all;

 public:
    explicit Control(winss::NotOwningPtr<winss::WaitMultiplexer> multiplexer,
        DWORD timeout = INFINITE, bool finish_all = true);
    Control(const Control&) = delete;
    Control(Control&&) = delete;

    bool IsStarted() const;

    void Add(winss::NotOwningPtr<ControlItem> item);
    void Ready(std::string name);
    void Remove(std::string name);

    int Start();

    void operator=(const Control&) = delete;
    Control& operator=(Control&&) = delete;
};
class OutboundControlItem :
    public ControlItem,
    public winss::PipeClientSendListener {
 private:
    static const int kNoSendExitCode;
    winss::NotOwningPtr<WaitMultiplexer> multiplexer;
    winss::NotOwningPtr<Control> control;
    winss::NotOwningPtr<winss::OutboundPipeClient> client;
    const std::vector<char>& commands;
    bool written = false;

 public:
    OutboundControlItem(winss::NotOwningPtr<WaitMultiplexer> multiplexer,
        winss::NotOwningPtr<Control> control,
        winss::NotOwningPtr<winss::OutboundPipeClient> client,
        const std::vector<char>& commands, std::string name);
    OutboundControlItem(const OutboundControlItem&) = delete;
    OutboundControlItem(OutboundControlItem&&) = delete;

    void Init();
    void Start();
    bool Completed() const;
    bool Connected();
    bool WriteComplete();
    bool Disconnected();

    void operator=(const OutboundControlItem&) = delete;
    OutboundControlItem& operator=(OutboundControlItem&&) = delete;
};
class InboundControlItemListener {
 public:
    virtual bool IsEnabled() = 0;
    virtual bool CanStart() = 0;
    virtual void HandleConnected() {}
    virtual bool HandleRecieved(const std::vector<char>& message) = 0;
    virtual ~InboundControlItemListener() {}
};
class InboundControlItem :
    public ControlItem,
    public winss::PipeClientRecieveListener {
 private:
    winss::NotOwningPtr<WaitMultiplexer> multiplexer;
    winss::NotOwningPtr<Control> control;
    winss::NotOwningPtr<winss::InboundPipeClient> client;
    winss::NotOwningPtr<InboundControlItemListener> listener;
    bool complete = false;

 public:
    InboundControlItem(winss::NotOwningPtr<WaitMultiplexer> multiplexer,
        winss::NotOwningPtr<Control> control,
        winss::NotOwningPtr<winss::InboundPipeClient> client,
        winss::NotOwningPtr<InboundControlItemListener> listener,
        std::string name);
    InboundControlItem(const InboundControlItem&) = delete;
    InboundControlItem(InboundControlItem&&) = delete;

    void Init();
    void Start();
    bool Completed() const;
    bool Connected();
    bool Recieved(const std::vector<char>& message);
    bool Disconnected();

    void operator=(const InboundControlItem&) = delete;
    InboundControlItem& operator=(InboundControlItem&&) = delete;
};
}  // namespace winss

#endif  // LIB_WINSS_CONTROL_HPP_
