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

#ifndef LIB_WINSS_WINDOWS_INTERFACE_HPP_
#define LIB_WINSS_WINDOWS_INTERFACE_HPP_

#include <windows.h>
#include <wincrypt.h>
#include <rpc.h>
#include <memory>

#define WINDOWS winss::WindowsInterface::GetInstance()

namespace winss {
/**
 * Wraps the windows functions as an interface so that they can be easily
 * mocked and tested.
 */
class WindowsInterface {
 protected:
    /**
     * A singleton Windows instance.
     */
    static std::shared_ptr<WindowsInterface> instance;

 public:
    /**
     * Create a new Windows interface.
     */
    WindowsInterface() {}
    WindowsInterface(const WindowsInterface&) = delete;  /**< No copy. */
    WindowsInterface(WindowsInterface&&) = delete;  /**< No move. */

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/ms682425.aspx">CreateProcess</a>
     */
    virtual bool CreateProcess(const char* application_name,
        char* command_line, SECURITY_ATTRIBUTES* proc_attr,
        SECURITY_ATTRIBUTES* thread_atttr, bool inherit_handles,
        DWORD creation_flags, void* env, const char* current_dir,
        STARTUPINFO* startup_info, PROCESS_INFORMATION* proc_info) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/ms682396.aspx">CreateEvent</a>
     */
    virtual HANDLE CreateEvent(SECURITY_ATTRIBUTES* event_attributes,
        bool manual_reset, bool initial_state, char* name) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/aa365152.aspx">CreatePipe</a>
     */
    virtual bool CreatePipe(PHANDLE read_pipe, PHANDLE write_pipe,
        LPSECURITY_ATTRIBUTES pipe_attr, DWORD size) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/aa365150.aspx">CreateNamedPipe</a>
     */
    virtual HANDLE CreateNamedPipe(char* name, DWORD open_mode,
        DWORD pipe_mode, DWORD max_instances, DWORD out_buffer_size,
        DWORD in_buffer_size, DWORD default_time_out,
        LPSECURITY_ATTRIBUTES security_attributes) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/aa363858.aspx">CreateFile</a>
     */
    virtual HANDLE CreateFile(char* file_name, DWORD desired_access,
        DWORD share_mode, LPSECURITY_ATTRIBUTES security_attributes,
        DWORD creation_disposition, DWORD flags_and_attributes,
        HANDLE template_file) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/ms682411.aspx">CreateMutex</a>
     */
    virtual HANDLE CreateMutex(LPSECURITY_ATTRIBUTES attributes,
        bool initial_owner, LPCTSTR name) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/ms684315.aspx">OpenMutex</a>
     */
    virtual HANDLE OpenMutex(DWORD desired_access, bool inherit,
        LPCTSTR name) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/ms686211.aspx">SetEvent</a>
     */
    virtual bool SetEvent(HANDLE handle) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/ms685081.aspx">ResetEvent</a>
     */
    virtual bool ResetEvent(HANDLE handle) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/aa365146.aspx">ConnectNamedPipe</a>
     */
    virtual bool ConnectNamedPipe(HANDLE handle,
        LPOVERLAPPED overlapped) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/aa365166.aspx">ConnectNamedPipe</a>
     */
    virtual bool DisconnectNamedPipe(HANDLE handle) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/ms683209.aspx">GetOverlappedResult</a>
     */
    virtual bool GetOverlappedResult(HANDLE handle, LPOVERLAPPED overlapped,
        LPDWORD bytes, BOOL wait) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/aa365467.aspx">ReadFile</a>
     */
    virtual bool ReadFile(HANDLE handle, LPVOID buffer, DWORD to_read,
        LPDWORD read, LPOVERLAPPED overlapped) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/aa365747.aspx">WriteFile</a>
     */
    virtual bool WriteFile(HANDLE handle, LPCVOID buffer, DWORD to_write,
        LPDWORD written, LPOVERLAPPED overlapped) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/ms687032.aspx">WaitForSingleObject</a>
     */
    virtual DWORD WaitForSingleObject(HANDLE handle, DWORD timeout) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/ms687025.aspx">WaitForMultipleObjects</a>
     */
    virtual DWORD WaitForMultipleObjects(DWORD handles_count,
        const HANDLE* handles, bool wait_all, DWORD timeout) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/ms724251.aspx">DuplicateHandle</a>
     */
    virtual bool DuplicateHandle(HANDLE source_process_handle,
        HANDLE source_handle, HANDLE target_process_handle,
        LPHANDLE target_handle, DWORD desired_access,
        bool inherit_handle,
        DWORD options) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/ms724935.aspx">SetHandleInformation</a>
     */
    virtual bool SetHandleInformation(HANDLE object, DWORD mask,
        DWORD flags) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/ms683231.aspx">GetStdHandle</a>
     */
    virtual HANDLE GetStdHandle(DWORD std_handle) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/ms685066.aspx">ReleaseMutex</a>
     */
    virtual bool ReleaseMutex(HANDLE mutex) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/ms724211.aspx">CloseHandle</a>
     */
    virtual bool CloseHandle(HANDLE handle) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/ms683155.aspx">GenerateConsoleCtrlEvent</a>
     */
    virtual bool GenerateConsoleCtrlEvent(DWORD ctrl_event,
        DWORD proc_group_id) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/ms683189.aspx">GetExitCodeProcess</a>
     */
    virtual bool GetExitCodeProcess(HANDLE handle, DWORD* exit_code) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/ms683179.aspx">GetCurrentProcess</a>
     */
    virtual HANDLE GetCurrentProcess() const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/ms686714.aspx">TerminateProcess</a>
     */
    virtual bool TerminateProcess(HANDLE process, UINT exit_code) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/ms679360.aspx">GetLastError</a>
     */
    virtual DWORD GetLastError() const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/ms686206.aspx">SetEnvironmentVariable</a>
     */
    virtual bool SetEnvironmentVariable(LPCTSTR name, LPCTSTR value) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/ms683188.aspx">GetEnvironmentVariable</a>
     */
    virtual DWORD GetEnvironmentVariable(LPCTSTR name, LPTSTR buffer,
        DWORD size) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/ms724265.aspx">ExpandEnvironmentStrings</a>
     */
    virtual DWORD ExpandEnvironmentStrings(LPCTSTR src, LPTSTR dst,
        DWORD size) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/ms683187.aspx">GetEnvironmentStrings</a>
     */
    virtual LPTCH GetEnvironmentStrings() const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/ms683151.aspx">FreeEnvironmentStrings</a>
     */
    virtual bool FreeEnvironmentStrings(LPTCH lpszEnvironmentBlock) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/ms686016.aspx">SetConsoleCtrlHandler</a>
     */
    virtual bool SetConsoleCtrlHandler(PHANDLER_ROUTINE handler,
        bool add) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/aa379322.aspx">UuidCreateSequential</a>
     */
    virtual RPC_STATUS UuidCreateSequential(GUID* uuid) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/aa379262.aspx">UuidCreateNil</a>
     */
    virtual RPC_STATUS UuidCreateNil(GUID* uuid) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/aa379201.aspx">UuidCompare</a>
     */
    virtual RPC_STATUS UuidCompare(GUID* uuid1, GUID* uuid2,
        RPC_STATUS* status) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/aa379352.aspx">UuidToString</a>
     */
    virtual RPC_STATUS UuidToString(const GUID* uuid,
        unsigned char** string_uuid) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/aa379336.aspx">UuidFromString</a>
     */
    virtual RPC_STATUS UuidFromString(unsigned char* string_uuid,
        GUID* uuid) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/aa378483.aspx">RpcStringFree</a>
     */
    virtual RPC_STATUS RpcStringFree(unsigned char** rpc_string) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/aa379886.aspx">CryptAcquireContext</a>
     */
    virtual bool CryptAcquireContext(HCRYPTPROV* csp, LPCTSTR container,
        LPCTSTR provider, DWORD type, DWORD flags) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/aa379908.aspx">CryptCreateHash</a>
     */
    virtual bool CryptCreateHash(HCRYPTPROV csp, ALG_ID algorithm,
        HCRYPTKEY key, DWORD flags, HCRYPTHASH* hash) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/aa380202.aspx">CryptHashData</a>
     */
    virtual bool CryptHashData(HCRYPTHASH hash, unsigned char* data,
        DWORD length, DWORD flags) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/aa379947.aspx">CryptGetHashParam</a>
     */
    virtual bool CryptGetHashParam(HCRYPTHASH hash, DWORD param,
        unsigned char* data, DWORD* length, DWORD flags) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/aa379917.aspx">CryptDestroyHash</a>
     */
    virtual bool CryptDestroyHash(HCRYPTHASH hash) const;

    /**
     * <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/aa380268.aspx">CryptReleaseContext</a>
     */
    virtual bool CryptReleaseContext(HCRYPTPROV csp, DWORD flags) const;

    /**
     * Gets the Wdinows interface instance.
     *
     * \return The singleton Windows interface.
     */
    static const WindowsInterface& GetInstance();

    /** No copy. */
    WindowsInterface& operator=(const WindowsInterface&) = delete;
    /** No move. */
    WindowsInterface& operator=(WindowsInterface&&) = delete;
};
}  // namespace winss

#endif  // LIB_WINSS_WINDOWS_INTERFACE_HPP_
