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

#include "controller.hpp"
#include <vector>
#include "../not_owning_ptr.hpp"
#include "../pipe_server.hpp"
#include "svscan.hpp"

const char winss::SvScanController::kAlarm = 'a';
const char winss::SvScanController::kAbort = 'b';
const char winss::SvScanController::kNuke = 'n';
const char winss::SvScanController::kQuit = 'q';

winss::SvScanController::SvScanController(
    winss::NotOwningPtr<winss::SvScan> svscan,
    winss::NotOwningPtr<winss::InboundPipeServer> inbound) :
    svscan(svscan), inbound(inbound) {
    inbound->AddListener(winss::NotOwned(this));
}

bool winss::SvScanController::Received(const std::vector<char>& data) {
    for (char c : data) {
        switch (c) {
        case kAlarm:
            VLOG(4) << "Received ALARM command";
            svscan->Scan(false);
            break;
        case kAbort:
            VLOG(4) << "Received ABORT command";
            svscan->Exit(false);
            break;
        case kNuke:
            VLOG(4) << "Received NUKE command";
            svscan->CloseAllServices(false);
            break;
        case kQuit:
            VLOG(4) << "Received QUIT command";
            svscan->Exit(true);
            break;
        default:
            VLOG(4) << "Received unknown command " << c;
            break;
        }
    }

    return true;
}
