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
