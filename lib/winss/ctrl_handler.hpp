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

#ifndef LIB_WINSS_CTRL_HANDLER_HPP_
#define LIB_WINSS_CTRL_HANDLER_HPP_

#include <windows.h>
#include "event_wrapper.hpp"

namespace winss {
    /**
     * Gets a global close event.
     */
    extern winss::EventWrapper& GetCloseEvent();

    /**
     * The CTRL handler.
     */
    extern BOOL WINAPI CtrlHandler(DWORD ctrlType);

    /**
     * Attaches the CTRL handler to the event.
     */
    extern void AttachCtrlHandler();
}  // namespace winss

#endif  // LIB_WINSS_CTRL_HANDLER_HPP_
