#ifndef LIB_WINSS_WINDOWS_INTERFACE_HPP_
#define LIB_WINSS_WINDOWS_INTERFACE_HPP_

#include <windows.h>
#include <wincrypt.h>
#include <rpc.h>
#include <memory>

#define WINDOWS winss::WindowsInterface::GetInstance()

namespace winss {
class WindowsInterface {
 protected:
    static std::shared_ptr<WindowsInterface> instance;

 public:
    WindowsInterface() {}
    WindowsInterface(const WindowsInterface&) = delete;
    WindowsInterface(WindowsInterface&&) = delete;

    virtual bool CreateProcess(const char* application_name,
        char* command_line, SECURITY_ATTRIBUTES* proc_attr,
        SECURITY_ATTRIBUTES* thread_atttr, bool inherit_handles,
        DWORD creation_flags, void* env, const char* current_dir,
        STARTUPINFO* startup_info, PROCESS_INFORMATION* proc_info) const;

    virtual HANDLE CreateEvent(SECURITY_ATTRIBUTES* event_attributes,
        bool manual_reset, bool initial_state, char* name) const;

    virtual bool CreatePipe(PHANDLE read_pipe, PHANDLE write_pipe,
        LPSECURITY_ATTRIBUTES pipe_attr, DWORD size) const;

    virtual HANDLE CreateNamedPipe(char* name, DWORD open_mode,
        DWORD pipe_mode, DWORD max_instances, DWORD out_buffer_size,
        DWORD in_buffer_size, DWORD default_time_out,
        LPSECURITY_ATTRIBUTES security_attributes) const;

    virtual HANDLE CreateFile(char* file_name, DWORD desired_access,
        DWORD share_mode, LPSECURITY_ATTRIBUTES security_attributes,
        DWORD creation_disposition, DWORD flags_and_attributes,
        HANDLE template_file) const;

    virtual HANDLE CreateMutex(LPSECURITY_ATTRIBUTES attributes,
        bool initial_owner, LPCTSTR name) const;

    virtual HANDLE OpenMutex(DWORD desired_access, bool inherit,
        LPCTSTR name) const;

    virtual bool SetEvent(HANDLE handle) const;

    virtual bool ResetEvent(HANDLE handle) const;

    virtual bool ConnectNamedPipe(HANDLE handle,
        LPOVERLAPPED overlapped) const;

    virtual bool DisconnectNamedPipe(HANDLE handle) const;

    virtual bool GetOverlappedResult(HANDLE handle, LPOVERLAPPED overlapped,
        LPDWORD bytes, BOOL wait) const;

    virtual bool ReadFile(HANDLE handle, LPVOID buffer, DWORD to_read,
        LPDWORD read, LPOVERLAPPED overlapped) const;

    virtual bool WriteFile(HANDLE handle, LPCVOID buffer, DWORD to_write,
        LPDWORD written, LPOVERLAPPED overlapped) const;

    virtual DWORD WaitForSingleObject(HANDLE handle, DWORD timeout) const;

    virtual DWORD WaitForMultipleObjects(DWORD handles_count,
        const HANDLE* handles, bool wait_all, DWORD timeout) const;

    virtual bool DuplicateHandle(HANDLE source_process_handle,
        HANDLE source_handle, HANDLE target_process_handle,
        LPHANDLE target_handle, DWORD desired_access,
        bool inherit_handle,
        DWORD options) const;

    virtual bool SetHandleInformation(HANDLE object, DWORD mask,
        DWORD flags) const;

    virtual HANDLE GetStdHandle(DWORD std_handle) const;

    virtual bool ReleaseMutex(HANDLE mutex) const;

    virtual bool CloseHandle(HANDLE handle) const;

    virtual bool GenerateConsoleCtrlEvent(DWORD ctrl_event,
        DWORD proc_group_id) const;

    virtual bool GetExitCodeProcess(HANDLE handle, DWORD* exit_code) const;

    virtual HANDLE GetCurrentProcess() const;

    virtual bool TerminateProcess(HANDLE process, UINT exit_code) const;

    virtual DWORD GetLastError() const;

    virtual bool SetEnvironmentVariable(LPCTSTR name, LPCTSTR value) const;

    virtual DWORD GetEnvironmentVariable(LPCTSTR name, LPTSTR buffer,
        DWORD size) const;

    virtual DWORD ExpandEnvironmentStrings(LPCTSTR src, LPTSTR dst,
        DWORD size) const;

    virtual LPTCH GetEnvironmentStrings() const;

    virtual bool FreeEnvironmentStrings(LPTCH lpszEnvironmentBlock) const;

    virtual bool SetConsoleCtrlHandler(PHANDLER_ROUTINE handler,
        bool add) const;

    virtual RPC_STATUS UuidCreateSequential(GUID* uuid) const;

    virtual RPC_STATUS UuidCreateNil(GUID* uuid) const;

    virtual RPC_STATUS UuidCompare(GUID* uuid1, GUID* uuid2,
        RPC_STATUS* status) const;

    virtual RPC_STATUS UuidToString(const GUID* uuid,
        unsigned char** string_uuid) const;

    virtual RPC_STATUS UuidFromString(unsigned char* string_uuid,
        GUID* uuid) const;

    virtual RPC_STATUS RpcStringFree(unsigned char** rpc_string) const;

    virtual bool CryptAcquireContext(HCRYPTPROV* csp, LPCTSTR container,
        LPCTSTR provider, DWORD type, DWORD flags) const;

    virtual bool CryptCreateHash(HCRYPTPROV csp, ALG_ID algorithm,
        HCRYPTKEY key, DWORD flags, HCRYPTHASH* hash) const;

    virtual bool CryptHashData(HCRYPTHASH hash, unsigned char* data,
        DWORD length, DWORD flags) const;

    virtual bool CryptGetHashParam(HCRYPTHASH hash, DWORD param,
        unsigned char* data, DWORD* length, DWORD flags) const;

    virtual bool CryptDestroyHash(HCRYPTHASH hash) const;

    virtual bool CryptReleaseContext(HCRYPTPROV csp, DWORD flags) const;

    static const WindowsInterface& GetInstance();

    void operator=(const WindowsInterface&) = delete;
    WindowsInterface& operator=(WindowsInterface&&) = delete;
};
}  // namespace winss

#endif  // LIB_WINSS_WINDOWS_INTERFACE_HPP_
