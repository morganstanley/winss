#ifndef LIB_WINSS_SUPERVISE_STATE_LISTENER_HPP_
#define LIB_WINSS_SUPERVISE_STATE_LISTENER_HPP_

#include <vector>
#include <queue>
#include "../control.hpp"
#include "state_file.hpp"
#include "supervise.hpp"

namespace winss {
enum SuperviseStateListenerAction {
    NO_WAIT,
    WAIT_UP,
    WAIT_DOWN,
    WAIT_FINISHED,
    WAIT_RESTART
};
class SuperviseStateListener : public InboundControlItemListener {
 private:
    const winss::SuperviseStateFile& state_file;
    SuperviseStateListenerAction action;
    std::queue<winss::SuperviseNotification> waiting;

 public:
    SuperviseStateListener(const winss::SuperviseStateFile& state_file,
        SuperviseStateListenerAction action);
    SuperviseStateListener(const SuperviseStateListener&) = delete;
    SuperviseStateListener(SuperviseStateListener&&) = delete;

    bool IsEnabled();
    bool CanStart();
    void HandleConnected();
    bool HandleRecieved(const std::vector<char>& message);

    void operator=(const SuperviseStateListener&) = delete;
    SuperviseStateListener& operator=(SuperviseStateListener&&) = delete;
};
}  // namespace winss

#endif  // LIB_WINSS_SUPERVISE_STATE_LISTENER_HPP_
