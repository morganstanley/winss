#include <filesystem>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "winss/winss.hpp"
#include "winss/log/log_settings_parser.hpp"
#include "../mock_interface.hpp"
#include "../mock_filesystem_interface.hpp"

namespace fs = std::experimental::filesystem;

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace winss {
class LogSettingsParserTest : public testing::Test {
};

TEST_F(LogSettingsParserTest, ParseDefault) {
    MockInterface<winss::MockFilesystemInterface> file;

    EXPECT_CALL(*file, Absolute(fs::path("."))).
        WillOnce(Return(fs::path("C:\\")));

    LogSettingsParser parser;
    winss::LogSettings settings = parser.Parse({ "n2", "s4096", "\\.", "g2" });

    EXPECT_EQ(2, settings.number);
    EXPECT_EQ(4096, settings.file_size);
    EXPECT_EQ(fs::path("C:\\"), settings.log_dir);
}
}  // namespace winss
