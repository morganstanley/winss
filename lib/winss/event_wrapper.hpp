#ifndef LIB_WINSS_EVENT_WRAPPER_HPP_
#define LIB_WINSS_EVENT_WRAPPER_HPP_

#include <windows.h>
#include "handle_wrapper.hpp"

namespace winss {
class EventWrapper {
 private:
    HANDLE handle;

 public:
    EventWrapper();
    EventWrapper(const EventWrapper&) = delete;
    EventWrapper(EventWrapper&&) = delete;

    bool IsSet() const;
    winss::HandleWrapper GetHandle() const;
    bool Set();

    void operator=(const EventWrapper&) = delete;
    EventWrapper& operator=(EventWrapper&&) = delete;

    ~EventWrapper();
};
}  // namespace winss

#endif  // LIB_WINSS_EVENT_WRAPPER_HPP_
