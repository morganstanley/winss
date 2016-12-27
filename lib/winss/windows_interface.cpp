#include "windows_interface.hpp"
#include <windows.h>
#include <wincrypt.h>
#include <rpc.h>
#include <memory>

std::shared_ptr<winss::WindowsInterface> winss::WindowsInterface::instance =
    std::make_shared<winss::WindowsInterface>();

bool winss::WindowsInterface::CreateProcess(const char* application_name,
    char* command_line, SECURITY_ATTRIBUTES* proc_attr,
    SECURITY_ATTRIBUTES* thread_atttr, bool inherit_handles,
    DWORD creation_flags, void* env, const char* current_dir,
    STARTUPINFO* startup_info, PROCESS_INFORMATION* proc_info) const {
    return ::CreateProcess(application_name, command_line, proc_attr,
        thread_atttr, inherit_handles, creation_flags, env, current_dir,
        startup_info, proc_info) != 0;
}

HANDLE winss::WindowsInterface::CreateEvent(
    SECURITY_ATTRIBUTES* event_attributes, bool manual_reset,
    bool initial_state, char* name) const {
    return ::CreateEvent(event_attributes, manual_reset,
        initial_state, name);
}

bool winss::WindowsInterface::CreatePipe(PHANDLE read_pipe, PHANDLE write_pipe,
    LPSECURITY_ATTRIBUTES pipe_attr, DWORD size) const {
    return ::CreatePipe(read_pipe, write_pipe, pipe_attr, size) != 0;
}

HANDLE winss::WindowsInterface::CreateNamedPipe(char* name, DWORD open_mode,
    DWORD pipe_mode, DWORD max_instances, DWORD out_buffer_size,
    DWORD in_buffer_size, DWORD default_time_out,
    LPSECURITY_ATTRIBUTES security_attributes) const {
    return ::CreateNamedPipe(name, open_mode, pipe_mode,
        max_instances, out_buffer_size, in_buffer_size,
        default_time_out, security_attributes);
}

HANDLE winss::WindowsInterface::CreateFile(char* file_name,
    DWORD desired_access, DWORD share_mode,
    LPSECURITY_ATTRIBUTES security_attributes,
    DWORD creation_disposition, DWORD flags_and_attributes,
    HANDLE template_file) const {
    return ::CreateFile(file_name, desired_access,
        share_mode, security_attributes, creation_disposition,
        flags_and_attributes, template_file);
}

HANDLE winss::WindowsInterface::CreateMutex(LPSECURITY_ATTRIBUTES attributes,
    bool initial_owner, LPCTSTR name) const {
    return ::CreateMutex(attributes, initial_owner, name);
}

HANDLE winss::WindowsInterface::OpenMutex(DWORD desired_access, bool inherit,
    LPCTSTR name) const {
    return ::OpenMutex(desired_access, inherit, name);
}

bool winss::WindowsInterface::SetEvent(HANDLE handle) const {
    return ::SetEvent(handle) != 0;
}

bool winss::WindowsInterface::ResetEvent(HANDLE handle) const {
    return ::ResetEvent(handle) != 0;
}

bool winss::WindowsInterface::ConnectNamedPipe(HANDLE handle,
    LPOVERLAPPED overlapped) const {
    return ::ConnectNamedPipe(handle, overlapped) != 0;
}

bool winss::WindowsInterface::DisconnectNamedPipe(HANDLE handle) const {
    return ::DisconnectNamedPipe(handle) != 0;
}

bool winss::WindowsInterface::GetOverlappedResult(HANDLE handle,
    LPOVERLAPPED overlapped, LPDWORD bytes, BOOL wait) const {
    return ::GetOverlappedResult(handle, overlapped, bytes, wait) != 0;
}

bool winss::WindowsInterface::ReadFile(HANDLE handle, LPVOID buffer,
    DWORD to_read, LPDWORD read, LPOVERLAPPED overlapped) const {
    return ::ReadFile(handle, buffer, to_read, read, overlapped) != 0;
}

bool winss::WindowsInterface::WriteFile(HANDLE handle, LPCVOID buffer,
    DWORD to_write, LPDWORD written, LPOVERLAPPED overlapped) const {
    return ::WriteFile(handle, buffer, to_write, written, overlapped) != 0;
}

DWORD winss::WindowsInterface::WaitForSingleObject(
    HANDLE handle, DWORD timeout) const {
    return ::WaitForSingleObject(handle, timeout);
}

DWORD winss::WindowsInterface::WaitForMultipleObjects(DWORD handles_count,
    const HANDLE* handles, bool wait_all, DWORD timeout) const {
    return ::WaitForMultipleObjects(handles_count, handles, wait_all, timeout);
}

bool winss::WindowsInterface::DuplicateHandle(HANDLE source_process_handle,
    HANDLE source_handle, HANDLE target_process_handle, LPHANDLE target_handle,
    DWORD desired_access, bool inherit_handle, DWORD options) const {
    return ::DuplicateHandle(source_process_handle, source_handle,
        target_process_handle, target_handle, desired_access,
        inherit_handle, options) != 0;
}

bool winss::WindowsInterface::SetHandleInformation(HANDLE object, DWORD mask,
    DWORD flags) const {
    return ::SetHandleInformation(object, mask, flags) != 0;
}

HANDLE winss::WindowsInterface::GetStdHandle(DWORD std_handle) const {
    return ::GetStdHandle(std_handle);
}

bool winss::WindowsInterface::ReleaseMutex(HANDLE mutex) const {
    return ::ReleaseMutex(mutex) != 0;
}

bool winss::WindowsInterface::CloseHandle(HANDLE handle) const {
    return ::CloseHandle(handle) != 0;
}

bool winss::WindowsInterface::GenerateConsoleCtrlEvent(
    DWORD ctrl_event, DWORD proc_group_id) const {
    return ::GenerateConsoleCtrlEvent(ctrl_event, proc_group_id) != 0;
}

bool winss::WindowsInterface::GetExitCodeProcess(
    HANDLE handle, DWORD* exit_code) const {
    return ::GetExitCodeProcess(handle, exit_code) != 0;
}

HANDLE winss::WindowsInterface::GetCurrentProcess() const {
    return ::GetCurrentProcess();
}

bool winss::WindowsInterface::TerminateProcess(
    HANDLE process, UINT exit_code) const {
    return ::TerminateProcess(process, exit_code) != 0;
}

DWORD winss::WindowsInterface::GetLastError() const {
    return ::GetLastError();
}

bool winss::WindowsInterface::SetEnvironmentVariable(LPCTSTR name,
    LPCTSTR value) const {
    return ::SetEnvironmentVariable(name, value) != 0;
}

DWORD winss::WindowsInterface::GetEnvironmentVariable(LPCTSTR name,
    LPTSTR buffer, DWORD size) const {
    return ::GetEnvironmentVariable(name, buffer, size);
}

DWORD winss::WindowsInterface::ExpandEnvironmentStrings(LPCTSTR src,
    LPTSTR dst, DWORD size) const {
    return ::ExpandEnvironmentStrings(src, dst, size);
}

LPTCH winss::WindowsInterface::GetEnvironmentStrings() const {
    return ::GetEnvironmentStrings();
}

bool winss::WindowsInterface::FreeEnvironmentStrings(
    LPTCH lpszEnvironmentBlock) const {
    return ::FreeEnvironmentStrings(lpszEnvironmentBlock) != 0;
}

bool winss::WindowsInterface::SetConsoleCtrlHandler(
    PHANDLER_ROUTINE handler, bool add) const {
    return ::SetConsoleCtrlHandler(handler, add) != 0;
}

RPC_STATUS winss::WindowsInterface::UuidCreateSequential(GUID* uuid) const {
    return ::UuidCreateSequential(uuid);
}

RPC_STATUS winss::WindowsInterface::UuidCreateNil(GUID* uuid) const {
    return ::UuidCreateNil(uuid);
}

RPC_STATUS winss::WindowsInterface::UuidCompare(GUID* uuid1, GUID* uuid2,
    RPC_STATUS* status) const {
    return ::UuidCompare(uuid1, uuid2, status);
}

RPC_STATUS winss::WindowsInterface::UuidToString(const GUID *uuid,
    unsigned char** string_uuid) const {
    return ::UuidToString(uuid, string_uuid);
}

RPC_STATUS winss::WindowsInterface::UuidFromString(unsigned char* string_uuid,
    GUID *uuid) const {
    return ::UuidFromString(string_uuid, uuid);
}

RPC_STATUS winss::WindowsInterface::RpcStringFree(
    unsigned char** rpc_string) const {
    return ::RpcStringFree(rpc_string);
}

bool winss::WindowsInterface::CryptAcquireContext(HCRYPTPROV* csp,
    LPCTSTR container, LPCTSTR provider, DWORD type, DWORD flags) const {
    return ::CryptAcquireContext(csp, container, provider, type, flags) != 0;
}

bool winss::WindowsInterface::CryptCreateHash(HCRYPTPROV csp, ALG_ID algorithm,
    HCRYPTKEY key, DWORD flags, HCRYPTHASH* hash) const {
    return ::CryptCreateHash(csp, algorithm, key, flags, hash) != 0;
}

bool winss::WindowsInterface::CryptHashData(HCRYPTHASH hash,
    unsigned char* data, DWORD length, DWORD flags) const {
    return ::CryptHashData(hash, data, length, flags) != 0;
}

bool winss::WindowsInterface::CryptGetHashParam(HCRYPTHASH hash, DWORD param,
    unsigned char* data, DWORD* length, DWORD flags) const {
    return ::CryptGetHashParam(hash, param, data, length, flags) != 0;
}

bool winss::WindowsInterface::CryptDestroyHash(HCRYPTHASH hash) const {
    return ::CryptDestroyHash(hash) != 0;
}

bool winss::WindowsInterface::CryptReleaseContext(HCRYPTPROV csp,
    DWORD flags) const {
    return ::CryptReleaseContext(csp, flags) != 0;
}

const winss::WindowsInterface& winss::WindowsInterface::GetInstance() {
    if (!winss::WindowsInterface::instance) {
        winss::WindowsInterface::instance =
            std::make_shared<WindowsInterface>();
    }

    return *winss::WindowsInterface::instance;
}
