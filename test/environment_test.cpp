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
#include <vector>
#include <algorithm>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "winss/winss.hpp"
#include "winss/environment.hpp"
#include "winss/filesystem_interface.hpp"
#include "mock_interface.hpp"
#include "mock_windows_interface.hpp"
#include "mock_filesystem_interface.hpp"

namespace fs = std::experimental::filesystem;

using ::testing::_;
using ::testing::Return;

namespace winss {
class EnvrionmentTest : public testing::Test {
};
class MockEnvironment: public winss::Environment {
    winss::env_t ReadEnvSource() override {
        winss::env_t env;

        env["test1"] = "value1";
        env["test2"] = "value2";
        env["path"] = "";

        return env;
    }
};
class MockEnvironmentEmpty : public winss::Environment {
    winss::env_t ReadEnvSource() override {
        winss::env_t env;
        return env;
    }
};

TEST_F(EnvrionmentTest, ReadEnv) {
    MockInterface<winss::MockWindowsInterface> windows;
    MockEnvironment env;

    char* env_array = "other=value\0path=123\0\0";

    EXPECT_CALL(*windows, GetEnvironmentStrings())
        .WillOnce(Return(env_array));

    auto env_vector = env.ReadEnv();

    const char *test1 = "test1=value1";
    EXPECT_NE(env_vector.end(),
        std::search(
            env_vector.begin(),
            env_vector.end(),
            test1, test1 + strlen(test1)));

    const char *test2 = "path";
    EXPECT_EQ(env_vector.end(),
        std::search(
            env_vector.begin(),
            env_vector.end(),
            test2, test2 + strlen(test2)));
}

TEST_F(EnvrionmentTest, ReadEnvEmpty) {
    MockInterface<winss::MockWindowsInterface> windows;
    MockEnvironmentEmpty env;

    EXPECT_CALL(*windows, GetEnvironmentStrings()).Times(0);

    auto env_vector = env.ReadEnv();

    EXPECT_TRUE(env_vector.empty());
}

TEST_F(EnvrionmentTest, ReadEnvironmentDir) {
    MockInterface<winss::MockWindowsInterface> windows;
    MockInterface<winss::MockFilesystemInterface> file;
    winss::EnvironmentDir env_dir("test");

    EXPECT_CALL(*file, Read(_))
        .WillOnce(Return(""))
        .WillOnce(Return("value1"))
        .WillOnce(Return("value2"))
        .WillOnce(Return(""));

    EXPECT_CALL(*file, GetFiles(_))
        .WillOnce(Return(std::vector<fs::path>{"key1", "key2", "key3"}));

    auto env = env_dir.ReadEnvSource();

    EXPECT_EQ(3, env.size());
    EXPECT_EQ("value1", env["key1"]);
    EXPECT_EQ("value2", env["key2"]);
    EXPECT_EQ("", env["key3"]);
}

TEST_F(EnvrionmentTest, ReadEnvironmentDirFile) {
    MockInterface<winss::MockWindowsInterface> windows;
    MockInterface<winss::MockFilesystemInterface> file;
    winss::EnvironmentDir env_dir("test");

    EXPECT_CALL(*file, Read(_))
        .WillOnce(Return("test1\ntest2\n"))
        .WillOnce(Return("value1"))
        .WillOnce(Return("value2"))
        .WillOnce(Return(""));

    EXPECT_CALL(*file, GetFiles(fs::path("test1")))
        .WillOnce(Return(std::vector<fs::path>{"key1"}));

    EXPECT_CALL(*file, GetFiles(fs::path("test2")))
        .WillOnce(Return(std::vector<fs::path>{"key2", "key3"}));

    auto env = env_dir.ReadEnvSource();

    EXPECT_EQ(3, env.size());
    EXPECT_EQ("value1", env["key1"]);
    EXPECT_EQ("value2", env["key2"]);
    EXPECT_EQ("", env["key3"]);
}

TEST_F(EnvrionmentTest, ReadEnvironmentDirSourceNotExists) {
    MockInterface<winss::MockWindowsInterface> windows;
    MockInterface<winss::MockFilesystemInterface> file;
    winss::EnvironmentDir env_dir("test");

    EXPECT_CALL(*file, Read(_)).WillOnce(Return(""));
    EXPECT_CALL(*file, GetFiles(_)).WillOnce(Return(std::vector<fs::path>()));

    auto env = env_dir.ReadEnvSource();

    EXPECT_TRUE(env.empty());
}
}  // namespace winss
