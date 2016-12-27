#ifndef LIB_WINSS_SUPERVISE_CONTROLLER_HPP_
#define LIB_WINSS_SUPERVISE_CONTROLLER_HPP_

#include <winss/not_owning_ptr.hpp>
#include <winss/pipe_server.hpp>
#include <vector>
#include "supervise.hpp"

namespace winss {
class SuperviseController :
    public winss::SuperviseListener,
    public winss::PipeServerRecieveListener {
 private:
    winss::NotOwningPtr<winss::Supervise> supervise;
    winss::NotOwningPtr<winss::OutboundPipeServer> outbound;
    winss::NotOwningPtr<winss::InboundPipeServer> inbound;

 public:
    static const char kSvcUp;
    static const char kSvcOnce;
    static const char kSvcOnceAtMost;
    static const char kSvcDown;
    static const char kSvcKill;
    static const char kSvcTerm;
    static const char kSvcExit;

    static const char kSuperviseStart;
    static const char kSuperviseRun;
    static const char kSuperviseEnd;
    static const char kSuperviseFinished;
    static const char kSuperviseExit;

    SuperviseController(winss::NotOwningPtr<winss::Supervise> supervise,
        winss::NotOwningPtr<winss::OutboundPipeServer> outbound,
        winss::NotOwningPtr<winss::InboundPipeServer> inbound);
    SuperviseController(const SuperviseController&) = delete;
    SuperviseController(SuperviseController&&) = delete;

    bool Notify(winss::SuperviseNotification notification,
        const winss::SuperviseState& state);
    bool Recieved(const std::vector<char>& data);

    static winss::SuperviseNotification GetNotification(char c);

    void operator=(const SuperviseController&) = delete;
    SuperviseController& operator=(SuperviseController&&) = delete;
};
}  // namespace winss

#endif  // LIB_WINSS_SUPERVISE_CONTROLLER_HPP_
