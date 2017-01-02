#include <thread>
#include <vector>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "winss/winss.hpp"
#include "winss/process.hpp"
#include "winss/handle_wrapper.hpp"
#include "mock_interface.hpp"
#include "mock_windows_interface.hpp"
#include "mock_environment.hpp"

using ::testing::_;
using ::testing::Assign;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::Unused;
using ::testing::NiceMock;

namespace winss {
class ProcessTest : public testing::Test {
 public:
    static const DWORD kProcId = 10;
    static const DWORD kExitCode = 20;
    static constexpr HANDLE kProcHandle = reinterpret_cast<HANDLE>(30000);

    bool SetProcInfo(Unused, Unused, Unused, Unused, Unused,
        Unused, Unused, Unused, Unused, PROCESS_INFORMATION* proc_info) {
        proc_info->dwProcessId = kProcId;
        proc_info->hProcess = kProcHandle;
        return true;
    }

    bool SetGetExitCodeProcessActive(Unused, DWORD* exit_code) {
        *exit_code = STILL_ACTIVE;
        return true;
    }

    bool SetGetExitCodeProcessNotActive(Unused, DWORD* exit_code) {
        *exit_code = kExitCode;
        return true;
    }

    bool DuplicateHandle(Unused, HANDLE source_handle, Unused,
        LPHANDLE target_handle, Unused, Unused, Unused) {
        *target_handle = source_handle;
        return true;
    }
};

TEST_F(ProcessTest, Create) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateProcess(_, _, _, _, _, _, _, _, _, _))
        .WillOnce(Invoke(this, &ProcessTest::SetProcInfo));

    EXPECT_CALL(*windows, GetExitCodeProcess(_, _))
        .WillOnce(Invoke(this, &ProcessTest::SetGetExitCodeProcessActive));

    EXPECT_CALL(*windows, CloseHandle(_)).Times(2);

    winss::Process proc;

    EXPECT_FALSE(proc.IsCreated());
    EXPECT_FALSE(proc.IsActive());

    EXPECT_TRUE(proc.Create(winss::ProcessParams{ "test --command", false }));
    EXPECT_FALSE(proc.Create(winss::ProcessParams{ "test --command", false }));

    EXPECT_TRUE(proc.IsCreated());
    EXPECT_TRUE(proc.IsActive());
    EXPECT_EQ(winss::ProcessTest::kProcId, proc.GetProcessId());

    winss::HandleWrapper handle(winss::ProcessTest::kProcHandle, false);
    EXPECT_EQ(proc.GetHandle(), kProcHandle);
}

TEST_F(ProcessTest, CreateFailed) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateProcess(_, _, _, _, _, _, _, _, _, _))
        .WillOnce(Return(false));

    EXPECT_CALL(*windows, CloseHandle(_)).Times(0);

    winss::Process proc;

    EXPECT_FALSE(proc.IsCreated());
    EXPECT_FALSE(proc.IsActive());

    EXPECT_FALSE(proc.Create(winss::ProcessParams{ "test --command", false }));

    EXPECT_FALSE(proc.IsCreated());
    EXPECT_FALSE(proc.IsActive());
}

TEST_F(ProcessTest, BreakTerminateAndClose) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateProcess(_, _, _, _, _, _, _, _, _, _))
        .WillOnce(Invoke(this, &ProcessTest::SetProcInfo));

    EXPECT_CALL(*windows, GetExitCodeProcess(_, _))
        .WillOnce(Invoke(this, &ProcessTest::SetGetExitCodeProcessActive))
        .WillOnce(Invoke(this, &ProcessTest::SetGetExitCodeProcessActive))
        .WillOnce(Invoke(this, &ProcessTest::SetGetExitCodeProcessNotActive));

    EXPECT_CALL(*windows, GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, 10))
        .Times(1);

    EXPECT_CALL(*windows, TerminateProcess(_, _))
        .Times(1);

    EXPECT_CALL(*windows, CloseHandle(_)).Times(2);

    winss::Process proc;

    EXPECT_TRUE(proc.Create(winss::ProcessParams{ "test --command", true }));

    proc.SendBreak();

    EXPECT_TRUE(proc.IsCreated());
    EXPECT_TRUE(proc.IsActive());

    proc.Terminate();

    EXPECT_TRUE(proc.IsCreated());
    EXPECT_FALSE(proc.IsActive());

    proc.Close();

    EXPECT_FALSE(proc.IsCreated());
}

TEST_F(ProcessTest, Move) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateProcess(_, _, _, _, _, _, _, _, _, _))
        .WillRepeatedly(Invoke(this, &ProcessTest::SetProcInfo));

    EXPECT_CALL(*windows, GetExitCodeProcess(_, _))
        .WillRepeatedly(
            Invoke(this, &ProcessTest::SetGetExitCodeProcessActive));

    winss::Process proc1;
    EXPECT_TRUE(proc1.Create(winss::ProcessParams{ "test --command", true }));

    winss::Process proc2(std::move(proc1));

    EXPECT_TRUE(proc2.IsCreated());
    EXPECT_FALSE(proc1.IsCreated());

    winss::Process proc3;

    EXPECT_FALSE(proc3.IsCreated());

    proc3 = std::move(proc2);

    EXPECT_TRUE(proc3.IsCreated());
    EXPECT_FALSE(proc2.IsCreated());
}

TEST_F(ProcessTest, RedirectStdInOut) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateProcess(_, _, _, _, _, _, _, _, _, _))
        .WillOnce(Invoke(this, &ProcessTest::SetProcInfo));

    EXPECT_CALL(*windows, DuplicateHandle(_, _, _, _, _, true, _))
        .WillOnce(Invoke(this, &ProcessTest::DuplicateHandle))
        .WillOnce(Invoke(this, &ProcessTest::DuplicateHandle))
        .WillOnce(Invoke(this, &ProcessTest::DuplicateHandle));

    EXPECT_CALL(*windows, CloseHandle(_)).Times(5);

    winss::Process proc;

    EXPECT_TRUE(proc.Create(winss::ProcessParams{
        "test --command",
        true,
        "",
        winss::HandleWrapper(reinterpret_cast<HANDLE>(50000), false),
        winss::HandleWrapper(reinterpret_cast<HANDLE>(51000), false),
        winss::HandleWrapper(reinterpret_cast<HANDLE>(52000), false)
    }));

    proc.Close();
}

TEST_F(ProcessTest, ChangeEnvironment) {
    MockInterface<winss::MockWindowsInterface> windows;
    NiceMock<winss::MockEnviornment> env;

    EXPECT_CALL(*windows, CreateProcess(_, _, _, _, _, _, _, _, _, _))
        .WillOnce(Invoke(this, &ProcessTest::SetProcInfo));

    EXPECT_CALL(env, ReadEnv())
        .WillOnce(Return(std::vector<char>({ '1', '2' })));

    winss::Process proc;
    winss::ProcessParams params{
        "test --command", true,
    };
    params.env = &env;

    EXPECT_TRUE(proc.Create(params));

    proc.Close();
}
}  // namespace winss
