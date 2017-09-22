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

#ifndef LIB_WINSS_SUPERVISE_SUPERVISE_HPP_
#define LIB_WINSS_SUPERVISE_SUPERVISE_HPP_

#include <windows.h>
#include <filesystem>
#include <vector>
#include <chrono>
#include <string>
#include "easylogging/easylogging++.hpp"
#include "../handle_wrapper.hpp"
#include "../windows_interface.hpp"
#include "../filesystem_interface.hpp"
#include "../wait_multiplexer.hpp"
#include "../not_owning_ptr.hpp"
#include "../environment.hpp"
#include "../path_mutex.hpp"
#include "../process.hpp"
#include "../utils.hpp"

namespace fs = std::experimental::filesystem;

namespace winss {
/**
 * The state of the supervisor.
 */
struct SuperviseState {
    std::chrono::system_clock::time_point time;
    std::chrono::system_clock::time_point last;
    bool is_run_process;
    bool is_up;
    bool initially_up;
    int up_count;
    int remaining_count;
    int exit_code;
    DWORD pid;
};

/**
 * The supervisor events which can occur.
 */
enum SuperviseNotification {
    UNKNOWN,  /**< Unknown notification. */
    START,  /**< Supervisor starting. */
    RUN,  /**< Run process has started. */
    END,  /**< Run process has ended. */
    FINISHED,  /**< Finish process has ended. */
    EXIT  /**< Supervisor exiting. */
};

/**
 * The supervisor listener.
 */
class SuperviseListener {
 public:
     /**
     * Supervisor listener handler.
     *
     * \param[in] notification The event which occurred.
     * \param[in] state The current state of the supervisor.
     * \return True if continue to listen otherwise false.
     */
    virtual bool Notify(SuperviseNotification notification,
        const SuperviseState& state) = 0;

    /**
     * Default virtual destructor.
     */
    virtual ~SuperviseListener() {}
};

/**
 * The supervisor class template.
 *
 * \tparam TMutex The mutex implementation type.
 * \tparam TProcess The process implementation type.
 */
template<typename TMutex, typename TProcess>
class SuperviseTmpl {
 protected:
    /** The event multiplexer for the supervisor. */
    winss::NotOwningPtr<winss::WaitMultiplexer> multiplexer;
    TMutex mutex;  /**< The supervisor global mutex. */
    TProcess process;  /**< The supervised process. */
    fs::path service_dir;  /**< The service directory. */
    SuperviseState state{};  /**< The current supervised state. */
    /** The supervisor listeners. */
    std::vector<winss::NotOwningPtr<winss::SuperviseListener>> listeners;
    int exiting = 0;  /**< The exiting state. */
    bool waiting = false;  /**< The waiting state. */

    /**
     * Initializes the supervisor.
     */
    virtual void Init() {
        if (mutex.HasLock()) {
            return;
        }

        if (!FILESYSTEM.ChangeDirectory(service_dir)) {
            LOG(ERROR)
                << "The directory '"
                << service_dir
                << "' does not exist.";
            multiplexer->Stop(kFatalExitCode);
            return;
        }

        if (!mutex.Lock()) {
            multiplexer->Stop(kMutexTaken);
            return;
        }

        state.time = std::chrono::system_clock::now();
        state.last = state.time;

        if (FILESYSTEM.FileExists(kDownFile)) {
            state.initially_up = false;
            state.remaining_count = 0;
        }

        NotifyAll(START);
        Triggered(false);
    }


    /**
     * Gets the finish timeout value.
     *
     * This will read the finish-timeout file if it exists.
     *
     * \return The finish timeout in milliseconds.
     */
    virtual DWORD GetFinishTimeout() const {
        std::string timeout_finish = FILESYSTEM.Read(kTimeoutFinishFile);

        if (timeout_finish.empty()) {
            return kCommandTimeout;
        }

        return std::strtoul(timeout_finish.data(), nullptr, 10);
    }

    /**
     * Starts the process defined in the given file.
     *
     * \param[in] file_name The file which contains the process and arguments.
     * \return True if the process started otherwise false.
     */
    virtual bool Start(const std::string& file_name) {
        process.Close();

        std::string cmd = FILESYSTEM.Read(file_name);

        if (cmd.empty()) {
            return false;
        }

        std::string expanded = winss::Utils::ExpandEnvironmentVariables(cmd);
        winss::EnvironmentDir env_dir(service_dir / fs::path(kEnvDir));

        winss::ProcessParams params{ expanded, true };
        params.env = &env_dir;
        bool created = process.Create(params);
        state.pid = process.GetProcessId();
        return created;
    }

    /**
     * Starts the run file process.
     *
     * \return True if the process started otherwise false.
     */
    virtual bool StartRun() {
        if (state.remaining_count == 0) {
            // Return true to prevent false positive unable to spawn message.
            return true;
        }

        VLOG(2) << "Starting run process";

        state.up_count++;
        state.is_run_process = true;

        WINDOWS.SetEnvironmentVariable(kRunExitCodeEnvName, nullptr);
        if (Start(kRunFile)) {
            multiplexer->AddTriggeredCallback(process.GetHandle(), [&](
                winss::WaitMultiplexer& m, const winss::HandleWrapper& handle) {
                Triggered(false);
            });
            state.is_run_process = true;
            state.is_up = true;
            state.exit_code = 0;
            if (state.remaining_count > 0) {
                state.remaining_count--;
            }
            NotifyAll(RUN);
            return true;
        }

        return false;
    }

    /**
     * Starts the finish file process.
     *
     * \return True if the process started otherwise false.
     */
    virtual bool StartFinish() {
        VLOG(2) << "Starting finish process";

        state.is_run_process = false;

        WINDOWS.SetEnvironmentVariable(kRunExitCodeEnvName,
            std::to_string(state.exit_code).c_str());

        if (Start(kFinishFile)) {
            multiplexer->AddTriggeredCallback(process.GetHandle(), [&](
                winss::WaitMultiplexer& m, const winss::HandleWrapper& handle) {
                Triggered(false);
            });
            DWORD timeout = GetFinishTimeout();
            if (timeout > 0) {
                multiplexer->AddTimeoutCallback(timeout, [&](
                    winss::WaitMultiplexer&) {
                    Triggered(true);
                }, kTimeoutGroup);
                waiting = true;
            }
            state.is_up = true;
            return true;
        }

        return false;
    }

    /**
     * Notify all the listeners with the given event.
     *
     * \param[in] notification The notification event.
     */
    virtual void NotifyAll(winss::SuperviseNotification notification) {
        state.time = std::chrono::system_clock::now();

        switch (notification) {
        case RUN:
            state.last = state.time;
            break;
        case END:
            state.last = state.time;
            break;
        }

        auto it = listeners.begin();
        while (it != listeners.end()) {
            if ((*it)->Notify(notification, state)) {
                ++it;
            } else {
                it = listeners.erase(it);
            }
        }
    }

    /**
     * Event triggered handler.
     *
     * This will step the state machine forward.
     *
     * \param[in] timeout If the event was a timeout event.
     */
    virtual void Triggered(bool timeout) {
        if (waiting && !timeout) {
            multiplexer->RemoveTimeoutCallback(kTimeoutGroup);
        }

        waiting = false;
        int restart = 0;
        DWORD wait = kBusyWait;

        if (state.is_up) {
            if (state.is_run_process) {
                VLOG(2) << "Run process ended";

                state.is_up = false;
                state.pid = 0;

                NotifyAll(END);

                if (exiting) {
                    state.exit_code = kSignaledExitCode;
                } else {
                    state.exit_code = process.GetExitCode();
                }

                if (!StartFinish()) {
                    restart = 2;
                }
            } else {
                if (timeout) {
                    process.Terminate();
                    return;
                }

                VLOG(2) << "Finish process ended";

                state.is_up = false;
                state.pid = 0;

                if (process.GetExitCode() == kDownExitCode) {
                    state.remaining_count = 0;
                }

                restart = 2;
            }
        } else if (!Complete()) {
            if (!StartRun()) {
                restart = 1;
                wait = kRunFailedWait;
                LOG(WARNING) << "Unable to spawn ./run - waiting 10 seconds";
            }
        }

        if (restart > 1) {
            NotifyAll(FINISHED);
        }

        if (restart && !Complete() && state.remaining_count != 0) {
            VLOG(2) << "Waiting for: " << wait;
            waiting = true;
            multiplexer->AddTimeoutCallback(wait, [&](
                winss::WaitMultiplexer&) {
                Triggered(true);
            }, kTimeoutGroup);
        }
    }

     /**
     * Tests exiting value.
     *
     * When the exiting value is 1 it will signal the multiplexer to stop
     * and notify the listeners that the supervisor is about exit.
     *
     * \return True if exiting otherwise false.
     */
    virtual bool Complete() {
        switch (exiting) {
        case 0:
            return false;
        case 1:
            ++exiting;

            if (!multiplexer->IsStopping()) {
                multiplexer->Stop(0);
            }

            NotifyAll(EXIT);
        default:
            return true;
        }
    }

    /**
     * Signal the supervisor to exit.
     */
    virtual void Stop() {
        if (exiting) {
            return;
        }

        exiting = 1;
        Down();
    }

 public:
    static const int kMutexTaken = 100;  /**< Service dir in use error. */
    static const int kFatalExitCode = 111;  /**< Something went wrong. */
    static const int kSignaledExitCode = 256;  /**< Signaled to exit. */
    static const int kDownExitCode = 125;  /**< Signal down. */
    static const DWORD kCommandTimeout = 5000;  /**< Default timeout 5s. */
    static const DWORD kBusyWait = 1000;  /**< Busy wait 1s. */
    static const DWORD kRunFailedWait = 10000;  /**< Run failed wait 10s. */
    /** Mutex name. */
    static constexpr const char kMutexName[10] = "supervise";
    static constexpr const char kRunFile[4] = "run";  /**< Run file. */
    /** Finish file. */
    static constexpr const char kFinishFile[7] = "finish";
    static constexpr const char kDownFile[5] = "down";  /**< Down file. */
    static constexpr const char kEnvDir[4] = "env";  /**< Env directory. */
    /** Timeout finish file. */
    static constexpr const char kTimeoutFinishFile[15] = "timeout-finish";
    /** The timeout group for the multiplexer. */
    static constexpr const char kTimeoutGroup[10] = "supervise";
     /** The environment variable to set with the exit code. */
    static constexpr const char kRunExitCodeEnvName[24] =
        "SUPERVISE_RUN_EXIT_CODE";

    /**
     * Supervise constructor.
     *
     * \param multiplexer The shared multiplexer.
     * \param service_dir The service directory.
     */
    SuperviseTmpl(winss::NotOwningPtr<winss::WaitMultiplexer> multiplexer,
        const fs::path& service_dir) : multiplexer(multiplexer),
        mutex(service_dir, kMutexName), service_dir(service_dir) {
        state.is_run_process = true;
        state.is_up = false;
        state.initially_up = true;
        state.up_count = 0;
        state.remaining_count = -1;
        state.exit_code = 0;
        state.pid = 0;

        multiplexer->AddInitCallback([&](winss::WaitMultiplexer&) {
            Init();
        });

        multiplexer->AddStopCallback([&](winss::WaitMultiplexer&) {
            Stop();
        });
    }

    SuperviseTmpl(const SuperviseTmpl&) = delete;  /**< No copy. */
    SuperviseTmpl(SuperviseTmpl&&) = delete;  /**< No move. */

    /**
     * Gets the current supervisor state.
     *
     * \return The supervisor state.
     */
    virtual const SuperviseState& GetState() const {
        return state;
    }

    /**
     * Adds a supervisor listener to the list of listeners.
     *
     * \param[in] listener The listener to add.
     */
    virtual void AddListener(
        winss::NotOwningPtr<winss::SuperviseListener> listener) {
        listeners.push_back(listener);
    }

    /**
     * Signals the supervisor to go into the up state.
     */
    virtual void Up() {
        if (!mutex.HasLock() || exiting) {
            return;
        }

        VLOG(3) << "Start supervised process if not started";
        state.remaining_count = -1;
        if (!state.is_up) {
            Triggered(false);
        }
    }

    /**
     * Signals the supervisor to be in the up state and when the process
     * exits then leave it down.
     */
    virtual void Once() {
        if (!mutex.HasLock() || exiting) {
            return;
        }

        VLOG(3) << "Run supervised process once";
        if (!state.is_up) {
            state.remaining_count = 1;
            Triggered(false);
        } else {
            state.remaining_count = 0;
        }
    }

    /**
     * Signals the supervisor to only run one if it is already running.
     */
    virtual void OnceAtMost() {
        if (!mutex.HasLock() || exiting) {
            return;
        }

        VLOG(3) << "Run supervised process at least once";
        state.remaining_count = 0;
    }

    /**
     * Signals the supervisor to be in the down state.
     */
    virtual void Down() {
        if (!mutex.HasLock()) {
            return;
        }

        state.remaining_count = 0;
        Term();
    }

    /**
     * Kills the supervised process.
     */
    virtual void Kill() {
        if (!mutex.HasLock()) {
            return;
        }

        VLOG(3) << "Kill supervised process if not stopped";
        if (state.is_up && state.is_run_process) {
            process.Terminate();
        }
    }

    /**
     * Sends a CTRL+BREAK to the supervised process.
     */
    virtual void Term() {
        if (!mutex.HasLock()) {
            return;
        }

        VLOG(3) << "Stop supervised process if not stopped";
        if (state.is_up && state.is_run_process) {
            process.SendBreak();
        }
    }

    /**
     * Signals the supervisor to exit.
     */
    virtual void Exit() {
        if (!mutex.HasLock() || exiting) {
            return;
        }

        VLOG(3) << "Exit supervisor when supervised process goes down";
        state.remaining_count = 0;
        exiting = 1;

        if (!state.is_up) {
            Triggered(false);
        }
    }

    void operator=(const SuperviseTmpl&) = delete;  /**< No copy. */
    SuperviseTmpl& operator=(SuperviseTmpl&&) = delete;  /**< No move. */
};

/**
 * Concrete supervise implementation.
 */
typedef SuperviseTmpl<winss::PathMutex, winss::Process> Supervise;
}  // namespace winss

#endif  // LIB_WINSS_SUPERVISE_SUPERVISE_HPP_
