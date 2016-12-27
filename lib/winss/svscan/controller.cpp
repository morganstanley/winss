#include "controller.hpp"
#include <winss/not_owning_ptr.hpp>
#include <winss/pipe_server.hpp>
#include <vector>
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

bool winss::SvScanController::Recieved(const std::vector<char>& data) {
    for (char c : data) {
        switch (c) {
        case kAlarm:
            VLOG(4) << "Recieved ALARM command";
            svscan->Scan(false);
            break;
        case kAbort:
            VLOG(4) << "Recieved ABORT command";
            svscan->Exit(false);
            break;
        case kNuke:
            VLOG(4) << "Recieved NUKE command";
            svscan->CloseAllServices(false);
            break;
        case kQuit:
            VLOG(4) << "Recieved QUIT command";
            svscan->Exit(true);
            break;
        default:
            VLOG(4) << "Recieved unknown command " << c;
            break;
        }
    }

    return true;
}
