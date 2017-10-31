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

/**
 * A callback function for wait multiplexer events.
 */
typedef std::function<void(WaitMultiplexer&)> Callback;

/**
 * A callback function specifically for triggered multiplexer events.
 */
typedef std::function<void(WaitMultiplexer&,
    const winss::HandleWrapper&)> TriggeredCallback;

/**
 * Holds timeout information such that when a timeout occurs the multiplexer
 * knows how to handle it.
 *
 * The multiplexer can handle multiple timeout items. 
 */
struct WaitTimeoutItem {
    /** Used to identify the group of items when removing them. */
    std::string group;
    /** The point in time the timeout will be in effect. **/
    std::chrono::system_clock::time_point timeout;
    Callback callback;  /**< The call back for when the timeout occurs. */

    /**
     * Used to order the timeout items such as next item is the one with the
     * point in time closest to now.
     *
     * \param rhs The other timeout item.
     * \return True if this point in time is less than rhs otherwise false.
     */
    bool operator<(const WaitTimeoutItem& rhs) const;
};

/**
* A HANDLE wait multiplexer 
*/
class WaitMultiplexer {
 private:
    bool started = false;   /**< Flags if the multiplexer is in a loop. */
    bool stopping = false;  /**< Flags if the multiplexer is stopping. */
    /** The return code which is usually the exit code. */
    int return_code = 0;
    /** Callbacks to call on initialization. */
    std::vector<Callback> init_callbacks;
    /** Wait events translate to trigger callbacks. */
    std::map<winss::HandleWrapper, TriggeredCallback> trigger_callbacks;
    /** The timeout callback items. */
    std::set<WaitTimeoutItem> timeout_callbacks;
    /** Callbacks to call on stop. */
    std::vector<Callback> stop_callbacks;

    /**
     * Gets the next timeout callback.
     *
     * It will simply get the first item in the timeout_callbacks which is a
     * sorted set of callback items.
     *
     * \return The callback which can be directly invoked.
     */
    Callback GetTimeoutCallback();

 public:
    /** The default constructor. */
    WaitMultiplexer() {}
    WaitMultiplexer(const WaitMultiplexer&) = delete;  /**< No copy. */
    WaitMultiplexer(WaitMultiplexer&&) = delete;  /**< No move. */

    /**
     * Add an initialization callback.
     *
     * \param callback The initialization callback.
     */
    virtual void AddInitCallback(Callback callback);

    /**
     * Add a triggered callback for when an event happens on the given handle.
     *
     * \param handle The handle which will be watched.
     * \param callback The initialization callback.
     */
    virtual void AddTriggeredCallback(const winss::HandleWrapper& handle,
        TriggeredCallback callback);

    /**
     * Add a timeout item which given the timeout period will call the callback
     * if it is not removed before that time.
     *
     * \param timeout The time in ms from now.
     * \param callback The callback to call on timeout event.
     * \param group The group to identify the callback.
     */
    virtual void AddTimeoutCallback(DWORD timeout,
        Callback callback, std::string group = "");

    /**
     * Add a stop callback.
     *
     * \param callback The stop callback.
     */
    virtual void AddStopCallback(Callback callback);

    /**
     * Removes the triggered callback which matches the given handle.
     *
     * \param handle The handle which the callback is associated with.
     */
    virtual bool RemoveTriggeredCallback(const winss::HandleWrapper& handle);

    /**
     * Removes the timeout call back for the given group.
     *
     * \param group The group which the timeout callback is associated with.
     */
    virtual bool RemoveTimeoutCallback(std::string group);

    /**
     * Gets the next timeout in ms from now.
     *
     * \return The next timeout in ms from now.
     */
    virtual DWORD GetTimeout() const;

    /**
     * Starts the multiplexer which will block until some other event stops it.
     *
     * It will go into a loop calling WaitForMultipleObjects with all handles
     * and the next timeout. When this returns the multiplexer will work out
     * which event fired or timed out and will call the appropriate callback.
     *
     * \return The exit code which was set.
     */
    virtual int Start();

    /**
     * Stops the multiplexer with the given code if one has not already been
     * set.
     *
     * \param code The exit code to stop with.
     */
    virtual void Stop(int code);

    /**
     * Gets if the multiplexer is stopping.
     *
     * \return True if the multiplexer is stopping otherwise false.
     */
    virtual bool IsStopping() const;

    /**
     * Gets if the multiplexer is has started.
     *
     * \return True if the multiplexer has started otherwise false.
     */
    virtual bool HasStarted() const;

    /**
     * Gets the set return code of the multiplexer.
     *
     * \return The return code.
     */
    virtual int GetReturnCode() const;

    /**
     * Add a close event to the multiplexer.
     *
     * This will watch the event and stop the multiplexer with the given return
     * code.
     *
     * \param close_event The event object to watch.
     * \param return_code The return code to exit with.
     */
    virtual void AddCloseEvent(const winss::EventWrapper& close_event,
        DWORD return_code);

    /** No copy. */
    WaitMultiplexer& operator=(const WaitMultiplexer&) = delete;
    /** No move. */
    WaitMultiplexer& operator=(WaitMultiplexer&&) = delete;
};
}  // namespace winss

#endif  // LIB_WINSS_WAIT_MULTIPLEXER_HPP_
