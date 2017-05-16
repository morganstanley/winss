/*
 * Copyright 2016-2017 Morgan Stanley
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
