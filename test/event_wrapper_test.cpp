#include <winss/winss.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <winss/event_wrapper.hpp>
#include <thread>
#include <functional>
#include "mock_interface.hpp"
#include "mock_windows_interface.hpp"

using ::testing::_;
using ::testing::NiceMock;

namespace winss {
class EventWrapperTest : public testing::Test {
};

TEST_F(EventWrapperTest, Set) {
    winss::EventWrapper e;

    EXPECT_FALSE(e.IsSet());
    EXPECT_TRUE(e.Set());
    EXPECT_TRUE(e.IsSet());
    EXPECT_TRUE(e.Set());
    EXPECT_TRUE(e.IsSet());
}

TEST_F(EventWrapperTest, Wait) {
    winss::EventWrapper e;

    std::thread bt([](winss::EventWrapper* evt) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        evt->Set();
    }, &e);

    auto result = e.GetHandle().Wait(1000);

    EXPECT_NE(TIMEOUT, result.state);
    EXPECT_EQ(e.GetHandle(), result.handle);
    EXPECT_TRUE(e.IsSet());

    bt.join();
}

TEST_F(EventWrapperTest, WaitTimeout) {
    winss::EventWrapper e;

    auto result = e.GetHandle().Wait(100);

    EXPECT_EQ(TIMEOUT, result.state);
    EXPECT_FALSE(e.IsSet());
}

TEST_F(EventWrapperTest, Close) {
    MockInterface<winss::MockWindowsInterface> windows;
    windows->SetupDefaults();

    EXPECT_CALL(*windows, CloseHandle(_)).Times(1);

    winss::EventWrapper e;
    e.GetHandle();  // Ensure CloseHandle is not called on handle wrapper
}

TEST_F(EventWrapperTest, DuplicateSet) {
    MockInterface<winss::MockWindowsInterface> windows;
    windows->SetupDefaults();

    winss::EventWrapper e;
    HANDLE h = e.GetHandle().Duplicate(false);

    EXPECT_NE(windows->WaitForSingleObject(h, 0), WAIT_OBJECT_0);

    EXPECT_FALSE(windows->SetEvent(h));
    EXPECT_FALSE(e.IsSet());
    EXPECT_TRUE(e.Set());
    EXPECT_TRUE(e.IsSet());

    EXPECT_EQ(windows->WaitForSingleObject(h, 0), WAIT_OBJECT_0);

    windows->CloseHandle(h);
}
}  // namespace winss
