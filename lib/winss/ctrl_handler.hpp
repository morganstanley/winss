#ifndef LIB_WINSS_CTRL_HANDLER_HPP_
#define LIB_WINSS_CTRL_HANDLER_HPP_

#include <windows.h>
#include "event_wrapper.hpp"

namespace winss {
    extern winss::EventWrapper& GetCloseEvent();
    extern BOOL WINAPI CtrlHandler(DWORD ctrlType);
    extern void AttachCtrlHandler();
}  // namespace winss

#endif  // LIB_WINSS_CTRL_HANDLER_HPP_
