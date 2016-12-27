#ifndef LIB_WINSS_SVSCAN_SVSCAN_HPP_
#define LIB_WINSS_SVSCAN_SVSCAN_HPP_

#include <windows.h>
#include <easylogging/easylogging++.hpp>
#include <winss/windows_interface.hpp>
#include <winss/filesystem_interface.hpp>
#include <winss/not_owning_ptr.hpp>
#include <winss/handle_wrapper.hpp>
#include <winss/wait_multiplexer.hpp>
#include <winss/path_mutex.hpp>
#include <winss/process.hpp>
#include <winss/utils.hpp>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>
#include "service.hpp"

namespace fs = std::experimental::filesystem;

namespace winss {
template<typename TService, typename TMutex, typename TProcess>
class SvScanTmpl {
 protected:
    winss::NotOwningPtr<winss::WaitMultiplexer> multiplexer;
    fs::path scan_dir;
    const DWORD rescan;
    TMutex mutex;
    bool exiting = false;
    bool close_on_exit = true;

    std::vector<TService> services;

    void Init() {
        if (mutex.HasLock()) {
            return;
        }

        if (!FILESYSTEM.DirectoryExists(scan_dir)) {
            LOG(ERROR) << "The directory '" << scan_dir << "' does not exist.";
            multiplexer->Stop(kFatalExitCode);
            return;
        }

        if (!mutex.Lock()) {
            multiplexer->Stop(kMutexTaken);
            return;
        }

        FILESYSTEM.CreateDirectory(scan_dir / fs::path(kSvscanDir));

        Scan(false);
    }

    void Check(const fs::path& service_dir) {
        std::string name = service_dir.filename().string();

        /* Current, parent and hidden directories should be ignored */
        if (name.empty() || name.front() == L'.') {
            VLOG(4) << "Skipping directory " << name;
            return;
        }

        auto pred = [name](const TService& service) {
            return service.GetName() == name;
        };

        auto it = find_if(services.begin(), services.end(), pred);
        if (it == services.end()) {
            TService service(name, service_dir);
            VLOG(2) << "Found new service " << name;
            service.Check();
            services.push_back(std::move(service));
        } else {
            VLOG(3) << "Found existing service " << name;
            it->Check();
        }
    }

    void Schedule() {
        if (rescan > 0 && rescan != INFINITE) {
            multiplexer->AddTimeoutCallback(rescan, [&](
                winss::WaitMultiplexer&) {
                Scan(true);
            }, kTimeoutGroup);
        }
    }

    void Stop() {
        if (!exiting) {
            multiplexer->RemoveTimeoutCallback(kTimeoutGroup);
            exiting = true;
            if (close_on_exit) {
                CloseAllServices(true);
            }

            fs::path svscan_dir = scan_dir / fs::path(kSvscanDir);
            fs::path finish_file = svscan_dir / fs::path(kFinishFile);

            std::string cmd = FILESYSTEM.Read(finish_file);

            if (!cmd.empty()) {
                VLOG(2) << "Starting finish process";

                std::string expanded =
                    winss::Utils::ExpandEnvironmentVariables(cmd);

                TProcess finish;
                finish.Create(winss::ProcessParams{
                    expanded, false, svscan_dir.string()
                });
            }
        }
    }

 public:
    static const int kMutexTaken = 100;
    static const int kFatalExitCode = 111;
    static constexpr const char kMutexName[7] = "svscan";
    static constexpr const char kTimeoutGroup[7] = "svscan";
    static constexpr const char kSvscanDir[14] = ".winss-svscan";
    static constexpr const char kFinishFile[7] = "finish";

    SvScanTmpl(winss::NotOwningPtr<winss::WaitMultiplexer> multiplexer,
        const fs::path& scan_dir, DWORD rescan) : multiplexer(multiplexer),
        scan_dir(scan_dir), rescan(rescan), mutex(scan_dir, kMutexName) {
        multiplexer->AddInitCallback([&](winss::WaitMultiplexer&) {
            Init();
        });

        multiplexer->AddStopCallback([&](winss::WaitMultiplexer&) {
            Stop();
        });
    }

    SvScanTmpl(const SvScanTmpl&) = delete;
    SvScanTmpl(SvScanTmpl&&) = delete;

    virtual void Scan(bool timeout) {
        if (!mutex.HasLock() || exiting) {
            return;
        }

        if (!timeout) {
            multiplexer->RemoveTimeoutCallback(kTimeoutGroup);
        }

        VLOG(2) << "Scanning directory " << scan_dir;

        for (TService& service : services) {
            service.Reset();
        }

        for (auto dir : FILESYSTEM.GetDirectories(scan_dir)) {
            Check(dir);
        }

        Schedule();
    }

    virtual void CloseAllServices(bool ignore_flagged) {
        if (!mutex.HasLock()) {
            return;
        }

        VLOG(3) << "Closing all services (forced: " << ignore_flagged << ")";

        auto it = services.begin();
        while (it != services.end()) {
            bool flagged = it->Close(ignore_flagged);
            if (!flagged) {
                VLOG(4) << "Removing service " << it->GetName();
                it = services.erase(it);
            } else {
                ++it;
            }
        }
    }

    virtual void Exit(bool close_services) {
        close_on_exit = close_services;
        multiplexer->Stop(0);
    }

    void operator=(const SvScanTmpl&) = delete;
    SvScanTmpl& operator=(SvScanTmpl&&) = delete;
};
typedef SvScanTmpl<winss::Service, winss::PathMutex, winss::Process> SvScan;
}  // namespace winss

#endif  // LIB_WINSS_SVSCAN_SVSCAN_HPP_
