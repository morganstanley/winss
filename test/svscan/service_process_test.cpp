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
#include "winss/winss.hpp"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "winss/handle_wrapper.hpp"
#include "winss/svscan/service_process.hpp"
#include "../mock_process.hpp"

namespace fs = std::experimental::filesystem;

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace winss {
class ServiceProcessTest : public testing::Test {
};
class MockedServiceProcess :
    public winss::ServiceProcessTmpl<winss::NiceMockProcess> {
 public:
    explicit MockedServiceProcess(const fs::path& service_dir) :
        winss::ServiceProcessTmpl<winss::NiceMockProcess>
        ::ServiceProcessTmpl(service_dir) {}

    MockedServiceProcess(const MockedServiceProcess&) = delete;

    MockedServiceProcess(MockedServiceProcess&& p) :
        winss::ServiceProcessTmpl<winss::NiceMockProcess>
        ::ServiceProcessTmpl(std::move(p)) {}

    winss::MockProcess* GetProcess() {
        return &proc;
    }

    MockedServiceProcess& operator=(const MockedServiceProcess&) = delete;

    MockedServiceProcess& operator=(MockedServiceProcess&& p) {
        winss::ServiceProcessTmpl<NiceMockProcess>::operator=(std::move(p));
        return *this;
    }
};

TEST_F(ServiceProcessTest, Start) {
    MockedServiceProcess service_process(".");

    EXPECT_CALL(*service_process.GetProcess(), Create(_))
        .WillOnce(Return(true));
    EXPECT_CALL(*service_process.GetProcess(), IsCreated())
        .WillOnce(Return(false))
        .WillOnce(Return(false))
        .WillOnce(Return(true));

    EXPECT_FALSE(service_process.IsCreated());
    service_process.Start(winss::ServicePipes{}, false);
    // This should not invoke Create again because IsCreated is true.
    service_process.Start(winss::ServicePipes{}, false);
}

MATCHER(IS_PRODUCER, "") {
    return (arg.stdout_pipe.HasHandle() && arg.stdout_pipe.HasHandle()
        && !arg.stdin_pipe.HasHandle());
}

MATCHER(IS_CONSUMER, "") {
    return (!arg.stdout_pipe.HasHandle() && !arg.stdout_pipe.HasHandle()
        && arg.stdin_pipe.HasHandle());
}

TEST_F(ServiceProcessTest, StartProducerConsumer) {
    MockedServiceProcess producer(".");
    MockedServiceProcess consumer(".");

    auto pipes = winss::ServicePipes{
        winss::HandleWrapper(reinterpret_cast<HANDLE>(1), false),
        winss::HandleWrapper(reinterpret_cast<HANDLE>(2), false)
    };

    EXPECT_CALL(*producer.GetProcess(), Create(IS_PRODUCER()))
        .WillOnce(Return(true));
    EXPECT_CALL(*consumer.GetProcess(), Create(IS_CONSUMER()))
        .WillOnce(Return(true));

    EXPECT_CALL(*producer.GetProcess(), IsCreated())
        .WillOnce(Return(false));

    EXPECT_CALL(*consumer.GetProcess(), IsCreated())
        .WillOnce(Return(false));

    producer.Start(pipes, false);
    consumer.Start(pipes, true);
}

TEST_F(ServiceProcessTest, Close) {
    MockedServiceProcess service_process(".");

    EXPECT_CALL(*service_process.GetProcess(), SendBreak()).Times(1);
    EXPECT_CALL(*service_process.GetProcess(), IsCreated())
        .WillOnce(Return(false))
        .WillOnce(Return(true));

    service_process.Close();
    service_process.Close();
}

TEST_F(ServiceProcessTest, Move) {
    MockedServiceProcess service_process1("C:\\1");
    MockedServiceProcess service_process2("C:\\2");

    EXPECT_EQ(fs::path("C:\\2"), service_process2.GetServiceDir());
    service_process2 = std::move(service_process1);
    EXPECT_EQ(fs::path("C:\\1"), service_process2.GetServiceDir());

    MockedServiceProcess service_process3(std::move(service_process2));
    EXPECT_EQ(fs::path("C:\\1"), service_process3.GetServiceDir());
}
}  // namespace winss
