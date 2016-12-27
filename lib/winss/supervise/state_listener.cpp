#include "state_listener.hpp"
#include <easylogging/easylogging++.hpp>
#include <winss/control.hpp>
#include <vector>
#include <queue>
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

bool winss::SuperviseStateListener::HandleRecieved(
    const std::vector<char>& message) {
    if (waiting.empty()) {
        return false;
    }

    for (char c : message) {
        if (c != 0) {
            winss::SuperviseNotification notification =
                winss::SuperviseController::GetNotification(c);

            if (waiting.front() == notification) {
                VLOG(2) << "Recieved expected notification: " << c;
                waiting.pop();
            }
        }
    }

    return !waiting.empty();
}
