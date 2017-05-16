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

#ifndef LIB_WINSS_SUPERVISE_STATE_LISTENER_HPP_
#define LIB_WINSS_SUPERVISE_STATE_LISTENER_HPP_

#include <vector>
#include <queue>
#include "../control.hpp"
#include "state_file.hpp"
#include "supervise.hpp"

namespace winss {
/**
 * The actions which the listener can wait for.
 */
enum SuperviseStateListenerAction {
    NO_WAIT,  /**< No-op. */
    WAIT_UP,  /**< Wait for the run process to start. */
    WAIT_DOWN,  /**< Wait for the run process to end. */
    WAIT_FINISHED,  /**< Wait for the finish process to end. */
    WAIT_RESTART  /**< Wait for the service to go down and back up. */
};

/**
 * A supervisor state listener which is race-condition free.
 */
class SuperviseStateListener : public InboundControlItemListener {
 private:
    const winss::SuperviseStateFile& state_file;  /**< The state file. */
    SuperviseStateListenerAction action;  /**< The action to listen to. */
    std::queue<winss::SuperviseNotification> waiting;  /**< Wait queue. */

 public:
    /**
     * Supervise state listener constructor.
     *
     * \param state_file The state file.
     * \param action The action to listen for.
     */
    SuperviseStateListener(const winss::SuperviseStateFile& state_file,
        SuperviseStateListenerAction action);
    /** No copy. */
    SuperviseStateListener(const SuperviseStateListener&) = delete;
    /** No move. */
    SuperviseStateListener(SuperviseStateListener&&) = delete;

    /**
     * Gets if the listener is enabled.
     *
     * \return True if enabled otherwise false.
     */
    bool IsEnabled();

    /**
     * Gets if the listener can start.
     *
     * \return True if can start otherwise false.
     */
    bool CanStart();

    /**
     * Handle the connected event.
     */
    void HandleConnected();

     /**
     * Handle the received event.
     *
     * \param[in] message The message that was received as a list of chars.
     */
    bool HandleReceived(const std::vector<char>& message);

    /** No copy. */
    void operator=(const SuperviseStateListener&) = delete;
    /** No move. */
    SuperviseStateListener& operator=(SuperviseStateListener&&) = delete;
};
}  // namespace winss

#endif  // LIB_WINSS_SUPERVISE_STATE_LISTENER_HPP_
