#include "ctrl_handler.hpp"
#include <windows.h>
#include "easylogging/easylogging++.hpp"
#include "event_wrapper.hpp"
#include "windows_interface.hpp"

winss::EventWrapper& winss::GetCloseEvent() {
    static winss::EventWrapper close_event;
    return close_event;
}

BOOL WINAPI winss::CtrlHandler(DWORD ctrlType) {
    switch (ctrlType) {
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_C_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        VLOG(2) << "Close event set";
        GetCloseEvent().Set();
        return TRUE;
    }

    return FALSE;
}

void winss::AttachCtrlHandler() {
    WINDOWS.SetConsoleCtrlHandler(winss::CtrlHandler, true);
}
