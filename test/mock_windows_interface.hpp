#ifndef TEST_MOCK_WINDOWS_INTERFACE_HPP_
#define TEST_MOCK_WINDOWS_INTERFACE_HPP_

#include <windows.h>
#include <wincrypt.h>
#include <rpc.h>
#include <gmock/gmock.h>
#include <winss/windows_interface.hpp>

using ::testing::_;
using ::testing::Invoke;

namespace winss {
class MockWindowsInterface : public winss::WindowsInterface {
 private:
    bool CreateProcessConcrete(const char* application_name,
        char* command_line, SECURITY_ATTRIBUTES* proc_attr,
        SECURITY_ATTRIBUTES* thread_atttr, bool inherit_handles,
        DWORD creation_flags, void* env, const char* current_dir,
        STARTUPINFO* startup_info, PROCESS_INFORMATION* proc_info) const {
        return winss::WindowsInterface::CreateProcess(application_name,
            command_line, proc_attr, thread_atttr, inherit_handles,
            creation_flags, env, current_dir, startup_info, proc_info);
    }

    HANDLE CreateEventConcrete(SECURITY_ATTRIBUTES* event_attributes,
        bool manual_reset, bool initial_state, char* name) const {
        return winss::WindowsInterface::CreateEvent(event_attributes,
            manual_reset, initial_state, name);
    }

    bool CreatePipeConcrete(PHANDLE read_pipe, PHANDLE write_pipe,
        LPSECURITY_ATTRIBUTES pipe_attr, DWORD size) const {
        return winss::WindowsInterface::CreatePipe(read_pipe, write_pipe,
            pipe_attr, size);
    }

    HANDLE CreateNamedPipeConcrete(char* name, DWORD open_mode,
        DWORD pipe_mode, DWORD max_instances, DWORD out_buffer_size,
        DWORD in_buffer_size, DWORD default_time_out,
        LPSECURITY_ATTRIBUTES security_attributes) const {
        return winss::WindowsInterface::CreateNamedPipe(name, open_mode,
            pipe_mode, max_instances, out_buffer_size, in_buffer_size,
            default_time_out, security_attributes);
    }

    HANDLE CreateFileConcrete(char* file_name, DWORD desired_access,
        DWORD share_mode, LPSECURITY_ATTRIBUTES security_attributes,
        DWORD creation_disposition, DWORD flags_and_attributes,
        HANDLE template_file) const {
        return winss::WindowsInterface::CreateFile(file_name, desired_access,
            share_mode, security_attributes, creation_disposition,
            flags_and_attributes, template_file);
    }

    HANDLE CreateMutexConcrete(LPSECURITY_ATTRIBUTES attributes,
        bool initial_owner, LPCTSTR name) const {
        return winss::WindowsInterface::CreateMutex(attributes,
            initial_owner, name);
    }

    HANDLE OpenMutexConcrete(DWORD desired_access, bool inherit,
        LPCTSTR name) const {
        return winss::WindowsInterface::OpenMutex(desired_access,
            inherit, name);
    }

    bool SetEventConcrete(HANDLE handle) const {
        return winss::WindowsInterface::SetEvent(handle);
    }

    bool ResetEventConcrete(HANDLE handle) const {
        return winss::WindowsInterface::ResetEvent(handle);
    }

    bool ConnectNamedPipeConcrete(HANDLE handle,
        LPOVERLAPPED overlapped) const {
        return winss::WindowsInterface::ConnectNamedPipe(handle, overlapped);
    }

    bool DisconnectNamedPipeConcrete(HANDLE handle) const {
        return winss::WindowsInterface::DisconnectNamedPipe(handle);
    }

    bool GetOverlappedResultConcrete(HANDLE handle, LPOVERLAPPED overlapped,
        LPDWORD bytes, BOOL wait) const {
        return winss::WindowsInterface::GetOverlappedResult(handle, overlapped,
            bytes, wait);
    }

    bool ReadFileConcrete(HANDLE handle, LPVOID buffer, DWORD to_read,
        LPDWORD read, LPOVERLAPPED overlapped) const {
        return winss::WindowsInterface::ReadFile(handle, buffer, to_read,
            read, overlapped);
    }

    bool WriteFileConcrete(HANDLE handle, LPCVOID buffer, DWORD to_write,
        LPDWORD written, LPOVERLAPPED overlapped) const {
        return winss::WindowsInterface::WriteFile(handle, buffer, to_write,
            written, overlapped);
    }

    DWORD WaitForSingleObjectConcrete(HANDLE handle, DWORD timeout) const {
        return winss::WindowsInterface::WaitForSingleObject(handle, timeout);
    }

    DWORD WaitForMultipleObjectsConcrete(DWORD handles_count,
        const HANDLE* handles, bool wait_all, DWORD timeout) const {
        return winss::WindowsInterface::WaitForMultipleObjects(handles_count,
            handles, wait_all, timeout);
    }

    bool DuplicateHandleConcrete(HANDLE source_process_handle,
        HANDLE source_handle, HANDLE target_process_handle,
        LPHANDLE target_handle, DWORD desired_access,
        bool inherit_handle, DWORD options) const {
        return winss::WindowsInterface::DuplicateHandle(source_process_handle,
            source_handle, target_process_handle, target_handle,
            desired_access, inherit_handle, options);
    }

    bool SetHandleInformationConcrete(HANDLE object, DWORD mask,
        DWORD flags) const {
        return winss::WindowsInterface::SetHandleInformation(object,
            mask, flags);
    }

    HANDLE GetStdHandleConcrete(DWORD std_handle) const {
        return winss::WindowsInterface::GetStdHandle(std_handle);
    }

    bool ReleaseMutexConcrete(HANDLE mutex) const {
        return winss::WindowsInterface::ReleaseMutex(mutex);
    }

    bool CloseHandleConcrete(HANDLE handle) const {
        return winss::WindowsInterface::CloseHandle(handle);
    }

    bool GenerateConsoleCtrlEventConcrete(DWORD ctrl_event,
        DWORD proc_group_id) const {
        return winss::WindowsInterface::GenerateConsoleCtrlEvent(ctrl_event,
            proc_group_id);
    }

    bool GetExitCodeProcessConcrete(HANDLE handle, DWORD* exit_code) const {
        return winss::WindowsInterface::GetExitCodeProcess(handle, exit_code);
    }

    HANDLE GetCurrentProcessConcrete() const {
        return winss::WindowsInterface::GetCurrentProcess();
    }

    bool TerminateProcessConcrete(HANDLE process, UINT exit_code) const {
        return winss::WindowsInterface::TerminateProcess(process, exit_code);
    }

    DWORD GetLastErrorConcrete() const {
        return winss::WindowsInterface::GetLastError();
    }

    bool SetEnvironmentVariableConcrete(LPCTSTR name, LPCTSTR value) const {
        return winss::WindowsInterface::SetEnvironmentVariable(name, value);
    }

    DWORD GetEnvironmentVariableConcrete(LPCTSTR name, LPTSTR buffer,
        DWORD size) const {
        return winss::WindowsInterface::GetEnvironmentVariable(name, buffer,
            size);
    }

    DWORD ExpandEnvironmentStringsConcrete(LPCTSTR src, LPTSTR dst,
        DWORD size) const {
        return winss::WindowsInterface::ExpandEnvironmentStrings(src,
            dst, size);
    }

    LPTCH GetEnvironmentStringsConcrete() const {
        return winss::WindowsInterface::GetEnvironmentStrings();
    }

    bool FreeEnvironmentStringsConcrete(LPTCH lpszEnvironmentBlock) const {
        return winss::WindowsInterface::FreeEnvironmentStrings(
            lpszEnvironmentBlock);
    }

    bool SetConsoleCtrlHandlerConcrete(PHANDLER_ROUTINE handler,
        bool add) const {
        return winss::WindowsInterface::SetConsoleCtrlHandler(handler, add);
    }

    RPC_STATUS UuidCreateSequentialConcrete(GUID* uuid) const {
        return winss::WindowsInterface::UuidCreateSequential(uuid);
    }

    RPC_STATUS UuidCreateNilConcrete(GUID* uuid) const {
        return winss::WindowsInterface::UuidCreateNil(uuid);
    }

    RPC_STATUS UuidCompareConcrete(GUID* uuid1, GUID* uuid2,
        RPC_STATUS* status) const {
        return winss::WindowsInterface::UuidCompare(uuid1, uuid2, status);
    }

    RPC_STATUS UuidToStringConcrete(const GUID* uuid,
        unsigned char** string_uuid) const {
        return winss::WindowsInterface::UuidToString(uuid, string_uuid);
    }

    RPC_STATUS UuidFromStringConcrete(unsigned char* string_uuid,
        GUID* uuid) const {
        return winss::WindowsInterface::UuidFromString(string_uuid, uuid);
    }

    RPC_STATUS RpcStringFreeConcrete(unsigned char** rpc_string) const {
        return winss::WindowsInterface::RpcStringFree(rpc_string);
    }

    bool CryptAcquireContextConcrete(HCRYPTPROV* csp, LPCTSTR container,
        LPCTSTR provider, DWORD type, DWORD flags) const {
        return winss::WindowsInterface::CryptAcquireContext(csp, container,
            provider, type, flags);
    }

    bool CryptCreateHashConcrete(HCRYPTPROV csp, ALG_ID algorithm,
        HCRYPTKEY key, DWORD flags, HCRYPTHASH* hash) const {
        return winss::WindowsInterface::CryptCreateHash(csp, algorithm,
            key, flags, hash);
    }

    bool CryptHashDataConcrete(HCRYPTHASH hash, unsigned char* data,
        DWORD length, DWORD flags) const {
        return winss::WindowsInterface::CryptHashData(hash, data, length,
            flags);
    }

    bool CryptGetHashParamConcrete(HCRYPTHASH hash, DWORD param,
        unsigned char* data, DWORD* length, DWORD flags) const {
        return winss::WindowsInterface::CryptGetHashParam(hash, param, data,
            length, flags);
    }

    bool CryptDestroyHashConcrete(HCRYPTHASH hash) const {
        return winss::WindowsInterface::CryptDestroyHash(hash);
    }

    bool CryptReleaseContextConcrete(HCRYPTPROV csp, DWORD flags) const {
        return winss::WindowsInterface::CryptReleaseContext(csp, flags);
    }

 public:
    using winss::WindowsInterface::instance;

    MockWindowsInterface() {}
    MockWindowsInterface(const MockWindowsInterface&) = delete;
    MockWindowsInterface(MockWindowsInterface&&) = delete;

    MOCK_CONST_METHOD10(CreateProcess, bool(const char* application_name,
        char* command_line, SECURITY_ATTRIBUTES* proc_attr,
        SECURITY_ATTRIBUTES* thread_atttr, bool inherit_handles,
        DWORD creation_flags, void* env, const char* current_dir,
        STARTUPINFO* startup_info, PROCESS_INFORMATION* proc_info));

    MOCK_CONST_METHOD4(CreateEvent, HANDLE(
        SECURITY_ATTRIBUTES* event_attributes, bool manual_reset,
        bool initial_state, char* name));

    MOCK_CONST_METHOD4(CreatePipe, bool(PHANDLE read_pipe, PHANDLE write_pipe,
        LPSECURITY_ATTRIBUTES pipe_attr, DWORD size));

    MOCK_CONST_METHOD8(CreateNamedPipe, HANDLE(char* name, DWORD open_mode,
        DWORD pipe_mode, DWORD max_instances, DWORD out_buffer_size,
        DWORD in_buffer_size, DWORD default_time_out,
        LPSECURITY_ATTRIBUTES security_attributes));

    MOCK_CONST_METHOD7(CreateFile, HANDLE(char* file_name,
        DWORD desired_access, DWORD share_mode,
        LPSECURITY_ATTRIBUTES security_attributes,
        DWORD creation_disposition, DWORD flags_and_attributes,
        HANDLE template_file));

    MOCK_CONST_METHOD3(CreateMutex, HANDLE(LPSECURITY_ATTRIBUTES attributes,
        bool initial_owner, LPCTSTR name));

    MOCK_CONST_METHOD3(OpenMutex, HANDLE(DWORD desired_access, bool inherit,
        LPCTSTR name));

    MOCK_CONST_METHOD1(SetEvent, bool(HANDLE handle));

    MOCK_CONST_METHOD1(ResetEvent, bool(HANDLE handle));

    MOCK_CONST_METHOD2(ConnectNamedPipe, bool(HANDLE handle,
        LPOVERLAPPED overlapped));

    MOCK_CONST_METHOD1(DisconnectNamedPipe, bool(HANDLE handle));

    MOCK_CONST_METHOD4(GetOverlappedResult, bool(HANDLE handle,
        LPOVERLAPPED overlapped, LPDWORD bytes, BOOL wait));

    MOCK_CONST_METHOD5(ReadFile, bool(HANDLE handle, LPVOID buffer,
        DWORD to_read, LPDWORD read, LPOVERLAPPED overlapped));

    MOCK_CONST_METHOD5(WriteFile, bool(HANDLE handle, LPCVOID buffer,
        DWORD to_write, LPDWORD written, LPOVERLAPPED overlapped));

    MOCK_CONST_METHOD2(WaitForSingleObject, DWORD(HANDLE handle,
        DWORD timeout));

    MOCK_CONST_METHOD4(WaitForMultipleObjects, DWORD(
        DWORD handles_count, const HANDLE* handles, bool wait_all,
        DWORD timeout));

    MOCK_CONST_METHOD7(DuplicateHandle, bool(HANDLE source_process_handle,
        HANDLE source_handle, HANDLE target_process_handle,
        LPHANDLE target_handle, DWORD desired_access,
        bool inherit_handle, DWORD options));

    MOCK_CONST_METHOD3(SetHandleInformation, bool(HANDLE object,
        DWORD mask, DWORD flags));

    MOCK_CONST_METHOD1(GetStdHandle, HANDLE(DWORD std_handle));

    MOCK_CONST_METHOD1(ReleaseMutex, bool(HANDLE mutex));

    MOCK_CONST_METHOD1(CloseHandle, bool(HANDLE handle));

    MOCK_CONST_METHOD2(GetExitCodeProcess, bool(HANDLE handle,
        DWORD* exit_code));

    MOCK_CONST_METHOD2(GenerateConsoleCtrlEvent, bool(DWORD ctrl_event,
        DWORD proc_group_id));

    MOCK_CONST_METHOD0(GetCurrentProcess, HANDLE());

    MOCK_CONST_METHOD2(TerminateProcess, bool(HANDLE process,
        UINT exit_code));

    MOCK_CONST_METHOD0(GetLastError, DWORD());

    MOCK_CONST_METHOD2(SetEnvironmentVariable, bool(LPCTSTR name,
        LPCTSTR value));

    MOCK_CONST_METHOD3(GetEnvironmentVariable, DWORD(LPCTSTR name,
        LPTSTR buffer, DWORD size));

    MOCK_CONST_METHOD3(ExpandEnvironmentStrings, DWORD(LPCTSTR src,
        LPTSTR dst, DWORD size));

    MOCK_CONST_METHOD0(GetEnvironmentStrings, LPTCH());

    MOCK_CONST_METHOD1(FreeEnvironmentStrings,
        bool(LPTCH lpszEnvironmentBlock));

    MOCK_CONST_METHOD2(SetConsoleCtrlHandler, bool(PHANDLER_ROUTINE handler,
        bool add));

    MOCK_CONST_METHOD1(UuidCreateSequential, RPC_STATUS(GUID* uuid));

    MOCK_CONST_METHOD1(UuidCreateNil, RPC_STATUS(GUID* uuid));

    MOCK_CONST_METHOD3(UuidCompare, RPC_STATUS(GUID* uuid1, GUID* uuid2,
        RPC_STATUS* status));

    MOCK_CONST_METHOD2(UuidToString, RPC_STATUS(const GUID* uuid,
        unsigned char** string_uuid));

    MOCK_CONST_METHOD2(UuidFromString, RPC_STATUS(unsigned char* string_uuid,
        GUID* uuid));

    MOCK_CONST_METHOD1(RpcStringFree, RPC_STATUS(unsigned char** rpc_string));

    MOCK_CONST_METHOD5(CryptAcquireContext, bool(HCRYPTPROV* csp,
        LPCTSTR container, LPCTSTR provider, DWORD type, DWORD flags));

    MOCK_CONST_METHOD5(CryptCreateHash, bool(HCRYPTPROV csp, ALG_ID algorithm,
        HCRYPTKEY key, DWORD flags, HCRYPTHASH* hash));

    MOCK_CONST_METHOD4(CryptHashData, bool(HCRYPTHASH hash,
        unsigned char* data, DWORD length, DWORD flags));

    MOCK_CONST_METHOD5(CryptGetHashParam, bool(HCRYPTHASH hash, DWORD param,
        unsigned char* data, DWORD* length, DWORD flags));

    MOCK_CONST_METHOD1(CryptDestroyHash, bool(HCRYPTHASH hash));

    MOCK_CONST_METHOD2(CryptReleaseContext, bool(HCRYPTPROV csp, DWORD flags));

    void SetupDefaults() {
        ON_CALL(*this, CreateProcess(_, _, _, _, _, _, _, _, _, _))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::CreateProcessConcrete));

        ON_CALL(*this, CreateEvent(_, _, _, _))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::CreateEventConcrete));

        ON_CALL(*this, CreatePipe(_, _, _, _))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::CreatePipeConcrete));

        ON_CALL(*this, CreateNamedPipe(_, _, _, _, _, _, _, _))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::CreateNamedPipeConcrete));

        ON_CALL(*this, CreateFile(_, _, _, _, _, _, _))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::CreateFileConcrete));

        ON_CALL(*this, CreateMutex(_, _, _))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::CreateMutexConcrete));

        ON_CALL(*this, OpenMutex(_, _, _))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::OpenMutexConcrete));

        ON_CALL(*this, SetEvent(_))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::SetEventConcrete));

        ON_CALL(*this, ResetEvent(_))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::ResetEventConcrete));

        ON_CALL(*this, ConnectNamedPipe(_, _))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::ConnectNamedPipeConcrete));

        ON_CALL(*this, DisconnectNamedPipe(_))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::DisconnectNamedPipeConcrete));

        ON_CALL(*this, GetOverlappedResult(_, _, _, _))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::GetOverlappedResultConcrete));

        ON_CALL(*this, ReadFile(_, _, _, _, _))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::ReadFileConcrete));

        ON_CALL(*this, WriteFile(_, _, _, _, _))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::WriteFileConcrete));

        ON_CALL(*this, WaitForSingleObject(_, _))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::WaitForSingleObjectConcrete));

        ON_CALL(*this, WaitForMultipleObjects(_, _, _, _))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::WaitForMultipleObjectsConcrete));

        ON_CALL(*this, DuplicateHandle(_, _, _, _, _, _, _))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::DuplicateHandleConcrete));

        ON_CALL(*this, SetHandleInformation(_, _, _))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::SetHandleInformationConcrete));

        ON_CALL(*this, GetStdHandle(_))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::GetStdHandleConcrete));

        ON_CALL(*this, ReleaseMutex(_))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::ReleaseMutexConcrete));

        ON_CALL(*this, CloseHandle(_))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::CloseHandleConcrete));

        ON_CALL(*this, GetExitCodeProcess(_, _))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::GetExitCodeProcessConcrete));

        ON_CALL(*this, GenerateConsoleCtrlEvent(_, _))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::GenerateConsoleCtrlEventConcrete));

        ON_CALL(*this, GetCurrentProcess())
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::GetCurrentProcessConcrete));

        ON_CALL(*this, TerminateProcess(_, _))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::TerminateProcessConcrete));

        ON_CALL(*this, GetLastError())
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::GetLastErrorConcrete));

        ON_CALL(*this, SetEnvironmentVariable(_, _))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::SetEnvironmentVariableConcrete));

        ON_CALL(*this, GetEnvironmentVariable(_, _, _))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::GetEnvironmentVariableConcrete));

        ON_CALL(*this, ExpandEnvironmentStrings(_, _, _))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::ExpandEnvironmentStringsConcrete));

        ON_CALL(*this, GetEnvironmentStrings())
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::GetEnvironmentStringsConcrete));

        ON_CALL(*this, FreeEnvironmentStrings(_))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::FreeEnvironmentStringsConcrete));

        ON_CALL(*this, SetConsoleCtrlHandler(_, _))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::SetConsoleCtrlHandlerConcrete));

        ON_CALL(*this, UuidCreateSequential(_))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::UuidCreateSequentialConcrete));

        ON_CALL(*this, UuidCreateNil(_))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::UuidCreateNilConcrete));

        ON_CALL(*this, UuidCompare(_, _, _))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::UuidCompareConcrete));

        ON_CALL(*this, UuidToString(_, _))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::UuidToStringConcrete));

        ON_CALL(*this, UuidFromString(_, _))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::UuidFromStringConcrete));

        ON_CALL(*this, RpcStringFree(_))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::RpcStringFreeConcrete));

        ON_CALL(*this, CryptAcquireContext(_, _, _, _, _))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::CryptAcquireContextConcrete));

        ON_CALL(*this, CryptCreateHash(_, _, _, _, _))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::CryptCreateHashConcrete));

        ON_CALL(*this, CryptHashData(_, _, _, _))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::CryptHashDataConcrete));

        ON_CALL(*this, CryptGetHashParam(_, _, _, _, _))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::CryptGetHashParamConcrete));

        ON_CALL(*this, CryptDestroyHash(_))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::CryptDestroyHashConcrete));

        ON_CALL(*this, CryptReleaseContext(_, _))
            .WillByDefault(Invoke(this,
                &MockWindowsInterface::CryptReleaseContextConcrete));
    }

    void operator=(const MockWindowsInterface&) = delete;
    MockWindowsInterface& operator=(MockWindowsInterface&&) = delete;
};
}  // namespace winss

#endif  // TEST_MOCK_WINDOWS_INTERFACE_HPP_
