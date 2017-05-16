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
#include <chrono>
#include <map>
#include "gtest/gtest.h"
#include "winss/winss.hpp"
#include "winss/utils.hpp"

namespace winss {
class UtilsTest : public testing::Test {
};

TEST_F(UtilsTest, ExpandEnvironmentStringsMatch) {
    std::string value = "test1";
    SetEnvironmentVariable("TEST_ENV_VAR", value.c_str());

    std::string test = "%TEST_ENV_VAR%";
    std::string expanded = winss::Utils::ExpandEnvironmentVariables(test);

    EXPECT_EQ(value, expanded);
}

TEST_F(UtilsTest, ExpandEnvironmentStringsNoMatch) {
    std::string test = "this string does not contain any env variables";
    std::string expanded = winss::Utils::ExpandEnvironmentVariables(test);

    EXPECT_EQ(test, expanded);
}

TEST_F(UtilsTest, GetEnvironmentVariables) {
    auto env = winss::Utils::GetEnvironmentVariables();

    EXPECT_NE(env.end(), env.find("path"));
}

TEST_F(UtilsTest, TimeISOString) {
    auto now = std::chrono::system_clock::now();
    std::string now_string = winss::Utils::ConvertToISOString(now);
    auto decoded = winss::Utils::ConvertFromISOString(now_string);

    auto diff = std::chrono::duration_cast<std::chrono::seconds>(
        now - decoded).count();

    EXPECT_EQ(0, diff);
}
}  // namespace winss
