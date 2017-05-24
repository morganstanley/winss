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

#include <filesystem>
#include <thread>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "winss/winss.hpp"
#include "winss/not_owning_ptr.hpp"
#include "winss/event_wrapper.hpp"
#include "winss/wait_multiplexer.hpp"
#include "winss/handle_wrapper.hpp"
#include "winss/supervise/supervise.hpp"
#include "../mock_interface.hpp"
#include "../mock_windows_interface.hpp"
#include "../mock_filesystem_interface.hpp"
#include "../mock_wait_multiplexer.hpp"
#include "../mock_path_mutex.hpp"
#include "../mock_process.hpp"
#include "mock_supervise.hpp"

namespace fs = std::experimental::filesystem;

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::An;

namespace winss {
class SuperviseTest : public testing::Test {
};
class MockedSupervise :
    public winss::SuperviseTmpl<NiceMock<winss::MockPathMutex>,
    NiceMock<winss::MockProcess>> {
 public:
     MockedSupervise(winss::NotOwningPtr<winss::WaitMultiplexer> multiplexer,
         const fs::path& service_dir) :
         winss::SuperviseTmpl<NiceMock<winss::MockPathMutex>,
         NiceMock<winss::MockProcess>>
         ::SuperviseTmpl(multiplexer, service_dir) {}

     winss::MockPathMutex* GetMutex() {
         return &mutex;
     }

     winss::MockProcess* GetProcess() {
         return &process;
     }
};

TEST_F(SuperviseTest, InitNotExistsDir) {
    MockInterface<winss::MockWindowsInterface> windows;
    MockInterface<winss::MockFilesystemInterface> file;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;

    EXPECT_CALL(*file, ChangeDirectory(_)).WillOnce(Return(false));
    EXPECT_CALL(multiplexer, Stop(_)).Times(1);

    MockedSupervise supervise(winss::NotOwned(&multiplexer), "dir");
    multiplexer.mock_init_callbacks.at(0)(multiplexer);
}

TEST_F(SuperviseTest, InitAlreadyRunning) {
    MockInterface<winss::MockWindowsInterface> windows;
    MockInterface<winss::MockFilesystemInterface> file;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;

    EXPECT_CALL(*file, ChangeDirectory(_)).WillOnce(Return(true));
    EXPECT_CALL(multiplexer, Stop(_)).Times(1);

    MockedSupervise supervise(winss::NotOwned(&multiplexer), "dir");

    EXPECT_CALL(*supervise.GetMutex(), Lock()).WillOnce(Return(false));

    multiplexer.mock_init_callbacks.at(0)(multiplexer);
}

TEST_F(SuperviseTest, InitDown) {
    MockInterface<winss::MockWindowsInterface> windows;
    MockInterface<winss::MockFilesystemInterface> file;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;

    EXPECT_CALL(*file, ChangeDirectory(_)).WillOnce(Return(true));
    EXPECT_CALL(*file, FileExists(_)).WillOnce(Return(true));

    MockedSupervise supervise(winss::NotOwned(&multiplexer), "dir");

    EXPECT_CALL(*supervise.GetMutex(), Lock()).WillOnce(Return(true));

    multiplexer.mock_init_callbacks.at(0)(multiplexer);

    EXPECT_FALSE(supervise.GetState().is_up);
    EXPECT_FALSE(supervise.GetState().initially_up);
}

TEST_F(SuperviseTest, InitUp) {
    MockInterface<winss::MockWindowsInterface> windows;
    MockInterface<winss::MockFilesystemInterface> file;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;

    EXPECT_CALL(*file, ChangeDirectory(_)).WillOnce(Return(true));
    EXPECT_CALL(*file, FileExists(_)).WillOnce(Return(false));
    EXPECT_CALL(*file, Read(_)).WillOnce(Return("cmd"));
    EXPECT_CALL(*windows, SetEnvironmentVariable(_, nullptr))
        .WillOnce(Return(true));

    MockedSupervise supervise(winss::NotOwned(&multiplexer), "dir");

    EXPECT_CALL(*supervise.GetMutex(), Lock()).WillOnce(Return(true));
    EXPECT_CALL(*supervise.GetProcess(), Create(_)).WillOnce(Return(true));
    EXPECT_CALL(*supervise.GetProcess(), GetHandle()).WillRepeatedly(
        Return(winss::HandleWrapper(reinterpret_cast<HANDLE>(10000), false)));

    multiplexer.mock_init_callbacks.at(0)(multiplexer);

    EXPECT_TRUE(supervise.GetState().is_up);
    EXPECT_TRUE(supervise.GetState().initially_up);
}

TEST_F(SuperviseTest, RunInvalidCommand) {
    MockInterface<winss::MockWindowsInterface> windows;
    MockInterface<winss::MockFilesystemInterface> file;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;

    EXPECT_CALL(*file, ChangeDirectory(_)).WillOnce(Return(true));
    EXPECT_CALL(*file, FileExists(_)).WillOnce(Return(false));
    EXPECT_CALL(*file, Read(_)).WillOnce(Return("this is invalid"));
    EXPECT_CALL(*windows, SetEnvironmentVariable(_, nullptr))
        .WillOnce(Return(true));

    MockedSupervise supervise(winss::NotOwned(&multiplexer), "dir");

    EXPECT_CALL(*supervise.GetMutex(), Lock()).WillOnce(Return(true));
    EXPECT_CALL(*supervise.GetProcess(), Create(_)).WillOnce(Return(false));
    EXPECT_CALL(*supervise.GetProcess(), GetHandle()).WillRepeatedly(
        Return(winss::HandleWrapper(reinterpret_cast<HANDLE>(10000), false)));

    EXPECT_CALL(multiplexer, AddTimeoutCallback(
        winss::Supervise::kRunFailedWait, _, _)).Times(1);

    multiplexer.mock_init_callbacks.at(0)(multiplexer);

    EXPECT_FALSE(supervise.GetState().is_up);
    EXPECT_TRUE(supervise.GetState().initially_up);
}

TEST_F(SuperviseTest, Restart) {
    MockInterface<winss::MockWindowsInterface> windows;
    MockInterface<winss::MockFilesystemInterface> file;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;
    NiceMock<winss::MockSuperviseListener> listener;

    EXPECT_CALL(*file, ChangeDirectory(_)).WillOnce(Return(true));
    EXPECT_CALL(*file, FileExists(_)).WillOnce(Return(false));
    EXPECT_CALL(*file, Read(_))
        .WillOnce(Return("run"))
        .WillOnce(Return("finish"))
        .WillOnce(Return("run"));

    EXPECT_CALL(listener, Notify(_, _)).Times(5).WillRepeatedly(Return(true));

    MockedSupervise supervise(winss::NotOwned(&multiplexer), "dir");

    supervise.AddListener(winss::NotOwned(&listener));

    EXPECT_CALL(*supervise.GetMutex(), Lock()).WillOnce(Return(true));
    EXPECT_CALL(*supervise.GetProcess(), Create(_))
        .WillOnce(Return(true))
        .WillOnce(Return(false))
        .WillOnce(Return(true));

    winss::HandleWrapper handle =
        winss::HandleWrapper(reinterpret_cast<HANDLE>(10000), false);
    EXPECT_CALL(*supervise.GetProcess(), GetHandle())
        .WillRepeatedly(Return(handle));

    multiplexer.mock_init_callbacks.at(0)(multiplexer);

    EXPECT_TRUE(supervise.GetState().is_up);
    EXPECT_TRUE(supervise.GetState().is_run_process);

    EXPECT_CALL(*supervise.GetProcess(), GetExitCode()).WillOnce(Return(0));
    EXPECT_CALL(multiplexer, AddTimeoutCallback(An<DWORD>(), _, _)).Times(1);

    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle);

    EXPECT_FALSE(supervise.GetState().is_up);
    EXPECT_FALSE(supervise.GetState().is_run_process);

    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle);

    EXPECT_TRUE(supervise.GetState().is_up);
    EXPECT_TRUE(supervise.GetState().is_run_process);
}

TEST_F(SuperviseTest, RestartFinish) {
    MockInterface<winss::MockWindowsInterface> windows;
    MockInterface<winss::MockFilesystemInterface> file;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;

    EXPECT_CALL(*file, ChangeDirectory(_)).WillOnce(Return(true));
    EXPECT_CALL(*file, FileExists(_)).WillOnce(Return(false));
    EXPECT_CALL(*file, Read(_))
        .WillOnce(Return("run"))
        .WillOnce(Return("finish"))
        .WillOnce(Return("1000"))
        .WillOnce(Return("run"));

    MockedSupervise supervise(winss::NotOwned(&multiplexer), "dir");

    EXPECT_CALL(*supervise.GetMutex(), Lock()).WillOnce(Return(true));
    EXPECT_CALL(*supervise.GetProcess(), Create(_))
        .Times(3).WillRepeatedly(Return(true));

    winss::HandleWrapper handle =
        winss::HandleWrapper(reinterpret_cast<HANDLE>(10000), false);
    EXPECT_CALL(*supervise.GetProcess(), GetHandle())
        .WillRepeatedly(Return(handle));

    multiplexer.mock_init_callbacks.at(0)(multiplexer);

    EXPECT_TRUE(supervise.GetState().is_up);
    EXPECT_TRUE(supervise.GetState().is_run_process);

    EXPECT_CALL(*supervise.GetProcess(), GetExitCode())
        .Times(2).WillRepeatedly(Return(0));
    EXPECT_CALL(multiplexer, AddTimeoutCallback(An<DWORD>(), _, _)).Times(2);

    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle);

    EXPECT_TRUE(supervise.GetState().is_up);
    EXPECT_FALSE(supervise.GetState().is_run_process);

    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle);

    EXPECT_FALSE(supervise.GetState().is_up);
    EXPECT_FALSE(supervise.GetState().is_run_process);

    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle);

    EXPECT_TRUE(supervise.GetState().is_up);
    EXPECT_TRUE(supervise.GetState().is_run_process);
}

TEST_F(SuperviseTest, FinishDown) {
    MockInterface<winss::MockWindowsInterface> windows;
    MockInterface<winss::MockFilesystemInterface> file;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;

    EXPECT_CALL(*file, ChangeDirectory(_)).WillOnce(Return(true));
    EXPECT_CALL(*file, FileExists(_)).WillOnce(Return(false));
    EXPECT_CALL(*file, Read(_))
        .WillOnce(Return("run"))
        .WillOnce(Return("finish"))
        .WillOnce(Return("1000"));

    MockedSupervise supervise(winss::NotOwned(&multiplexer), "dir");

    EXPECT_CALL(*supervise.GetMutex(), Lock()).WillOnce(Return(true));
    EXPECT_CALL(*supervise.GetProcess(), Create(_))
        .Times(2).WillRepeatedly(Return(true));

    winss::HandleWrapper handle =
        winss::HandleWrapper(reinterpret_cast<HANDLE>(10000), false);
    EXPECT_CALL(*supervise.GetProcess(), GetHandle())
        .WillRepeatedly(Return(handle));

    multiplexer.mock_init_callbacks.at(0)(multiplexer);

    EXPECT_TRUE(supervise.GetState().is_up);
    EXPECT_TRUE(supervise.GetState().is_run_process);

    EXPECT_CALL(*supervise.GetProcess(), GetExitCode())
        .WillOnce(Return(0))
        .WillOnce(Return(winss::Supervise::kDownExitCode));

    EXPECT_CALL(multiplexer, AddTimeoutCallback(An<DWORD>(), _, _)).Times(1);

    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle);

    EXPECT_TRUE(supervise.GetState().is_up);
    EXPECT_FALSE(supervise.GetState().is_run_process);
    EXPECT_NE(0, supervise.GetState().remaining_count);

    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle);

    EXPECT_FALSE(supervise.GetState().is_up);
    EXPECT_FALSE(supervise.GetState().is_run_process);
    EXPECT_EQ(0, supervise.GetState().remaining_count);
}

TEST_F(SuperviseTest, FinishTimeout) {
    MockInterface<winss::MockWindowsInterface> windows;
    MockInterface<winss::MockFilesystemInterface> file;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;

    EXPECT_CALL(*file, ChangeDirectory(_)).WillOnce(Return(true));
    EXPECT_CALL(*file, FileExists(_)).WillOnce(Return(false));
    EXPECT_CALL(*file, Read(_))
        .WillOnce(Return("run"))
        .WillOnce(Return("finish"))
        .WillOnce(Return(""));

    MockedSupervise supervise(winss::NotOwned(&multiplexer), "dir");

    EXPECT_CALL(*supervise.GetMutex(), Lock()).WillOnce(Return(true));
    EXPECT_CALL(*supervise.GetProcess(), Create(_))
        .Times(2).WillRepeatedly(Return(true));

    winss::HandleWrapper handle =
        winss::HandleWrapper(reinterpret_cast<HANDLE>(10000), false);
    EXPECT_CALL(*supervise.GetProcess(), GetHandle())
        .WillRepeatedly(Return(handle));

    multiplexer.mock_init_callbacks.at(0)(multiplexer);

    EXPECT_TRUE(supervise.GetState().is_up);
    EXPECT_TRUE(supervise.GetState().is_run_process);

    EXPECT_CALL(*supervise.GetProcess(), GetExitCode())
        .Times(2).WillRepeatedly(Return(0));
    EXPECT_CALL(multiplexer, AddTimeoutCallback(An<DWORD>(), _, _)).Times(2);

    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle);

    EXPECT_TRUE(supervise.GetState().is_up);
    EXPECT_FALSE(supervise.GetState().is_run_process);

    EXPECT_CALL(*supervise.GetProcess(), Terminate()).Times(1);

    multiplexer.mock_timeout_callbacks.at(0)(multiplexer);

    EXPECT_TRUE(supervise.GetState().is_up);
    EXPECT_FALSE(supervise.GetState().is_run_process);

    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle);

    EXPECT_FALSE(supervise.GetState().is_up);
    EXPECT_FALSE(supervise.GetState().is_run_process);
}

TEST_F(SuperviseTest, Stop) {
    MockInterface<winss::MockWindowsInterface> windows;
    MockInterface<winss::MockFilesystemInterface> file;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;

    EXPECT_CALL(*file, ChangeDirectory(_)).WillOnce(Return(true));
    EXPECT_CALL(*file, FileExists(_)).WillOnce(Return(false));
    EXPECT_CALL(*file, Read(_))
        .WillOnce(Return("run"))
        .WillOnce(Return("finish"))
        .WillOnce(Return("1000"));

    MockedSupervise supervise(winss::NotOwned(&multiplexer), "dir");

    EXPECT_CALL(*supervise.GetMutex(), Lock()).WillOnce(Return(true));
    EXPECT_CALL(*supervise.GetMutex(), HasLock())
        .WillOnce(Return(false))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*supervise.GetProcess(), Create(_))
        .Times(2).WillRepeatedly(Return(true));

    winss::HandleWrapper handle =
        winss::HandleWrapper(reinterpret_cast<HANDLE>(10000), false);
    EXPECT_CALL(*supervise.GetProcess(), GetHandle())
        .WillRepeatedly(Return(handle));

    multiplexer.mock_init_callbacks.at(0)(multiplexer);

    EXPECT_TRUE(supervise.GetState().is_up);
    EXPECT_TRUE(supervise.GetState().is_run_process);

    EXPECT_CALL(*supervise.GetProcess(), SendBreak()).Times(1);

    multiplexer.mock_stop_callbacks.at(0)(multiplexer);

    EXPECT_CALL(*supervise.GetProcess(), GetExitCode()).WillOnce(Return(0));
    EXPECT_CALL(multiplexer, IsStopping()).WillOnce(Return(false));

    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle);

    EXPECT_TRUE(supervise.GetState().is_up);
    EXPECT_FALSE(supervise.GetState().is_run_process);

    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle);

    EXPECT_FALSE(supervise.GetState().is_up);
    EXPECT_FALSE(supervise.GetState().is_run_process);
    EXPECT_EQ(winss::Supervise::kSignaledExitCode,
        supervise.GetState().exit_code);
}

TEST_F(SuperviseTest, EmptyServiceDir) {
    MockInterface<winss::MockWindowsInterface> windows;
    MockInterface<winss::MockFilesystemInterface> file;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;

    EXPECT_CALL(*file, ChangeDirectory(_)).WillOnce(Return(true));
    EXPECT_CALL(*file, FileExists(_)).WillOnce(Return(false));
    EXPECT_CALL(*file, Read(_))
        .WillOnce(Return(""))
        .WillOnce(Return(""));

    MockedSupervise supervise(winss::NotOwned(&multiplexer), "dir");

    EXPECT_CALL(*supervise.GetMutex(), Lock()).WillOnce(Return(true));
    EXPECT_CALL(*supervise.GetProcess(), Create(_)).Times(0);

    winss::HandleWrapper handle =
        winss::HandleWrapper(reinterpret_cast<HANDLE>(10000), false);
    EXPECT_CALL(*supervise.GetProcess(), GetHandle())
        .WillRepeatedly(Return(handle));

    multiplexer.mock_init_callbacks.at(0)(multiplexer);

    EXPECT_FALSE(supervise.GetState().is_up);
    EXPECT_TRUE(supervise.GetState().is_run_process);

    multiplexer.mock_timeout_callbacks.at(0)(multiplexer);

    EXPECT_FALSE(supervise.GetState().is_up);
    EXPECT_TRUE(supervise.GetState().is_run_process);
}

TEST_F(SuperviseTest, UpOnceDownKillExit) {
    MockInterface<winss::MockWindowsInterface> windows;
    MockInterface<winss::MockFilesystemInterface> file;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;

    EXPECT_CALL(*file, ChangeDirectory(_)).WillOnce(Return(true));
    EXPECT_CALL(*file, FileExists(_)).WillOnce(Return(true));
    EXPECT_CALL(*file, Read(_))
        .WillOnce(Return("run"))
        .WillOnce(Return(""));

    MockedSupervise supervise(winss::NotOwned(&multiplexer), "dir");

    EXPECT_CALL(*supervise.GetMutex(), Lock()).WillOnce(Return(true));
    EXPECT_CALL(*supervise.GetMutex(), HasLock())
        .WillOnce(Return(false))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*supervise.GetProcess(), Create(_)).WillOnce(Return(true));

    winss::HandleWrapper handle =
        winss::HandleWrapper(reinterpret_cast<HANDLE>(10000), false);
    EXPECT_CALL(*supervise.GetProcess(), GetHandle())
        .WillRepeatedly(Return(handle));

    multiplexer.mock_init_callbacks.at(0)(multiplexer);

    EXPECT_FALSE(supervise.GetState().is_up);
    EXPECT_TRUE(supervise.GetState().is_run_process);

    supervise.Up();
    supervise.Once();

    EXPECT_TRUE(supervise.GetState().is_up);
    EXPECT_TRUE(supervise.GetState().is_run_process);
    EXPECT_EQ(0, supervise.GetState().remaining_count);

    supervise.OnceAtMost();

    EXPECT_TRUE(supervise.GetState().is_up);
    EXPECT_TRUE(supervise.GetState().is_run_process);
    EXPECT_EQ(0, supervise.GetState().remaining_count);

    EXPECT_CALL(*supervise.GetProcess(), SendBreak())
        .Times(1);

    supervise.Down();

    EXPECT_CALL(*supervise.GetProcess(), Terminate())
        .Times(1);

    supervise.Kill();

    EXPECT_TRUE(supervise.GetState().is_up);
    EXPECT_TRUE(supervise.GetState().is_run_process);
    EXPECT_EQ(0, supervise.GetState().remaining_count);

    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle);

    EXPECT_FALSE(supervise.GetState().is_up);
    EXPECT_FALSE(supervise.GetState().is_run_process);

    EXPECT_CALL(multiplexer, IsStopping()).WillOnce(Return(false));

    supervise.Exit();

    EXPECT_FALSE(supervise.GetState().is_up);
    EXPECT_FALSE(supervise.GetState().is_run_process);
}

TEST_F(SuperviseTest, CommandsNotStarted) {
    MockInterface<winss::MockWindowsInterface> windows;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;

    MockedSupervise supervise(winss::NotOwned(&multiplexer), "dir");

    supervise.Up();
    supervise.Once();
    supervise.OnceAtMost();
    supervise.Down();
    supervise.Exit();

    EXPECT_FALSE(supervise.GetState().is_up);
    EXPECT_TRUE(supervise.GetState().is_run_process);
}

TEST_F(SuperviseTest, OnceNotUp) {
    MockInterface<winss::MockWindowsInterface> windows;
    MockInterface<winss::MockFilesystemInterface> file;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;

    EXPECT_CALL(*file, ChangeDirectory(_)).WillOnce(Return(true));
    EXPECT_CALL(*file, FileExists(_)).WillOnce(Return(true));
    EXPECT_CALL(*file, Read(_)).WillOnce(Return("run"));

    MockedSupervise supervise(winss::NotOwned(&multiplexer), "dir");

    EXPECT_CALL(*supervise.GetMutex(), Lock()).WillOnce(Return(true));
    EXPECT_CALL(*supervise.GetMutex(), HasLock())
        .WillOnce(Return(false))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*supervise.GetProcess(), Create(_)).WillOnce(Return(true));

    winss::HandleWrapper handle =
        winss::HandleWrapper(reinterpret_cast<HANDLE>(10000), false);
    EXPECT_CALL(*supervise.GetProcess(), GetHandle())
        .WillRepeatedly(Return(handle));

    multiplexer.mock_init_callbacks.at(0)(multiplexer);

    EXPECT_FALSE(supervise.GetState().is_up);
    EXPECT_TRUE(supervise.GetState().is_run_process);

    supervise.Once();

    EXPECT_TRUE(supervise.GetState().is_up);
    EXPECT_TRUE(supervise.GetState().is_run_process);
    EXPECT_EQ(0, supervise.GetState().remaining_count);
}

TEST_F(SuperviseTest, Notifications) {
    MockInterface<winss::MockWindowsInterface> windows;
    MockInterface<winss::MockFilesystemInterface> file;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;
    NiceMock<winss::MockSuperviseListener> listener;

    EXPECT_CALL(*file, ChangeDirectory(_)).WillOnce(Return(true));
    EXPECT_CALL(*file, FileExists(_))
        .WillOnce(Return(false));
    EXPECT_CALL(*file, Read(_))
        .WillOnce(Return("run"))
        .WillOnce(Return("finish"))
        .WillOnce(Return(""))
        .WillOnce(Return("run"))
        .WillOnce(Return("finish"))
        .WillOnce(Return(""));

    EXPECT_CALL(listener, Notify(winss::SuperviseNotification::START, _))
        .WillOnce(Return(true));

    EXPECT_CALL(listener, Notify(winss::SuperviseNotification::RUN, _))
        .WillOnce(Return(true))
        .WillOnce(Return(true));

    EXPECT_CALL(listener, Notify(winss::SuperviseNotification::END, _))
        .WillOnce(Return(true))
        .WillOnce(Return(true));

    EXPECT_CALL(listener, Notify(winss::SuperviseNotification::FINISHED, _))
        .WillOnce(Return(true))
        .WillOnce(Return(true));

    EXPECT_CALL(listener, Notify(winss::SuperviseNotification::EXIT, _))
        .WillOnce(Return(true));

    MockedSupervise supervise(winss::NotOwned(&multiplexer), "dir");
    supervise.AddListener(winss::NotOwned(&listener));

    EXPECT_CALL(*supervise.GetMutex(), Lock()).WillOnce(Return(true));
    EXPECT_CALL(*supervise.GetMutex(), HasLock())
        .WillOnce(Return(false))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*supervise.GetProcess(), Create(_))
        .WillRepeatedly(Return(true));

    winss::HandleWrapper handle =
        winss::HandleWrapper(reinterpret_cast<HANDLE>(10000), false);
    EXPECT_CALL(*supervise.GetProcess(), GetHandle())
        .WillRepeatedly(Return(handle));

    multiplexer.mock_init_callbacks.at(0)(multiplexer);

    EXPECT_TRUE(supervise.GetState().is_up);
    EXPECT_TRUE(supervise.GetState().is_run_process);

    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle);
    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle);
    multiplexer.mock_timeout_callbacks.at(0)(multiplexer);

    EXPECT_TRUE(supervise.GetState().is_up);
    EXPECT_TRUE(supervise.GetState().is_run_process);

    supervise.Exit();
    supervise.Down();

    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle);
    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle);

    EXPECT_FALSE(supervise.GetState().is_up);
    EXPECT_FALSE(supervise.GetState().is_run_process);
}
}  // namespace winss
