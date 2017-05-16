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

#ifndef LIB_WINSS_SUPERVISE_CONTROLLER_HPP_
#define LIB_WINSS_SUPERVISE_CONTROLLER_HPP_

#include <vector>
#include "../not_owning_ptr.hpp"
#include "../pipe_server.hpp"
#include "supervise.hpp"

namespace winss {
/**
 * A controller for supervised processes.
 *
 * Brokers the communication to and from the supervised process for control
 * and event notification.
 */
class SuperviseController :
    public winss::SuperviseListener,
    public winss::PipeServerReceiveListener {
 private:
    winss::NotOwningPtr<winss::Supervise> supervise;  /**< The supervisor. */
    winss::NotOwningPtr<winss::OutboundPipeServer> outbound;  /**< Events. */
    winss::NotOwningPtr<winss::InboundPipeServer> inbound;  /**< Control. */

 public:
    static const char kSvcUp;  /**< Up control char. */
    static const char kSvcOnce;  /**< Up once code. */
    static const char kSvcOnceAtMost;  /**< Up once at most control char. */
    static const char kSvcDown;  /**< Down control char. */
    static const char kSvcKill;  /**< Kill control char. */
    static const char kSvcTerm;  /**< Terminate control char. */
    static const char kSvcExit;  /**< Exit control char. */

    static const char kSuperviseStart;  /**< Start event. */
    static const char kSuperviseRun;  /**< Run event. */
    static const char kSuperviseEnd;  /**< End event. */
    static const char kSuperviseFinished;  /**< Finished event. */
    static const char kSuperviseExit;  /**< Exit event. */

    /**
     * Supervise controller constructor.
     *
     * \param supervise The supervisor.
     * \param outbound The outbound named pipe server.
     * \param inbound The inbound named pipe server.
     */
    SuperviseController(winss::NotOwningPtr<winss::Supervise> supervise,
        winss::NotOwningPtr<winss::OutboundPipeServer> outbound,
        winss::NotOwningPtr<winss::InboundPipeServer> inbound);
    SuperviseController(const SuperviseController&) = delete;  /**< No copy. */
    SuperviseController(SuperviseController&&) = delete;  /**< No move. */

    /**
     * Supervisor listener handler.
     *
     * \param[in] notification The event which occurred.
     * \param[in] state The current state of the supervisor.
     * \return Always true.
     */
    bool Notify(winss::SuperviseNotification notification,
        const winss::SuperviseState& state);

    /**
     * Pipe server received handler.
     *
     * \param[in] data The received data.
     * \return Always true.
     */
    bool Received(const std::vector<char>& data);

    /**
     * Gets the notification for the given control char.
     *
     * \param[in] c The control char.
     * \return The notification.
     */
    static winss::SuperviseNotification GetNotification(char c);

    /** No copy. */
    void operator=(const SuperviseController&) = delete;
    /** No move. */
    SuperviseController& operator=(SuperviseController&&) = delete;
};
}  // namespace winss

#endif  // LIB_WINSS_SUPERVISE_CONTROLLER_HPP_
