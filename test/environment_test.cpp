#include <winss/winss.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <winss/environment.hpp>
#include <winss/filesystem_interface.hpp>
#include <filesystem>
#include <vector>
#include <algorithm>
#include "mock_interface.hpp"
#include "mock_windows_interface.hpp"
#include "mock_filesystem_interface.hpp"

namespace fs = std::experimental::filesystem;

using ::testing::_;
using ::testing::Return;

namespace winss {
class EnvrionmentTest : public testing::Test {
};

TEST_F(EnvrionmentTest, ReadEnvironmentDir) {
    MockInterface<winss::MockWindowsInterface> windows;
    MockInterface<winss::MockFilesystemInterface> file;
    winss::EnvironmentDir env_dir("test");

    char* env_array = "test=1\0path=123\0\0";

    EXPECT_CALL(*windows, GetEnvironmentStrings())
        .WillOnce(Return(env_array));

    EXPECT_CALL(*file, DirectoryExists(_)).WillOnce(Return(true));

    EXPECT_CALL(*file, GetFiles(_)).WillOnce(Return(std::vector<fs::path>({
        "path",
        "testing_1",
        ".testing_2",
        "testing=3"
    })));

    EXPECT_CALL(*file, Read(_))
        .WillOnce(Return(""))
        .WillOnce(Return("value_1"));

    auto env_vector = env_dir.ReadEnv();

    const char *test1 = "testing_1=value_1";
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

TEST_F(EnvrionmentTest, ReadEnvironmentDirNotExists) {
    MockInterface<winss::MockWindowsInterface> windows;
    MockInterface<winss::MockFilesystemInterface> file;
    winss::EnvironmentDir env_dir("test");

    char* env_array = "test=1\0path=123\0\0";

    EXPECT_CALL(*windows, GetEnvironmentStrings())
        .WillOnce(Return(env_array));

    EXPECT_CALL(*file, DirectoryExists(_)).WillOnce(Return(false));

    EXPECT_CALL(*file, GetFiles(_)).Times(0);
    EXPECT_CALL(*file, Read(_)).Times(0);

    auto env_vector = env_dir.ReadEnv();

    EXPECT_TRUE(env_vector.empty());
}
}  // namespace winss
