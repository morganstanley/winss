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
#include "gmock/gmock-matchers.h"
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

TEST_F(UtilsTest, GetEnvironmentString) {
    winss::env_t env;

    env["key1"] = "value1";
    env["key2"] = "value2";

    auto env_vec = winss::Utils::GetEnvironmentString(env);

    std::vector<char> cmp_env_vec = {
        'k', 'e', 'y', '1', '=', 'v', 'a', 'l', 'u', 'e', '1', '\0',
        'k', 'e', 'y', '2', '=', 'v', 'a', 'l', 'u', 'e', '2', '\0',
        '\0'
    };

    ASSERT_THAT(cmp_env_vec, ::testing::ElementsAreArray(env_vec));
}

TEST_F(UtilsTest, SplitStringMultiple) {
    std::string input = R"(
string1
string2
string3
)";

    std::vector<std::string> strings = winss::Utils::SplitString(input);

    EXPECT_EQ(3, strings.size());
    EXPECT_EQ("string1", strings.at(0));
    EXPECT_EQ("string2", strings.at(1));
    EXPECT_EQ("string3", strings.at(2));
}

TEST_F(UtilsTest, SplitStringSingle) {
    std::string input = "string1";

    std::vector<std::string> strings = winss::Utils::SplitString(input);

    EXPECT_EQ(1, strings.size());
    EXPECT_EQ("string1", strings.at(0));
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
