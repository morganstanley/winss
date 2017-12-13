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
/**
 * Holds the STDIN and STDOUT pipes for redirecting.
 */
struct ServicePipes {
    winss::HandleWrapper stdin_pipe;
    winss::HandleWrapper stdout_pipe;
};

/**
 * A template for a service process.
 *
 * Models a service process which would be either the main service or the log
 * service. Log services should be started with a consumer pipe to the pipes
 * of the main service.
 *
 * \tparam TMutex The process implementation type.
 */
template<typename TProcess>
class ServiceProcessTmpl {
 protected:
    fs::path service_dir;  /**< The service directory. */
    TProcess proc;  /**< The supervisor process. */

 public:
    /** The supervisor name. */
    static constexpr const char kSuperviseExe[16] = "winss-supervise";

    /**
     * The default service process template constructor.
     */
    ServiceProcessTmpl() {}

    /**
     * Initializes the service process with the service directory.
     *
     * \param service_dir The path to the service directory.
     */
    explicit ServiceProcessTmpl(fs::path service_dir) :
        service_dir(std::move(service_dir)) {}

    ServiceProcessTmpl(const ServiceProcessTmpl&) = delete;  /**< No copy. */

    /**
     * Creates a new service process and moves it from an old one.
     *
     * \param p The previous service process.
     */
    ServiceProcessTmpl(ServiceProcessTmpl&& p) :
        service_dir(std::move(p.service_dir)),
        proc(std::move(p.proc)) {}

    /**
     * Gets the path of the service directory.
     *
     * \return The service directory path.
     */
    virtual const fs::path& GetServiceDir() const {
        return service_dir;
    }

    /**
    * Gets the service process is created.
    *
    * \return True if the service process is created otherwise false.
    */
    virtual bool IsCreated() const {
        return proc.IsCreated();
    }

    /**
     * Starts the service process.
     *
     * \param[in] pipes The redirected pipes.
     * \param[in] consumer The process is a consumer of the pipes.
     */
    virtual void Start(const ServicePipes& pipes, bool consumer) {
        if (proc.IsCreated()) {
            VLOG(3)
                << "Process for service dir "
                << service_dir
                << " is already running";
            return;
        }

        VLOG(3) << "Starting service " << service_dir;

        std::string cmd = kSuperviseExe + std::string(SUFFIX) + ".exe"
            " \"" + service_dir.string() + "\"";

        winss::ProcessParams params{ cmd, true };

        if (consumer) {
            params.stdin_pipe = pipes.stdin_pipe;
        } else {
            params.stdout_pipe = pipes.stdout_pipe;
            params.stderr_pipe = pipes.stdout_pipe;
        }

        proc.Create(params);
    }

    /**
     * Closes the service process.
     */
    virtual void Close() {
        if (proc.IsCreated()) {
            proc.SendBreak();
            proc.Close();
        }
    }

    /** No copy. */
    ServiceProcessTmpl& operator=(const ServiceProcessTmpl&) = delete;

    /**
     * Moves the service process object to this object.
     *
     * \param p The previous service process.
     * \return This service process.
     */
    ServiceProcessTmpl& operator=(ServiceProcessTmpl&& p) {
        service_dir = std::move(p.service_dir);
        proc = std::move(p.proc);
        return *this;
    }

    /**
     * Service process destructor which will close the process.
     */
    virtual ~ServiceProcessTmpl() {
        proc.Close();
    }
};

/**
 * Concrete service process implementation.
 */
typedef ServiceProcessTmpl<winss::Process> ServiceProcess;
}  // namespace winss

#endif  // LIB_WINSS_SVSCAN_SERVICE_PROCESS_HPP_
