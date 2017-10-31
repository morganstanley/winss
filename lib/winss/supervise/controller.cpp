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
#include "easylogging/easylogging++.hpp"
#include "../not_owning_ptr.hpp"
#include "../pipe_server.hpp"
#include "supervise.hpp"

const char winss::SuperviseController::kSvcUp = 'u';
const char winss::SuperviseController::kSvcOnce = 'o';
const char winss::SuperviseController::kSvcOnceAtMost = 'O';
const char winss::SuperviseController::kSvcDown = 'd';
const char winss::SuperviseController::kSvcKill = 'k';
const char winss::SuperviseController::kSvcTerm = 't';
const char winss::SuperviseController::kSvcExit = 'x';

const char winss::SuperviseController::kSuperviseStart = 's';
const char winss::SuperviseController::kSuperviseRun = 'u';
const char winss::SuperviseController::kSuperviseEnd = 'd';
const char winss::SuperviseController::kSuperviseBroken = 'O';
const char winss::SuperviseController::kSuperviseFinished = 'D';
const char winss::SuperviseController::kSuperviseExit = 'x';

winss::SuperviseController::SuperviseController(
    winss::NotOwningPtr<winss::Supervise> supervise,
    winss::NotOwningPtr<winss::OutboundPipeServer> outbound,
    winss::NotOwningPtr<winss::InboundPipeServer> inbound) :
    supervise(supervise), outbound(outbound), inbound(inbound) {
    supervise->AddListener(winss::NotOwned(this));
    inbound->AddListener(winss::NotOwned(this));
}

bool winss::SuperviseController::Notify(
    winss::SuperviseNotification notification,
    const winss::SuperviseState& state) {
    switch (notification) {
    case START:
        VLOG(4) << "Sending START";
        outbound->Send({ kSuperviseStart });
        break;
    case RUN:
        VLOG(4) << "Sending RUN";
        outbound->Send({ kSuperviseRun });
        break;
    case END:
        VLOG(4) << "Sending END";
        outbound->Send({ kSuperviseEnd });
        break;
    case BROKEN:
        VLOG(4) << "Sending BROKEN";
        outbound->Send({ kSuperviseBroken });
        break;
    case FINISHED:
        VLOG(4) << "Sending FINISHED";
        outbound->Send({ kSuperviseFinished });
        break;
    case EXIT:
        VLOG(4) << "Sending EXIT";
        outbound->Send({ kSuperviseExit });
        break;
    }

    return true;
}

bool winss::SuperviseController::Received(const std::vector<char>& data) {
    for (char c : data) {
        switch (c) {
        case kSvcUp:
            VLOG(4) << "Received UP command";
            supervise->Up();
            break;
        case kSvcOnce:
            VLOG(4) << "Received ONCE command";
            supervise->Once();
            break;
        case kSvcOnceAtMost:
            VLOG(4) << "Received ONCE_AT_MOST command";
            supervise->OnceAtMost();
            break;
        case kSvcDown:
            VLOG(4) << "Received DOWN command";
            supervise->Down();
            break;
        case kSvcKill:
            VLOG(4) << "Received KILL command";
            supervise->Kill();
            break;
        case kSvcTerm:
            VLOG(4) << "Received TERM command";
            supervise->Term();
            break;
        case kSvcExit:
            VLOG(4) << "Received EXIT command";
            supervise->Exit();
            break;
        default:
            VLOG(1) << "Received unknown command " << c;
            break;
        }
    }

    return true;
}

winss::SuperviseNotification winss::SuperviseController::GetNotification(
    char c) {
    switch (c) {
    case kSuperviseStart:
        return START;
    case kSuperviseRun:
        return RUN;
    case kSuperviseEnd:
        return END;
    case kSuperviseFinished:
        return FINISHED;
    case kSuperviseExit:
        return EXIT;
    default:
        VLOG(4) << "Notification unknown: " << c;
        return UNKNOWN;
    }
}
