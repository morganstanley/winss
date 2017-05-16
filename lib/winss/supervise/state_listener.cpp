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

#include "state_listener.hpp"
#include <vector>
#include <queue>
#include "easylogging/easylogging++.hpp"
#include "state_file.hpp"
#include "controller.hpp"
#include "supervise.hpp"

winss::SuperviseStateListener::SuperviseStateListener(
    const winss::SuperviseStateFile& state_file,
    winss::SuperviseStateListenerAction action) :
    state_file(state_file), action(action) {}

bool winss::SuperviseStateListener::IsEnabled() {
    return action != NO_WAIT;
}

bool winss::SuperviseStateListener::CanStart() {
    return !waiting.empty();
}

void winss::SuperviseStateListener::HandleConnected() {
    winss::SuperviseState state{};
    if (state_file.Read(&state)) {
        switch (action) {
        case WAIT_UP:
            if (!state.is_up || !state.is_run_process) {
                VLOG(2) << "Wating on RUN notification";
                waiting.push(winss::SuperviseNotification::RUN);
            }
            break;
        case WAIT_DOWN:
            if (state.is_up && state.is_run_process) {
                VLOG(2) << "Wating on END notification";
                waiting.push(winss::SuperviseNotification::END);
            }
            break;
        case WAIT_FINISHED:
            if (state.is_up || state.is_run_process) {
                VLOG(2) << "Wating on FINISH notification";
                waiting.push(winss::SuperviseNotification::FINISHED);
            }
            break;
        case WAIT_RESTART:
            if (state.is_up) {
                VLOG(2) << "Wating on FINISH & RUN notification";
                waiting.push(winss::SuperviseNotification::FINISHED);
                waiting.push(winss::SuperviseNotification::RUN);
            } else {
                VLOG(2) << "Wating on RUN notification";
                waiting.push(winss::SuperviseNotification::RUN);
            }
            break;
        }
    }
}

bool winss::SuperviseStateListener::HandleReceived(
    const std::vector<char>& message) {
    if (waiting.empty()) {
        return false;
    }

    for (char c : message) {
        if (c != 0) {
            winss::SuperviseNotification notification =
                winss::SuperviseController::GetNotification(c);

            if (waiting.front() == notification) {
                VLOG(2) << "Received expected notification: " << c;
                waiting.pop();
            }
        }
    }

    return !waiting.empty();
}
