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
