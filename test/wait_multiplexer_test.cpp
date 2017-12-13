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

#include <chrono>
#include <thread>
#include <functional>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "winss/winss.hpp"
#include "winss/wait_multiplexer.hpp"
#include "winss/handle_wrapper.hpp"
#include "winss/event_wrapper.hpp"
#include "mock_interface.hpp"
#include "mock_windows_interface.hpp"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::InvokeWithoutArgs;

namespace winss {
class WaitMultiplexerTest : public testing::Test {
};

TEST_F(WaitMultiplexerTest, WaitTimeoutItemOrdering) {
    auto now = std::chrono::system_clock::now();
    auto later = now + std::chrono::milliseconds(5000);

    auto callback1 = [](winss::WaitMultiplexer&) {};
    auto callback2 = [](winss::WaitMultiplexer&) {};

    winss::WaitTimeoutItem item1({ "", now, callback1 });
    winss::WaitTimeoutItem item2({ "", later, callback2 });

    EXPECT_LT(item1, item2);
}

TEST_F(WaitMultiplexerTest, RemoveTriggered) {
    winss::WaitMultiplexer multiplexer;
    winss::HandleWrapper handle1(reinterpret_cast<HANDLE>(10000), false);
    winss::HandleWrapper handle2(reinterpret_cast<HANDLE>(20000), false);

    auto callback1 = [](winss::WaitMultiplexer&,
        const winss::HandleWrapper&) {};
    auto callback2 = [](winss::WaitMultiplexer&,
        const winss::HandleWrapper&) {};

    EXPECT_FALSE(multiplexer.RemoveTriggeredCallback(handle1));
    multiplexer.AddTriggeredCallback(handle1, callback1);
    multiplexer.AddTriggeredCallback(handle2, callback2);
    EXPECT_TRUE(multiplexer.RemoveTriggeredCallback(handle1));
    EXPECT_FALSE(multiplexer.RemoveTriggeredCallback(handle1));
}

TEST_F(WaitMultiplexerTest, RemoveTimeout) {
    winss::WaitMultiplexer multiplexer;

    auto callback = [](winss::WaitMultiplexer&) {};
    multiplexer.AddTimeoutCallback(1000, callback, "test1");
    multiplexer.AddTimeoutCallback(1001, callback, "test2");
    EXPECT_NE(INFINITE, multiplexer.GetTimeout());
    EXPECT_TRUE(multiplexer.RemoveTimeoutCallback("test1"));
    EXPECT_FALSE(multiplexer.RemoveTimeoutCallback("test1"));
    EXPECT_NE(INFINITE, multiplexer.GetTimeout());
    EXPECT_TRUE(multiplexer.RemoveTimeoutCallback("test2"));
    EXPECT_EQ(INFINITE, multiplexer.GetTimeout());
}

TEST_F(WaitMultiplexerTest, GetTimeoutEmpty) {
    winss::WaitMultiplexer multiplexer;

    auto callback = [](winss::WaitMultiplexer&) {};
    multiplexer.AddTimeoutCallback(0, callback, "test1");
    EXPECT_EQ(0, multiplexer.GetTimeout());
}

TEST_F(WaitMultiplexerTest, StartEmpty) {
    winss::WaitMultiplexer multiplexer;

    EXPECT_EQ(0, multiplexer.Start());;
}

TEST_F(WaitMultiplexerTest, Start) {
    MockInterface<winss::MockWindowsInterface> windows;
    winss::WaitMultiplexer multiplexer;

    int init = 0;
    auto callback1 = [&init](winss::WaitMultiplexer&) {
        init++;
    };
    int triggered = 0;
    auto callback2 = [&triggered](winss::WaitMultiplexer&,
        const winss::HandleWrapper&) {
        triggered++;
    };
    auto callback3 = [&triggered](winss::WaitMultiplexer&,
        const winss::HandleWrapper&) {
        triggered++;
    };

    EXPECT_CALL(*windows, WaitForMultipleObjects(_, _, _, _))
        .WillOnce(Return(WAIT_OBJECT_0 + 1))
        .WillOnce(Return(WAIT_FAILED));

    HANDLE handle1 = reinterpret_cast<HANDLE>(10000);
    winss::HandleWrapper wrapper1(handle1, false);

    HANDLE handle2 = reinterpret_cast<HANDLE>(20000);
    winss::HandleWrapper wrapper2(handle2, false);

    multiplexer.AddInitCallback(callback1);
    multiplexer.AddTriggeredCallback(wrapper1, callback2);
    multiplexer.AddTriggeredCallback(wrapper2, callback3);

    EXPECT_EQ(0, multiplexer.Start());

    EXPECT_EQ(1, init);
    EXPECT_EQ(1, triggered);
}

TEST_F(WaitMultiplexerTest, StartTimeoutEmpty) {
    MockInterface<winss::MockWindowsInterface> windows;
    winss::WaitMultiplexer multiplexer;

    EXPECT_CALL(*windows, WaitForMultipleObjects(_, _, _, _))
        .WillOnce(Return(WAIT_OBJECT_0));

    int timeout = 0;
    auto callback = [&timeout](winss::WaitMultiplexer&) {
        timeout++;
    };

    multiplexer.AddTimeoutCallback(0, callback);

    HANDLE handle = reinterpret_cast<HANDLE>(10000);
    winss::HandleWrapper wrapper(handle, false);

    int triggered = 0;
    auto callback2 = [&triggered](winss::WaitMultiplexer&,
        const winss::HandleWrapper&) {
        triggered++;
    };

    multiplexer.AddTriggeredCallback(wrapper, callback2);

    EXPECT_EQ(0, multiplexer.Start());
    EXPECT_EQ(1, timeout);
    EXPECT_EQ(1, triggered);
}

TEST_F(WaitMultiplexerTest, StartTimeout) {
    MockInterface<winss::MockWindowsInterface> windows;
    winss::WaitMultiplexer multiplexer;

    EXPECT_CALL(*windows, WaitForMultipleObjects(_, _, _, _))
        .WillOnce(Return(WAIT_TIMEOUT))
        .WillOnce(Return(WAIT_FAILED));

    HANDLE handle1 = reinterpret_cast<HANDLE>(10000);
    winss::HandleWrapper wrapper1(handle1, false);

    int timeout = 0;
    auto callback = [&timeout](winss::WaitMultiplexer&) {
        timeout++;
    };

    multiplexer.AddTimeoutCallback(100, callback);

    HANDLE handle = reinterpret_cast<HANDLE>(10000);
    winss::HandleWrapper wrapper(handle, false);

    int triggered = 0;
    auto callback2 = [&triggered](winss::WaitMultiplexer&,
        const winss::HandleWrapper&) {
        triggered++;
    };

    multiplexer.AddTriggeredCallback(wrapper, callback2);

    EXPECT_EQ(0, multiplexer.Start());

    EXPECT_EQ(1, timeout);
    EXPECT_EQ(0, triggered);
}

TEST_F(WaitMultiplexerTest, StartWhenStopped) {
    winss::WaitMultiplexer multiplexer;

    int init = 0;
    auto callback1 = [&init](winss::WaitMultiplexer&) {
        init++;
    };

    int stop = 0;
    auto callback2 = [&stop](winss::WaitMultiplexer&) {
        stop++;
    };

    multiplexer.AddInitCallback(callback1);
    multiplexer.AddStopCallback(callback2);

    multiplexer.Stop(5);
    EXPECT_TRUE(multiplexer.IsStopping());
    EXPECT_FALSE(multiplexer.HasStarted());
    EXPECT_EQ(5, multiplexer.Start());

    EXPECT_EQ(0, init);
    EXPECT_EQ(1, stop);
}

TEST_F(WaitMultiplexerTest, Stop) {
    MockInterface<winss::MockWindowsInterface> windows;
    winss::WaitMultiplexer multiplexer;

    EXPECT_CALL(*windows, WaitForMultipleObjects(_, _, _, _))
        .WillOnce(Return(WAIT_OBJECT_0));

    auto callback1 = [&multiplexer](winss::WaitMultiplexer&,
        const winss::HandleWrapper&) {
        multiplexer.Stop(5);
    };

    HANDLE handle1 = reinterpret_cast<HANDLE>(10000);
    winss::HandleWrapper wrapper1(handle1, false);

    auto callback2 = [&multiplexer, &wrapper1](winss::WaitMultiplexer&) {
        multiplexer.RemoveTriggeredCallback(wrapper1);
    };

    multiplexer.AddTriggeredCallback(wrapper1, callback1);
    multiplexer.AddStopCallback(callback2);

    multiplexer.Start();
    EXPECT_EQ(5, multiplexer.GetReturnCode());
}

TEST_F(WaitMultiplexerTest, CloseWaitTriggeredCallback) {
    MockInterface<winss::MockWindowsInterface> windows;
    windows->SetupDefaults();
    winss::EventWrapper close_event;
    winss::WaitMultiplexer multiplexer;
    multiplexer.AddCloseEvent(close_event, 10);

    std::thread bt([](winss::EventWrapper* evt) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        evt->Set();
    }, &close_event);

    multiplexer.Start();
    bt.join();
    EXPECT_EQ(10, multiplexer.GetReturnCode());
}
}  // namespace winss
