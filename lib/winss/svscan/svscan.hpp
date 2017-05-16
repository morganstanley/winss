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

#ifndef LIB_WINSS_SVSCAN_SVSCAN_HPP_
#define LIB_WINSS_SVSCAN_SVSCAN_HPP_

#include <windows.h>
#include <filesystem>
#include <functional>
#include <utility>
#include <vector>
#include <string>
#include "easylogging/easylogging++.hpp"
#include "../windows_interface.hpp"
#include "../filesystem_interface.hpp"
#include "../not_owning_ptr.hpp"
#include "../wait_multiplexer.hpp"
#include "../path_mutex.hpp"
#include "../process.hpp"
#include "../utils.hpp"
#include "service.hpp"

namespace fs = std::experimental::filesystem;

namespace winss {
/**
 * The svscan template.
 *
 * Scans a directory either on a timer or on demand and starts supervisors
 * for each service directory it sees.
 *
 * \tparam TService The service implementation type.
 * \tparam TMutex The mutex implementation type.
 * \tparam TMutex The process implementation type.
 */
template<typename TService, typename TMutex, typename TProcess>
class SvScanTmpl {
 protected:
    /** The event multiplexer for svscan. */
    winss::NotOwningPtr<winss::WaitMultiplexer> multiplexer;
    fs::path scan_dir;  /**< The scan directory. */
    const DWORD rescan;  /**< The directory scan period. */
    TMutex mutex;  /**< The svscan global mutex. */
    bool exiting = false;  /**< Exiting flag. */
    bool close_on_exit = true;  /**< Option to close services on exit. */

    std::vector<TService> services;  /**< A list of services. */

    /**
     * Initializes svscan.
     */
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

    /**
     * Checks the given service directory.
     *
     * If the service dir has not been seen then it will be added to the list.
     * If the service dir has been seen it will be checked to make sure it is
     * running.
     *
     * \param[in] service_dir The service directory.
     */
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

    /**
     * Schedules the next scan of the scan directory.
     */
    void Schedule() {
        if (rescan > 0 && rescan != INFINITE) {
            multiplexer->AddTimeoutCallback(rescan, [&](
                winss::WaitMultiplexer&) {
                Scan(true);
            }, kTimeoutGroup);
        }
    }

    /**
     * Stops the svscan instance.
     */
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
    static const int kMutexTaken = 100;  /**< Scan dir in use error. */
    static const int kFatalExitCode = 111;  /**< Something went wrong. */
    static constexpr const char kMutexName[7] = "svscan"; /**< Mutex name. */
    /** The timeout group for the multiplexer. */
    static constexpr const char kTimeoutGroup[7] = "svscan";
    /** The directory for svscan data. */
    static constexpr const char kSvscanDir[14] = ".winss-svscan";
    static constexpr const char kFinishFile[7] = "finish";  /**< Finish file. */

    /**
     * SvScan constructor.
     *
     * \param multiplexer The shared multiplexer.
     * \param scan_dir The scan directory.
     * \param rescan The scan period.
     */
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

    SvScanTmpl(const SvScanTmpl&) = delete;  /**< No copy. */
    SvScanTmpl(SvScanTmpl&&) = delete;  /**< No move. */

    /**
     * Does a scan of the scan directory.
     */
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

    /**
     * Closes all the services.
     *
     * \param ignore_flagged Force the services to close.
     */
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


    /**
     * Signals the scanner to exit.
     *
     * \param close_services Mark if the services should be closed.
     */
    virtual void Exit(bool close_services) {
        close_on_exit = close_services;
        multiplexer->Stop(0);
    }

    void operator=(const SvScanTmpl&) = delete;  /**< No copy. */
    SvScanTmpl& operator=(SvScanTmpl&&) = delete;  /**< No move. */
};

/**
 * Concrete svscan implementation.
 */
typedef SvScanTmpl<winss::Service, winss::PathMutex, winss::Process> SvScan;
}  // namespace winss

#endif  // LIB_WINSS_SVSCAN_SVSCAN_HPP_
