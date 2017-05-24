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
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "winss/winss.hpp"
#include "winss/svscan/service.hpp"
#include "../mock_interface.hpp"
#include "../mock_windows_interface.hpp"
#include "../mock_filesystem_interface.hpp"
#include "mock_service_process.hpp"

namespace fs = std::experimental::filesystem;

using ::testing::_;
using ::testing::NiceMock;
using ::testing::DoAll;
using ::testing::SetArgPointee;
using ::testing::Return;
using ::testing::ReturnRef;

namespace winss {
class ServiceTest : public testing::Test {
};
class MockedService :
    public winss::ServiceTmpl<winss::NiceMockServiceProcess> {
 public:
    explicit MockedService(std::string name) :
        winss::ServiceTmpl<winss::NiceMockServiceProcess>::ServiceTmpl(name) {}

    MockedService(const MockedService&) = delete;

    MockedService(MockedService&& s) :
        winss::ServiceTmpl<winss::NiceMockServiceProcess>
        ::ServiceTmpl(std::move(s)) {}

    winss::NiceMockServiceProcess* GetMain() {
        return &main;
    }

    winss::NiceMockServiceProcess* GetLog() {
        return &log;
    }

    void SetFlagged(bool flagged) {
        this->flagged = flagged;
    }

    void operator=(const MockedService&) = delete;

    MockedService& operator=(MockedService&& p) {
        winss::ServiceTmpl<winss::NiceMockServiceProcess>
            ::operator=(std::move(p));
        return *this;
    }
};

TEST_F(ServiceTest, Check) {
    MockInterface<winss::MockFilesystemInterface> file;

    EXPECT_CALL(*file, DirectoryExists(_))
        .WillOnce(Return(false));

    MockedService service("test");

    EXPECT_CALL(*service.GetMain(), IsCreated()).WillOnce(Return(false));
    EXPECT_CALL(*service.GetMain(), Start(_, false)).Times(1);

    EXPECT_FALSE(service.IsFlagged());
    EXPECT_EQ("test", service.GetName());
    service.Check();
    EXPECT_TRUE(service.IsFlagged());
}

TEST_F(ServiceTest, CheckCreated) {
    MockInterface<winss::MockFilesystemInterface> file;

    MockedService service("test");

    EXPECT_CALL(*service.GetMain(), IsCreated()).WillOnce(Return(true));
    EXPECT_CALL(*service.GetMain(), Start(_, false)).Times(0);

    service.Check();
    EXPECT_TRUE(service.IsFlagged());
}

TEST_F(ServiceTest, CheckLog) {
    MockInterface<winss::MockWindowsInterface> windows;
    MockInterface<winss::MockFilesystemInterface> file;

    EXPECT_CALL(*windows, CreatePipe(_, _, _, _))
        .WillOnce(DoAll(SetArgPointee<0>(reinterpret_cast<HANDLE>(10000)),
            SetArgPointee<1>(reinterpret_cast<HANDLE>(20000)), Return(true)));

    EXPECT_CALL(*file, DirectoryExists(_))
        .WillOnce(Return(true));

    MockedService service("test");

    EXPECT_CALL(*service.GetMain(), IsCreated()).WillOnce(Return(false));
    EXPECT_CALL(*service.GetMain(), Start(_, false)).Times(1);
    EXPECT_CALL(*service.GetLog(), Start(_, true)).Times(1);
    fs::path log(".\\log");
    EXPECT_CALL(*service.GetLog(), GetServiceDir())
        .WillOnce(ReturnRef(log));

    service.Check();

    EXPECT_EQ("test", service.GetName());
    EXPECT_TRUE(service.IsFlagged());
}

TEST_F(ServiceTest, Reset) {
    MockedService service("test");

    service.SetFlagged(true);
    service.Reset();

    EXPECT_FALSE(service.IsFlagged());
}

TEST_F(ServiceTest, Close) {
    MockedService service("test");

    service.SetFlagged(false);

    EXPECT_CALL(*service.GetMain(), Close()).Times(1);
    EXPECT_CALL(*service.GetLog(), Close()).Times(1);

    EXPECT_FALSE(service.Close(false));
}

TEST_F(ServiceTest, CloseFlagged) {
    MockedService service("test");

    service.SetFlagged(true);

    EXPECT_CALL(*service.GetMain(), Close()).Times(0);
    EXPECT_CALL(*service.GetLog(), Close()).Times(0);

    EXPECT_TRUE(service.Close(false));
}

TEST_F(ServiceTest, CloseIgnore) {
    MockedService service("test");

    service.SetFlagged(true);

    EXPECT_CALL(*service.GetMain(), Close()).Times(1);
    EXPECT_CALL(*service.GetLog(), Close()).Times(1);

    EXPECT_FALSE(service.Close(true));
}

TEST_F(ServiceTest, Move) {
    MockedService service1("test1");
    MockedService service2("test2");

    service1.SetFlagged(true);

    service2 = std::move(service1);
    EXPECT_EQ("test1", service2.GetName());
    EXPECT_EQ(true, service2.IsFlagged());

    MockedService service3(std::move(service2));
    EXPECT_EQ("test1", service3.GetName());
    EXPECT_EQ(true, service3.IsFlagged());
}
}  // namespace winss
