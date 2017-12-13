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

#ifndef LIB_WINSS_EVENT_WRAPPER_HPP_
#define LIB_WINSS_EVENT_WRAPPER_HPP_

#include <windows.h>
#include "handle_wrapper.hpp"

namespace winss {
/**
 * Wraps a windows event.
 */
class EventWrapper {
 private:
     winss::TrustedHandleWrapper handle;  /**< The handle to the event. */

 public:
    /**
     * Creates the event wrapper.
     */
    EventWrapper();

    /**
    * Copies the event wrapper.
    *
    * Any ownership rights will not be copied.
    * 
    * \param e The event wrapper to copy.
    */
    EventWrapper(const EventWrapper& e) = default;

    /**
    * Move the event wrapper.
    *
    * Any ownership rights will be moved
    *
    * \param e The event wrapper to move.
    */
    EventWrapper(EventWrapper&& e) = default;

    /**
     * Gets if the event is set or not.
     *
     * \return True if the event has been set otherwise false.
     */
    bool IsSet() const;

    /**
     * Gets a handle to the event.
     *
     * \return The handle as a handle wrapper.
     * \see HandleWrapper
     */
    winss::HandleWrapper GetHandle() const;

    /**
     * Trigger the event.
     *
     * \return True if the set event succeeded.
     */
    bool Set();

    /**
    * Resets the event.
    *
    * \return True if the reset event succeeded.
    */
    bool Reset();

    /**
    * Copies the event wrapper.
    *
    * Any ownership rights will not be copied.
    *
    * \param e The event wrapper to copy.
    * \return This event wrapper.
    */
    EventWrapper& operator=(const EventWrapper& e) = default;

    /**
    * Move the event wrapper.
    *
    * Any ownership rights will be moved
    *
    * \param e The event wrapper to move.
    * \return This event wrapper.
    */
    EventWrapper& operator=(EventWrapper&& e) = default;
};
}  // namespace winss

#endif  // LIB_WINSS_EVENT_WRAPPER_HPP_
