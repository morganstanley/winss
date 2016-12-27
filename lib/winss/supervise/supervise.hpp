#ifndef LIB_WINSS_SUPERVISE_SUPERVISE_HPP_
#define LIB_WINSS_SUPERVISE_SUPERVISE_HPP_

#include <windows.h>
#include <easylogging/easylogging++.hpp>
#include <winss/handle_wrapper.hpp>
#include <winss/windows_interface.hpp>
#include <winss/filesystem_interface.hpp>
#include <winss/wait_multiplexer.hpp>
#include <winss/not_owning_ptr.hpp>
#include <winss/environment.hpp>
#include <winss/path_mutex.hpp>
#include <winss/process.hpp>
#include <winss/utils.hpp>
#include <filesystem>
#include <vector>
#include <chrono>
#include <string>
#include <set>

namespace fs = std::experimental::filesystem;

namespace winss {
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
enum SuperviseNotification {
    UNKNOWN,
    START,
    RUN,
    END,
    FINISHED,
    EXIT
};
class SuperviseListener {
 public:
    virtual bool Notify(SuperviseNotification notification,
        const SuperviseState& state) = 0;
    virtual ~SuperviseListener() {}
};
template<typename TMutex, typename TProcess>
class SuperviseTmpl {
 protected:
    winss::NotOwningPtr<winss::WaitMultiplexer> multiplexer;
    TMutex mutex;
    TProcess process;
    fs::path service_dir;
    SuperviseState state{};
    std::vector<winss::NotOwningPtr<winss::SuperviseListener>> listeners;
    int exiting = 0;
    bool waiting = false;

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

    virtual DWORD GetFinishTimeout() const {
        std::string timeout_finish = FILESYSTEM.Read(kTimeoutFinishFile);

        if (timeout_finish.empty()) {
            return kCommandTimeout;
        }

        return std::strtoul(timeout_finish.c_str(), nullptr, 10);
    }

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

    virtual bool StartRun() {
        if (state.remaining_count == 0) {
            return false;
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
            multiplexer->AddTimeoutCallback(GetFinishTimeout(), [&](
                winss::WaitMultiplexer&) {
                Triggered(true);
            }, kTimeoutGroup);
            waiting = true;
            state.is_up = true;
            return true;
        }

        return false;
    }

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

    virtual void Triggered(bool timeout) {
        if (waiting && !timeout) {
            multiplexer->RemoveTimeoutCallback(kTimeoutGroup);
        }

        waiting = false;
        int restart = 0;

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
                } else {
                    VLOG(2) << "Finish process ended";

                    state.is_up = false;
                    state.pid = 0;
                }

                restart = 2;
            }
        } else if (!Complete()) {
            if (!StartRun()) {
                restart = 1;
            }
        }

        if (restart > 1) {
            NotifyAll(FINISHED);
        }

        if (restart && !Complete() && state.remaining_count != 0) {
            VLOG(2) << "Waiting for: " << kBusyWait;
            waiting = true;
            multiplexer->AddTimeoutCallback(kBusyWait, [&](
                winss::WaitMultiplexer&) {
                Triggered(true);
            }, kTimeoutGroup);
        }
    }

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

    virtual void Stop() {
        if (exiting) {
            return;
        }

        exiting = 1;
        Down();
    }

 public:
    static const int kMutexTaken = 100;
    static const int kFatalExitCode = 111;
    static const int kSignaledExitCode = 256;
    static const DWORD kCommandTimeout = 5000;  // 5 seconds
    static const DWORD kBusyWait = 1000;  // 1 second
    static constexpr const char kMutexName[10] = "supervise";
    static constexpr const char kRunFile[4] = "run";
    static constexpr const char kFinishFile[7] = "finish";
    static constexpr const char kDownFile[5] = "down";
    static constexpr const char kEnvDir[4] = "env";
    static constexpr const char kTimeoutFinishFile[15] = "timeout-finish";
    static constexpr const char kTimeoutGroup[10] = "supervise";
    static constexpr const char kRunExitCodeEnvName[24] =
        "SUPERVISE_RUN_EXIT_CODE";

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

    SuperviseTmpl(const SuperviseTmpl&) = delete;
    SuperviseTmpl(SuperviseTmpl&&) = delete;

    virtual const SuperviseState& GetState() const {
        return state;
    }

    virtual void AddListener(
        winss::NotOwningPtr<winss::SuperviseListener> listener) {
        listeners.push_back(listener);
    }

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

    virtual void OnceAtMost() {
        if (!mutex.HasLock() || exiting) {
            return;
        }

        VLOG(3) << "Run supervised process at least once";
        state.remaining_count = 0;
    }

    virtual void Down() {
        if (!mutex.HasLock()) {
            return;
        }

        state.remaining_count = 0;
        Term();
    }

    virtual void Kill() {
        if (!mutex.HasLock()) {
            return;
        }

        VLOG(3) << "Kill supervised process if not stopped";
        if (state.is_up && state.is_run_process) {
            process.Terminate();
        }
    }

    virtual void Term() {
        if (!mutex.HasLock()) {
            return;
        }

        VLOG(3) << "Stop supervised process if not stopped";
        if (state.is_up && state.is_run_process) {
            process.SendBreak();
        }
    }

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

    void operator=(const SuperviseTmpl&) = delete;
    SuperviseTmpl& operator=(SuperviseTmpl&&) = delete;
};
typedef SuperviseTmpl<winss::PathMutex, winss::Process> Supervise;
}  // namespace winss

#endif  // LIB_WINSS_SUPERVISE_SUPERVISE_HPP_
