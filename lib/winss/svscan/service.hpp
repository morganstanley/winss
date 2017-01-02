#ifndef LIB_WINSS_SVSCAN_SERVICE_HPP_
#define LIB_WINSS_SVSCAN_SERVICE_HPP_

#include <windows.h>
#include <filesystem>
#include <utility>
#include <string>
#include "easylogging/easylogging++.hpp"
#include "../windows_interface.hpp"
#include "../filesystem_interface.hpp"
#include "../handle_wrapper.hpp"
#include "service_process.hpp"

namespace fs = std::experimental::filesystem;

namespace winss {
template<typename TServiceProcess>
class ServiceTmpl {
 protected:
    std::string name;
    TServiceProcess main;
    TServiceProcess log;

    winss::ServicePipes CreatePipes() {
        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = FALSE;
        sa.lpSecurityDescriptor = nullptr;

        HANDLE stdin_pipe = nullptr;
        HANDLE stdout_pipe = nullptr;

        if (!WINDOWS.CreatePipe(&stdin_pipe, &stdout_pipe, &sa, 0)) {
            VLOG(1) << "CreatePipe() failed: " << ::GetLastError();
            return winss::ServicePipes{};
        }

        return winss::ServicePipes{
            winss::HandleWrapper(stdin_pipe),
            winss::HandleWrapper(stdout_pipe)
        };
    }

 public:
    static constexpr const char kLogDir[4] = "log";

    ServiceTmpl() {}

    ServiceTmpl(std::string name, const fs::path& service_dir) : name(name),
        main(std::move(TServiceProcess(service_dir, false))),
        log(std::move(TServiceProcess(
            service_dir / fs::path(kLogDir), true))) {}

    ServiceTmpl(const ServiceTmpl&) = delete;

    ServiceTmpl(ServiceTmpl&& s) : name(std::move(s.name)),
        main(std::move(s.main)), log(std::move(s.log)) {}

    virtual const std::string& GetName() const {
        return name;
    }

    virtual void Reset() {
        main.Reset();
        log.Reset();
    }

    virtual void Check() {
        winss::ServicePipes pipes;

        if (FILESYSTEM.DirectoryExists(log.GetServiceDir())) {
            VLOG(3) << "Log directory exists for service " << name;
            pipes = CreatePipes();
            log.Start(pipes);
        }

        main.Start(pipes);
    }

    virtual bool Close(bool ignore_flagged) {
        bool flagged = main.Close(ignore_flagged);
        log.Close(ignore_flagged || !flagged);

        return flagged;
    }

    void operator=(const ServiceTmpl&) = delete;

    ServiceTmpl& operator=(ServiceTmpl&& s) {
        name = std::move(s.name);
        main = std::move(s.main);
        log = std::move(s.log);
        return *this;
    }
};
typedef ServiceTmpl<winss::ServiceProcess> Service;
}  // namespace winss

#endif  // LIB_WINSS_SVSCAN_SERVICE_HPP_
