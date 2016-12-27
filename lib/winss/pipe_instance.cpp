#include "pipe_instance.hpp"
#include <windows.h>
#include <easylogging/easylogging++.hpp>
#include <vector>
#include <queue>
#include <utility>
#include <string>
#include "windows_interface.hpp"
#include "handle_wrapper.hpp"
#include "pipe_name.hpp"

winss::PipeInstance::PipeInstance() {
    buffer.resize(kBufferSize);
    std::memset(&overlapped, 0, sizeof(OVERLAPPED));
    overlapped.hEvent = WINDOWS.CreateEvent(
        nullptr, true, true, nullptr);
}

winss::PipeInstance::PipeInstance(winss::PipeInstance&& instance) {
    overlapped = instance.overlapped;
    std::memset(&instance.overlapped, 0, sizeof(OVERLAPPED));

    handle = instance.handle;
    instance.handle = nullptr;

    connected = instance.connected;
    instance.connected = false;
    pending_io = instance.pending_io;
    instance.pending_io = false;
    close = instance.close;
    instance.close = false;
    buffer.resize(kBufferSize);
    buffer.swap(instance.buffer);
    bytes = instance.bytes;
    instance.bytes = 0;
}

bool winss::PipeInstance::IsPendingIO() const {
    return pending_io;
}

bool winss::PipeInstance::IsConnected() const {
    return connected;
}

bool winss::PipeInstance::IsClosing() const {
    return close;
}

winss::HandleWrapper winss::PipeInstance::GetHandle() const {
    return winss::HandleWrapper(overlapped.hEvent, false);
}

bool winss::PipeInstance::SetConnected() {
    if (handle == nullptr || connected) {
        return false;
    }

    VLOG(5) << "Connected pipe instance: " << handle;
    connected = true;
    return true;
}

bool winss::PipeInstance::CheckError() {
    if (handle == nullptr) {
        return false;
    }

    DWORD error = WINDOWS.GetLastError();
    if (error != ERROR_IO_PENDING) {
        if (error == ERROR_BROKEN_PIPE) {
            VLOG(5) << "Pipe " << handle << " closed";
        } else {
            VLOG(1) << "overlapped failed: " << error << " (closing)";
        }
        close = true;
        WINDOWS.SetEvent(overlapped.hEvent);
        return false;
    } else {
        pending_io = true;
    }

    return true;
}

winss::OverlappedResult winss::PipeInstance::GetOverlappedResult() {
    if (handle == nullptr) {
        return REMOVE;
    }

    if (close) {
        WINDOWS.ResetEvent(overlapped.hEvent);
        return REMOVE;
    }

    if (!pending_io) {
        WINDOWS.ResetEvent(overlapped.hEvent);
        return CONTINUE;
    }

    bytes = 0;
    pending_io = false;

    if (!WINDOWS.GetOverlappedResult(handle, &overlapped, &bytes, FALSE)) {
        DWORD error = WINDOWS.GetLastError();
        if (error == ERROR_IO_INCOMPLETE) {
            VLOG(6)
                << "Skipping result for instance: "
                << handle;

            return SKIP;
        } else if (error == ERROR_BROKEN_PIPE) {
            VLOG(5) << "Pipe " << handle << " closed";
            return REMOVE;
        }

        VLOG(1) << "GetOverlappedResult failed: " << error;
        return REMOVE;
    }

    return CONTINUE;
}

bool winss::PipeInstance::CreateNamedPipe(const winss::PipeName& pipe_name) {
    if (handle != nullptr) {
        return false;
    }

    VLOG(6) << "Creating named pipe instance";

    handle = WINDOWS.CreateNamedPipe(
        const_cast<char*>(pipe_name.Get().c_str()),
        FILE_FLAG_OVERLAPPED | PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_BYTE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES, kBufferSize, kBufferSize, kTimeout, nullptr);

    if (handle == INVALID_HANDLE_VALUE) {
        VLOG(1) << "CreateNamedPipe failed: " << WINDOWS.GetLastError();
        return false;
    }

    VLOG(5) << "Connecting pipe instance: " << handle;

    if (WINDOWS.ConnectNamedPipe(handle, &overlapped)) {
        VLOG(1) << "ConnectNamedPipe should be overlapped";
        return false;
    }

    pending_io = false;

    DWORD error = WINDOWS.GetLastError();
    switch (error) {
    case ERROR_IO_PENDING:
        // The overlapped connection in progress.
        pending_io = true;
        return true;
    case ERROR_PIPE_CONNECTED:
        // Client is already connected, so signal an event.
        if (WINDOWS.SetEvent(overlapped.hEvent))
            return true;
    default:
        VLOG(1) << "ConnectNamedPipe failed: " << error;
        return false;
    }
}

bool winss::PipeInstance::CreateFile(const winss::PipeName& pipe_name) {
    if (handle != nullptr) {
        return false;
    }

    handle = WINDOWS.CreateFile(
        const_cast<char*>(pipe_name.Get().c_str()),
        GENERIC_READ | GENERIC_WRITE, 0, nullptr,
        OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);

    if (handle == INVALID_HANDLE_VALUE) {
        VLOG(1)
            << "CreateFile failed: "
            << WINDOWS.GetLastError();
        return false;
    }

    return true;
}

void winss::PipeInstance::Closing() {
    if (handle != nullptr && !close) {
        VLOG(6) << "Signalling close of pipe instance: " << handle;
        close = true;
        WINDOWS.SetEvent(overlapped.hEvent);
    }
}

void winss::PipeInstance::DisconnectNamedPipe() {
    if (handle != nullptr && connected) {
        VLOG(5) << "Disconnecting pipe instance: " << handle;
        WINDOWS.DisconnectNamedPipe(handle);
        connected = false;
    }
}

bool winss::PipeInstance::Close() {
    if (handle != nullptr && handle != INVALID_HANDLE_VALUE) {
        WINDOWS.CloseHandle(handle);
        VLOG(5) << "Closed pipe instance: " << handle;

        if (overlapped.hEvent != nullptr &&
            overlapped.hEvent != INVALID_HANDLE_VALUE) {
            WINDOWS.CloseHandle(overlapped.hEvent);
        }

        handle = nullptr;
        std::memset(&overlapped, 0, sizeof(OVERLAPPED));
        connected = false;
        pending_io = false;
        close = false;
        buffer.clear();
        bytes = 0;

        return true;
    }

    return false;
}

winss::PipeInstance& winss::PipeInstance::operator=(
    winss::PipeInstance&& instance) {
    overlapped = instance.overlapped;
    std::memset(&instance.overlapped, 0, sizeof(OVERLAPPED));

    handle = instance.handle;
    instance.handle = nullptr;

    connected = instance.connected;
    instance.connected = false;
    pending_io = instance.pending_io;
    instance.pending_io = false;
    close = instance.close;
    instance.close = false;
    buffer.swap(instance.buffer);
    bytes = instance.bytes;
    instance.bytes = 0;
    return *this;
}

winss::PipeInstance::~PipeInstance() {
    Close();
}

winss::OutboundPipeInstance::OutboundPipeInstance() :
    winss::PipeInstance::PipeInstance() {}

winss::OutboundPipeInstance::OutboundPipeInstance(
    winss::OutboundPipeInstance&& instance) :
    winss::PipeInstance::PipeInstance(std::move(instance)) {
    message_queue = std::move(instance.message_queue);
}

bool winss::OutboundPipeInstance::Queue(const std::vector<char>& data) {
    if (!connected || data.size() == 0) {
        return false;
    }

    size_t parts = data.size() / kBufferSize;
    auto start = data.begin();
    for (size_t i = 0; i < parts; i++) {
        auto end = start + kBufferSize;
        std::vector<char> data(start, end);
        message_queue.push(std::move(data));
        start = end;
    }

    if (start != data.end()) {
        std::vector<char> data(start, data.end());
        message_queue.push(std::move(data));
    }

    return true;
}

bool winss::OutboundPipeInstance::HasMessages() const {
    return !message_queue.empty();
}

bool winss::OutboundPipeInstance::IsWriting() const {
    return writting;
}

bool winss::OutboundPipeInstance::FinishWrite() {
    if (!connected) {
        return false;
    }

    if (!message_queue.empty()) {
        message_queue.pop();
        return !message_queue.empty();
    }

    return false;
}

bool winss::OutboundPipeInstance::Write() {
    if (!connected || message_queue.empty()) {
        return false;
    }

    writting = true;
    pending_io = false;

    const std::vector<char>& write_buffer = message_queue.front();

    VLOG(5) << "Sending " << write_buffer.size() << " bytes";

    bool success = WINDOWS.WriteFile(handle, &write_buffer[0],
        static_cast<DWORD>(write_buffer.size()), &bytes,
        &overlapped);

    if (success) {
        WINDOWS.SetEvent(overlapped.hEvent);
        return true;
    } else {
        return CheckError();
    }
}

void winss::OutboundPipeInstance::Read() {
    if (!connected || close) {
        return;
    }

    writting = false;
    pending_io = false;

    bool success = WINDOWS.ReadFile(handle, &buffer[0], 0, &bytes,
        &overlapped);

    if (success) {
        WINDOWS.SetEvent(overlapped.hEvent);
    } else {
        CheckError();
    }
}

winss::OutboundPipeInstance& winss::OutboundPipeInstance::operator=(
    winss::OutboundPipeInstance&& instance) {
    winss::PipeInstance::operator=(std::move(instance));
    writting = instance.writting;
    instance.writting = false;
    message_queue = std::move(instance.message_queue);
    return *this;
}

winss::InboundPipeInstance::InboundPipeInstance() :
    winss::PipeInstance::PipeInstance() {}

winss::InboundPipeInstance::InboundPipeInstance(
    winss::InboundPipeInstance&& instance) :
    winss::PipeInstance::PipeInstance(std::move(instance)) {
}

bool winss::InboundPipeInstance::FinishRead() {
    if (connected && bytes != 0) {
        VLOG(5) << "Recieved " << bytes << " bytes";
        pending_io = false;
        return true;
    }

    return false;
}

void winss::InboundPipeInstance::Read() {
    if (!connected || close) {
        return;
    }

    pending_io = false;

    bool success = WINDOWS.ReadFile(handle, &buffer[0],
        static_cast<DWORD>(buffer.size()), &bytes, &overlapped);

    if (success) {
        WINDOWS.SetEvent(overlapped.hEvent);
    } else {
        CheckError();
    }
}

std::vector<char> winss::InboundPipeInstance::SwapBuffer() {
    std::vector<char> buff;
    buff.resize(kBufferSize);

    if (connected && bytes > 0) {
        buff.swap(buffer);
        buff.resize(bytes);
    }

    return buff;
}

winss::InboundPipeInstance& winss::InboundPipeInstance::operator=(
    winss::InboundPipeInstance&& instance) {
    winss::PipeInstance::operator=(std::move(instance));
    return *this;
}
