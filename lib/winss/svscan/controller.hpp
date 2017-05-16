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

#ifndef LIB_WINSS_SVSCAN_CONTROLLER_HPP_
#define LIB_WINSS_SVSCAN_CONTROLLER_HPP_

#include <vector>
#include "../not_owning_ptr.hpp"
#include "../pipe_server.hpp"
#include "svscan.hpp"

namespace winss {
/**
 * A controller for svscan process.
 */
class SvScanController : public winss::PipeServerReceiveListener {
 private:
    winss::NotOwningPtr<winss::SvScan> svscan;  /**< The svscan instance. */
    /** Inbound pipe server to listen for commands. */
    winss::NotOwningPtr<winss::InboundPipeServer> inbound;

 public:
    static const char kAlarm;  /**< Alarm control char. */
    static const char kAbort;  /**< Abort control char. */
    static const char kNuke;  /**< Nuke control char. */
    static const char kQuit;  /**< Quit control char. */

     /**
     * svscan controller constructor.
     *
     * \param svscan The svscan instance.
     * \param inbound The inbound named pipe server.
     */
    SvScanController(winss::NotOwningPtr<winss::SvScan> svscan,
        winss::NotOwningPtr<winss::InboundPipeServer> inbound);
    SvScanController(const SvScanController&) = delete;  /**< No copy. */
    SvScanController(SvScanController&&) = delete;  /**< No move. */

    /**
     * Pipe server received handler.
     *
     * \param[in] data The received data.
     * \return Always true.
     */
    bool Received(const std::vector<char>& data);

    void operator=(const SvScanController&) = delete;  /**< No copy. */
    SvScanController& operator=(SvScanController&&) = delete;  /**< No move. */
};
}  // namespace winss

#endif  // LIB_WINSS_SVSCAN_CONTROLLER_HPP_
