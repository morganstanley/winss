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

#include <string>
#include <thread>
#include <functional>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "winss/winss.hpp"
#include "winss/path_mutex.hpp"
#include "mock_interface.hpp"
#include "mock_windows_interface.hpp"

using ::testing::_;
using ::testing::NiceMock;

namespace winss {
class PathMutexTest : public testing::Test {
};

TEST_F(PathMutexTest, Name) {
    winss::PathMutex mutex1("does_not_exit", "");
    winss::PathMutex mutex2("does_not_exit", "test1");

    EXPECT_EQ(0, mutex1.GetName().find("Global\\"));
    EXPECT_EQ(std::string::npos, mutex1.GetName().find("_"));
    EXPECT_NE(std::string::npos, mutex2.GetName().find("_test1"));
}

TEST_F(PathMutexTest, Lock) {
    MockInterface<winss::MockWindowsInterface> windows;
    windows->SetupDefaults();

    EXPECT_CALL(*windows, ReleaseMutex(_)).Times(1);
    EXPECT_CALL(*windows, CloseHandle(_)).Times(1);

    winss::PathMutex mutex("does_not_exit", "test2");

    EXPECT_TRUE(mutex.CanLock());
    EXPECT_FALSE(mutex.HasLock());
    EXPECT_TRUE(mutex.Lock());
    EXPECT_TRUE(mutex.HasLock());
    EXPECT_TRUE(mutex.Lock());
    EXPECT_TRUE(mutex.HasLock());
    EXPECT_TRUE(mutex.CanLock());
}

TEST_F(PathMutexTest, MutlipleInstances) {
    MockInterface<winss::MockWindowsInterface> windows;
    windows->SetupDefaults();

    EXPECT_CALL(*windows, ReleaseMutex(_)).Times(1);
    EXPECT_CALL(*windows, CloseHandle(_)).Times(2);

    winss::PathMutex mutex1("does_not_exit", "test3");
    winss::PathMutex mutex2("does_not_exit", "test3");

    EXPECT_TRUE(mutex1.CanLock());
    EXPECT_TRUE(mutex2.CanLock());

    EXPECT_FALSE(mutex1.HasLock());
    EXPECT_FALSE(mutex2.HasLock());

    auto set = [](winss::PathMutex* mutex) {
        mutex->Lock();
    };

    std::thread mt1(set, &mutex1);
    std::thread mt2(set, &mutex2);

    mt1.join();
    mt2.join();

    EXPECT_NE(mutex1.HasLock(), mutex2.HasLock());
}
}  // namespace winss
