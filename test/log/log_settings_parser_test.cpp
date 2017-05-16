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
