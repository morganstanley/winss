#ifndef LIB_WINSS_PROCESS_HPP_
#define LIB_WINSS_PROCESS_HPP_

#include <windows.h>
#include <string>
#include "handle_wrapper.hpp"
#include "environment.hpp"

namespace winss {
struct ProcessParams {
    std::string cmd;
    bool create_group;
    std::string dir;
    winss::HandleWrapper stdout_pipe;
    winss::HandleWrapper stderr_pipe;
    winss::HandleWrapper stdin_pipe;
    winss::Environment* env;
};
class Process {
 private:
    PROCESS_INFORMATION proc_info;

 public:
    Process();
    Process(const Process&) = delete;
    Process(Process&& p);

    virtual DWORD GetProcessId() const;
    virtual DWORD GetExitCode() const;
    virtual bool IsCreated() const;
    virtual bool IsActive() const;
    virtual winss::HandleWrapper GetHandle() const;

    virtual bool Create(const ProcessParams& params);
    virtual void SendBreak();
    virtual void Terminate();
    virtual void Close();

    void operator=(const Process&) = delete;
    Process& operator=(Process&& p);

    ~Process();
};
}  // namespace winss

#endif  // LIB_WINSS_PROCESS_HPP_
