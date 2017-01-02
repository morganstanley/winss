#include "process.hpp"
#include <windows.h>
#include <vector>
#include "easylogging/easylogging++.hpp"
#include "windows_interface.hpp"
#include "environment.hpp"

enum Pipe {
    STDIN,
    STDOUT,
    STDERR
};

class ScopedSTARTUPINFO {
 private:
    STARTUPINFO startup;

 public:
    ScopedSTARTUPINFO() {
        std::memset(&startup, 0, sizeof(STARTUPINFO));
        startup.cb = sizeof(STARTUPINFO);
    }

    STARTUPINFO* Recieve() {
        return &startup;
    }

    void SetPipe(Pipe pipe, HANDLE handle) {
        startup.dwFlags |= STARTF_USESTDHANDLES;

        switch (pipe) {
        case STDIN:
            startup.hStdInput = handle;
            break;
        case STDOUT:
            startup.hStdOutput = handle;
            break;
        case STDERR:
            startup.hStdError = handle;
            break;
        }
    }

    ~ScopedSTARTUPINFO() {
        if (startup.hStdOutput != nullptr) {
            WINDOWS.CloseHandle(startup.hStdOutput);
        }
        if (startup.hStdError != nullptr) {
            WINDOWS.CloseHandle(startup.hStdError);
        }
        if (startup.hStdInput != nullptr) {
            WINDOWS.CloseHandle(startup.hStdInput);
        }
    }
};

winss::Process::Process() {
    std::memset(&proc_info, 0, sizeof(PROCESS_INFORMATION));
}

winss::Process::Process(Process&& p) : proc_info(p.proc_info) {
    std::memset(&p.proc_info, 0, sizeof(PROCESS_INFORMATION));
}

DWORD winss::Process::GetProcessId() const {
    return proc_info.dwProcessId;
}

DWORD winss::Process::GetExitCode() const {
    DWORD exitCode;
    WINDOWS.GetExitCodeProcess(proc_info.hProcess, &exitCode);
    return exitCode;
}

bool winss::Process::IsCreated() const {
    return proc_info.hProcess != nullptr;
}

bool winss::Process::IsActive() const {
    if (!IsCreated()) {
        return false;
    }

    return GetExitCode() == STILL_ACTIVE;
}

winss::HandleWrapper winss::Process::GetHandle() const {
    return winss::HandleWrapper(proc_info.hProcess, false);
}

bool winss::Process::Create(const ProcessParams& params) {
    if (IsCreated()) {
        return false;
    }

    ScopedSTARTUPINFO startup;

    if (params.stdout_pipe.HasHandle()) {
        VLOG(4)
            << "Redirecting stdout for cmd '"
            << params.cmd
            << "'";

        startup.SetPipe(STDOUT, params.stdout_pipe.Duplicate(true));
    }

    if (params.stderr_pipe.HasHandle()) {
        VLOG(4)
            << "Redirecting stderr for cmd '"
            << params.cmd
            << "'";

        startup.SetPipe(STDERR, params.stderr_pipe.Duplicate(true));
    }

    if (params.stdin_pipe.HasHandle()) {
        VLOG(4)
            << "Redirecting stdin for cmd '"
            << params.cmd
            << "'";

        startup.SetPipe(STDIN, params.stdin_pipe.Duplicate(true));
    }

    DWORD flags = 0;
    if (params.create_group) {
        VLOG(4) << "Using new process group for cmd '" << params.cmd << "'";
        flags |= CREATE_NEW_PROCESS_GROUP;
    }

    std::vector<char> env_string;
    if (params.env != nullptr) {
        env_string = params.env->ReadEnv();
    }

    LPVOID env_ptr = nullptr;
    if (!env_string.empty()) {
        VLOG(4) << "Replacing env for cmd '" << params.cmd << "'";
        env_ptr = &env_string[0];
    }

    const char* dir = nullptr;
    if (!params.dir.empty()) {
        VLOG(4)
            << "Using current directory '"
            << params.dir
            << "' for cmd '"
            << params.cmd
            << "'";
        dir = &params.dir[0];
    }

    if (!WINDOWS.CreateProcess(nullptr, const_cast<char*>(params.cmd.c_str()),
        nullptr, nullptr, true, flags, env_ptr, dir,
        startup.Recieve(), &proc_info)) {
        VLOG(1)
            << "CreateProcess('"
            << params.cmd
            << "') failed to start: "
            << WINDOWS.GetLastError();
        return false;
    }

    VLOG(3)
        << "Process for cmd '"
        << params.cmd
        << "' started with id "
        << proc_info.dwProcessId;

    /* Close all handles we don't care about */
    WINDOWS.CloseHandle(proc_info.hThread);

    return true;
}

void winss::Process::SendBreak() {
    if (IsActive()) {
        /* Send a CTRL+BREAK because CTRL+C cannot be sent */
        VLOG(3) << "Sending CTRL+BREAK to " << proc_info.dwProcessId;
        WINDOWS.GenerateConsoleCtrlEvent(
            CTRL_BREAK_EVENT, proc_info.dwProcessId);
    }
}

void winss::Process::Terminate() {
    if (IsCreated()) {
        VLOG(1) << "Terminating process id " << proc_info.dwProcessId;
        WINDOWS.TerminateProcess(proc_info.hProcess, 0);
    }
}

void winss::Process::Close() {
    if (IsCreated()) {
        VLOG(4) << "Closing handle to process id " << proc_info.dwProcessId;
        WINDOWS.CloseHandle(proc_info.hProcess);
        std::memset(&proc_info, 0, sizeof(PROCESS_INFORMATION));
    }
}

winss::Process& winss::Process::operator=(winss::Process&& p) {
    proc_info = p.proc_info;
    std::memset(&p.proc_info, 0, sizeof(PROCESS_INFORMATION));
    return *this;
}

winss::Process::~Process() {
    Close();
}
