#ifndef LIB_WINSS_SVSCAN_CONTROLLER_HPP_
#define LIB_WINSS_SVSCAN_CONTROLLER_HPP_

#include <vector>
#include "../not_owning_ptr.hpp"
#include "../pipe_server.hpp"
#include "svscan.hpp"

namespace winss {
class SvScanController : public winss::PipeServerRecieveListener {
 private:
    winss::NotOwningPtr<winss::SvScan> svscan;
    winss::NotOwningPtr<winss::InboundPipeServer> inbound;

 public:
    static const char kAlarm;
    static const char kAbort;
    static const char kNuke;
    static const char kQuit;

    SvScanController(winss::NotOwningPtr<winss::SvScan> svscan,
        winss::NotOwningPtr<winss::InboundPipeServer> inbound);
    SvScanController(const SvScanController&) = delete;
    SvScanController(SvScanController&&) = delete;

    bool Recieved(const std::vector<char>& data);

    void operator=(const SvScanController&) = delete;
    SvScanController& operator=(SvScanController&&) = delete;
};
}  // namespace winss

#endif  // LIB_WINSS_SVSCAN_CONTROLLER_HPP_
