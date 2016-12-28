#ifndef LIB_WINSS_WAIT_MULTIPLEXER_HPP_
#define LIB_WINSS_WAIT_MULTIPLEXER_HPP_

#include <windows.h>
#include <chrono>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <functional>
#include "handle_wrapper.hpp"
#include "event_wrapper.hpp"

namespace winss {
class WaitMultiplexer;
typedef std::function<void(WaitMultiplexer&)> Callback;
typedef std::function<void(WaitMultiplexer&,
    const winss::HandleWrapper&)> TriggeredCallback;
struct WaitTimeoutItem {
    std::string group;
    std::chrono::system_clock::time_point timeout;
    Callback callback;

    bool operator<(const WaitTimeoutItem& rhs) const;
};
class WaitMultiplexer {
 private:
    bool started = false;
    bool stopping = false;
    int return_code = 0;
    std::vector<Callback> init_callbacks;
    std::map<winss::HandleWrapper, TriggeredCallback> trigger_callbacks;
    std::set<WaitTimeoutItem> timeout_callbacks;
    std::vector<Callback> stop_callbacks;

    Callback GetTimeoutCallback();

 public:
    WaitMultiplexer() {}
    WaitMultiplexer(const WaitMultiplexer&) = delete;
    WaitMultiplexer(WaitMultiplexer&&) = delete;

    virtual void AddInitCallback(Callback callback);
    virtual void AddTriggeredCallback(const winss::HandleWrapper& handle,
        TriggeredCallback callback);
    virtual void AddTimeoutCallback(DWORD timeout,
        Callback callback, std::string group = "");
    virtual void AddStopCallback(Callback callback);

    virtual bool RemoveTriggeredCallback(const winss::HandleWrapper& handle);
    virtual bool RemoveTimeoutCallback(std::string group);

    virtual DWORD GetTimeout() const;

    virtual int Start();
    virtual void Stop(int code);

    virtual bool IsStopping() const;
    virtual bool HasStarted() const;
    virtual int GetReturnCode() const;

    virtual void AddCloseEvent(const winss::EventWrapper& close_event,
        DWORD return_code);

    void operator=(const WaitMultiplexer&) = delete;
    WaitMultiplexer& operator=(WaitMultiplexer&&) = delete;
};
}  // namespace winss

#endif  // LIB_WINSS_WAIT_MULTIPLEXER_HPP_
