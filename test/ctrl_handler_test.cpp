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

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "winss/winss.hpp"
#include "winss/ctrl_handler.hpp"
#include "winss/windows_interface.hpp"
#include "mock_interface.hpp"
#include "mock_windows_interface.hpp"

using ::testing::_;
using ::testing::NiceMock;

namespace winss {
class CtrlHandlerTest : public testing::Test {
};

TEST_F(CtrlHandlerTest, SetConsoleCtrlHandler) {
    MockInterface<winss::MockWindowsInterface> windows;
    windows->SetupDefaults();

    EXPECT_CALL(*windows, SetConsoleCtrlHandler(_, _)).Times(2);

    winss::AttachCtrlHandler();

    EXPECT_FALSE(winss::GetCloseEvent().IsSet());

    // Logoff should not trigger close event
    winss::CtrlHandler(CTRL_LOGOFF_EVENT);

    EXPECT_FALSE(winss::GetCloseEvent().IsSet());

    winss::CtrlHandler(CTRL_C_EVENT);

    EXPECT_TRUE(winss::GetCloseEvent().IsSet());

    EXPECT_TRUE(WINDOWS.SetConsoleCtrlHandler(winss::CtrlHandler, false));
}
}  // namespace winss
