#ifndef LIB_WINSS_SVSCAN_SERVICE_PROCESS_HPP_
#define LIB_WINSS_SVSCAN_SERVICE_PROCESS_HPP_

#include <filesystem>
#include <utility>
#include <string>
#include "easylogging/easylogging++.hpp"
#include "../handle_wrapper.hpp"
#include "../process.hpp"
#include "winss/winss.hpp"

namespace fs = std::experimental::filesystem;

namespace winss {
struct ServicePipes {
    winss::HandleWrapper stdin_pipe;
    winss::HandleWrapper stdout_pipe;
};
template<typename TProcess>
class ServiceProcessTmpl {
 protected:
    fs::path service_dir;
    TProcess proc;
    bool flagged = false;
    bool is_log = false;

 public:
    static constexpr const char kSuperviseExe[16] = "winss-supervise";

    ServiceProcessTmpl() {}

    ServiceProcessTmpl(const fs::path& service_dir, bool is_log) :
        service_dir(std::move(service_dir)), is_log(is_log) {}

    ServiceProcessTmpl(const ServiceProcessTmpl&) = delete;

    ServiceProcessTmpl(ServiceProcessTmpl&& p) :
        service_dir(std::move(p.service_dir)), proc(std::move(p.proc)),
        flagged(p.flagged), is_log(p.is_log) {}

    virtual bool IsFlagged() const {
        return flagged;
    }

    virtual bool IsLog() const {
        return is_log;
    }

    virtual const fs::path& GetServiceDir() const {
        return service_dir;
    }

    virtual void Reset() {
        VLOG(4) << "Resetting service " << service_dir;
        flagged = false;
    }

    virtual void Start(const ServicePipes& pipes) {
        VLOG(3) << "Starting service " << service_dir;
        flagged = true;

        if (proc.IsCreated()) {
            VLOG(3)
                << "Process for service dir "
                << service_dir
                << " is already running";
            return;
        }

        std::string cmd = kSuperviseExe + std::string(SUFFIX) + ".exe"
            " \"" + service_dir.string() + "\"";

        winss::ProcessParams params{ cmd, true };

        if (is_log) {
            params.stdin_pipe = pipes.stdin_pipe;
        } else {
            params.stdout_pipe = pipes.stdout_pipe;
            params.stderr_pipe = pipes.stdout_pipe;
        }

        proc.Create(params);
    }

    virtual bool Close(bool ignore_flagged) {
        if ((ignore_flagged || !flagged) && proc.IsCreated()) {
            VLOG(3) << "Closing service " << service_dir;
            proc.SendBreak();
            proc.Close();
            flagged = false;
        }

        return flagged;
    }

    void operator=(const ServiceProcessTmpl&) = delete;

    ServiceProcessTmpl& operator=(ServiceProcessTmpl&& p) {
        service_dir = std::move(p.service_dir);
        proc = std::move(p.proc);
        flagged = p.flagged;
        is_log = p.is_log;
        return *this;
    }

    virtual ~ServiceProcessTmpl() {
        proc.Close();
    }
};
typedef ServiceProcessTmpl<winss::Process> ServiceProcess;
}  // namespace winss

#endif  // LIB_WINSS_SVSCAN_SERVICE_PROCESS_HPP_
