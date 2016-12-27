#ifndef LIB_WINSS_PIPE_INSTANCE_HPP_
#define LIB_WINSS_PIPE_INSTANCE_HPP_

#include <windows.h>
#include <vector>
#include <queue>
#include "handle_wrapper.hpp"
#include "pipe_name.hpp"

namespace winss {
enum OverlappedResult {
    REMOVE,
    CONTINUE,
    SKIP
};
class PipeInstance {
 protected:
    OVERLAPPED overlapped;
    HANDLE handle = nullptr;
    bool connected = false;
    bool pending_io = false;
    bool close = false;
    std::vector<char> buffer;
    DWORD bytes = 0;

    bool CheckError();

 public:
    static const DWORD kBufferSize = 4096;
    static const DWORD kTimeout = 5000;

    PipeInstance();
    PipeInstance(const PipeInstance&) = delete;
    PipeInstance(PipeInstance&& instance);

    virtual bool IsPendingIO() const;
    virtual bool IsConnected() const;
    virtual bool IsClosing() const;
    virtual winss::HandleWrapper GetHandle() const;
    virtual bool SetConnected();
    virtual OverlappedResult GetOverlappedResult();

    virtual bool CreateNamedPipe(const winss::PipeName& pipe_name);
    virtual bool CreateFile(const winss::PipeName& pipe_name);

    virtual void Closing();
    virtual void DisconnectNamedPipe();
    virtual bool Close();

    void operator=(const PipeInstance&) = delete;
    PipeInstance& operator=(PipeInstance&& instance);

    virtual ~PipeInstance();
};
class OutboundPipeInstance : public PipeInstance {
 private:
    bool writting = false;
    std::queue<std::vector<char>> message_queue;

 public:
    OutboundPipeInstance();
    OutboundPipeInstance(const OutboundPipeInstance&) = delete;
    OutboundPipeInstance(OutboundPipeInstance&& instance);

    bool Queue(const std::vector<char>& data);
    bool HasMessages() const;
    bool IsWriting() const;
    bool FinishWrite();
    bool Write();
    void Read();

    void operator=(const OutboundPipeInstance&) = delete;
    OutboundPipeInstance& operator=(OutboundPipeInstance&& instance);
};
class InboundPipeInstance : public PipeInstance {
 public:
    InboundPipeInstance();
    InboundPipeInstance(const InboundPipeInstance&) = delete;
    InboundPipeInstance(InboundPipeInstance&& instance);

    bool FinishRead();
    void Read();
    std::vector<char> SwapBuffer();

    void operator=(const InboundPipeInstance&) = delete;
    InboundPipeInstance& operator=(InboundPipeInstance&& instance);
};
}  // namespace winss

#endif  // LIB_WINSS_PIPE_INSTANCE_HPP_
