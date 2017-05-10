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

#include <thread>
#include <functional>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "winss/winss.hpp"
#include "winss/event_wrapper.hpp"
#include "mock_interface.hpp"
#include "mock_windows_interface.hpp"

using ::testing::_;
using ::testing::NiceMock;

namespace winss {
class EventWrapperTest : public testing::Test {
};

TEST_F(EventWrapperTest, Set) {
    winss::EventWrapper e;

    EXPECT_FALSE(e.IsSet());
    EXPECT_TRUE(e.Set());
    EXPECT_TRUE(e.IsSet());
    EXPECT_TRUE(e.Set());
    EXPECT_TRUE(e.IsSet());
}

TEST_F(EventWrapperTest, Wait) {
    winss::EventWrapper e;

    std::thread bt([](winss::EventWrapper* evt) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        evt->Set();
    }, &e);

    auto result = e.GetHandle().Wait(1000);

    EXPECT_NE(TIMEOUT, result.state);
    EXPECT_EQ(e.GetHandle(), result.handle);
    EXPECT_TRUE(e.IsSet());

    bt.join();
}

TEST_F(EventWrapperTest, WaitTimeout) {
    winss::EventWrapper e;

    auto result = e.GetHandle().Wait(100);

    EXPECT_EQ(TIMEOUT, result.state);
    EXPECT_FALSE(e.IsSet());
}

TEST_F(EventWrapperTest, Close) {
    MockInterface<winss::MockWindowsInterface> windows;
    windows->SetupDefaults();

    EXPECT_CALL(*windows, CloseHandle(_)).Times(1);

    winss::EventWrapper e;
    e.GetHandle();  // Ensure CloseHandle is not called on handle wrapper
}

TEST_F(EventWrapperTest, DuplicateSet) {
    MockInterface<winss::MockWindowsInterface> windows;
    windows->SetupDefaults();

    winss::EventWrapper e;
    HANDLE h = e.GetHandle().Duplicate(false);

    EXPECT_NE(windows->WaitForSingleObject(h, 0), WAIT_OBJECT_0);

    EXPECT_FALSE(windows->SetEvent(h));
    EXPECT_FALSE(e.IsSet());
    EXPECT_TRUE(e.Set());
    EXPECT_TRUE(e.IsSet());

    EXPECT_EQ(windows->WaitForSingleObject(h, 0), WAIT_OBJECT_0);

    windows->CloseHandle(h);
}
}  // namespace winss
