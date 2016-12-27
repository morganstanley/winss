#include <winss/winss.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <winss/control.hpp>
#include <winss/not_owning_ptr.hpp>
#include <winss/windows_interface.hpp>
#include <winss/wait_multiplexer.hpp>
#include <winss/pipe_client.hpp>
#include <string>
#include <vector>
#include "mock_interface.hpp"
#include "mock_windows_interface.hpp"
#include "mock_wait_multiplexer.hpp"
#include "mock_pipe_instance.hpp"
#include "mock_pipe_name.hpp"
#include "mock_pipe_client.hpp"
#include "mock_control.hpp"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::InvokeWithoutArgs;
using ::testing::Return;

namespace winss {
class ControlTest : public testing::Test {};
class MockControlItem : public winss::ControlItem {
 public:
    explicit MockControlItem(std::string name) :
        winss::ControlItem::ControlItem(name) {}

    MOCK_METHOD0(Init, void());
    MOCK_METHOD0(Start, void());
    MOCK_CONST_METHOD0(Completed, bool());
};

TEST_F(ControlTest, Single) {
    MockInterface<winss::MockWindowsInterface> windows;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;
    NiceMock<winss::MockControlItem> control_item("1");

    EXPECT_CALL(control_item, Start()).Times(1);
    EXPECT_CALL(multiplexer, Start()).WillOnce(Return(25));

    winss::Control control(winss::NotOwned(&multiplexer), INFINITE, false);

    control.Add(winss::NotOwned(&control_item));

    EXPECT_EQ(25, control.Start());

    EXPECT_FALSE(control.IsStarted());

    control.Ready("1");

    EXPECT_TRUE(control.IsStarted());
}

TEST_F(ControlTest, Empty) {
    MockInterface<winss::MockWindowsInterface> windows;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;

    EXPECT_CALL(multiplexer, Start()).Times(0);

    winss::Control control(winss::NotOwned(&multiplexer), INFINITE, false);

    EXPECT_EQ(0, control.Start());
    EXPECT_FALSE(control.IsStarted());
}

TEST_F(ControlTest, MultipleWaitAll) {
    MockInterface<winss::MockWindowsInterface> windows;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;
    NiceMock<winss::MockControlItem> control_item1("1");
    NiceMock<winss::MockControlItem> control_item2("2");

    EXPECT_CALL(control_item1, Start()).Times(1);
    EXPECT_CALL(control_item2, Start()).Times(1);

    winss::Control control(winss::NotOwned(&multiplexer), INFINITE, true);

    control.Add(winss::NotOwned(&control_item1));
    control.Add(winss::NotOwned(&control_item2));

    EXPECT_FALSE(control.IsStarted());

    control.Ready("1");

    EXPECT_FALSE(control.IsStarted());

    control.Ready("2");

    EXPECT_TRUE(control.IsStarted());

    control.Remove("1");

    EXPECT_CALL(multiplexer, Stop(_)).Times(1);

    control.Remove("2");
}

TEST_F(ControlTest, MultipleWaitOne) {
    MockInterface<winss::MockWindowsInterface> windows;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;
    NiceMock<winss::MockControlItem> control_item1("1");
    NiceMock<winss::MockControlItem> control_item2("2");

    EXPECT_CALL(control_item1, Start()).Times(1);
    EXPECT_CALL(control_item2, Start()).Times(1);

    winss::Control control(winss::NotOwned(&multiplexer), INFINITE, false);

    control.Add(winss::NotOwned(&control_item1));
    control.Add(winss::NotOwned(&control_item2));

    EXPECT_FALSE(control.IsStarted());

    control.Ready("1");

    EXPECT_FALSE(control.IsStarted());

    control.Ready("2");

    EXPECT_TRUE(control.IsStarted());

    EXPECT_CALL(multiplexer, Stop(_)).Times(1);

    control.Remove("1");
}

TEST_F(ControlTest, SingleRemove) {
    MockInterface<winss::MockWindowsInterface> windows;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;
    NiceMock<winss::MockControlItem> control_item("1");

    EXPECT_CALL(control_item, Start()).Times(0);
    EXPECT_CALL(multiplexer, Stop(0)).Times(1);

    winss::Control control(winss::NotOwned(&multiplexer), INFINITE, false);

    control.Add(winss::NotOwned(&control_item));
    control.Remove("1");
    control.Remove("1");
}

TEST_F(ControlTest, AlreadyStarted) {
    MockInterface<winss::MockWindowsInterface> windows;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;
    NiceMock<winss::MockControlItem> control_item("1");

    EXPECT_CALL(control_item, Start()).Times(1);

    winss::Control control(winss::NotOwned(&multiplexer), INFINITE, false);

    control.Add(winss::NotOwned(&control_item));
    control.Add(winss::NotOwned(&control_item));
    control.Ready("1");
    EXPECT_TRUE(control.IsStarted());
    control.Ready("1");
    EXPECT_TRUE(control.IsStarted());
}

TEST_F(ControlTest, Timeout) {
    MockInterface<winss::MockWindowsInterface> windows;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;
    NiceMock<winss::MockControlItem> control_item("1");

    EXPECT_CALL(control_item, Start()).Times(1);

    winss::Control control(winss::NotOwned(&multiplexer), 500, false);
    control.Add(winss::NotOwned(&control_item));

    multiplexer.mock_init_callbacks.at(0)(multiplexer);

    control.Ready("1");

    EXPECT_CALL(multiplexer, Stop(_)).Times(1);

    multiplexer.mock_timeout_callbacks.at(0)(multiplexer);
}

TEST_F(ControlTest, TimeoutCancelled) {
    MockInterface<winss::MockWindowsInterface> windows;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;
    NiceMock<winss::MockControlItem> control_item("1");

    EXPECT_CALL(control_item, Start()).Times(1);

    winss::Control control(winss::NotOwned(&multiplexer), 500, false);
    control.Add(winss::NotOwned(&control_item));

    multiplexer.mock_init_callbacks.at(0)(multiplexer);

    control.Ready("1");

    EXPECT_CALL(multiplexer, Stop(_)).Times(1);
    EXPECT_CALL(multiplexer, RemoveTimeoutCallback(_)).Times(1);

    control.Remove("1");

    multiplexer.mock_stop_callbacks.at(0)(multiplexer);
}

TEST_F(ControlTest, OutboundInbound) {
    MockInterface<winss::MockWindowsInterface> windows;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;
    winss::Control control(winss::NotOwned(&multiplexer));

    NiceMock<winss::MockOutboundPipeClient> outbound_client(
        winss::PipeClientConfig{ winss::MockPipeName("inbound"),
        winss::NotOwned(&multiplexer)
    });

    std::vector<char> commands{ 'a', 'b', 'c' };
    winss::OutboundControlItem outbound(winss::NotOwned(&multiplexer),
        winss::NotOwned(&control), winss::NotOwned(&outbound_client),
        commands, "");

    NiceMock<winss::MockInboundPipeClient> inbound_client(
        winss::PipeClientConfig{ winss::MockPipeName("outbound"),
        winss::NotOwned(&multiplexer)
    });

    NiceMock<winss::MockInboundControlItemListener> listener;
    EXPECT_CALL(listener, IsEnabled()).WillOnce(Return(true));
    winss::InboundControlItem inbound(winss::NotOwned(&multiplexer),
        winss::NotOwned(&control), winss::NotOwned(&inbound_client),
        winss::NotOwned(&listener), "");

    EXPECT_CALL(outbound_client, Connect()).Times(1);
    EXPECT_CALL(inbound_client, Connect()).Times(1);
    EXPECT_CALL(inbound_client, Stop()).Times(1);
    EXPECT_CALL(multiplexer, Stop(_)).Times(1);
    EXPECT_CALL(outbound_client, Send(_)).Times(1);
    EXPECT_CALL(listener, CanStart()).WillOnce(Return(true));
    EXPECT_CALL(listener, HandleRecieved(_)).WillOnce(Return(false));

    outbound.Init();
    inbound.Init();

    EXPECT_FALSE(outbound.Completed());
    EXPECT_FALSE(inbound.Completed());

    outbound.Connected();

    EXPECT_FALSE(control.IsStarted());
    inbound.Connected();
    EXPECT_TRUE(control.IsStarted());

    outbound.WriteComplete();
    outbound.Disconnected();

    inbound.Recieved(commands);
    inbound.Disconnected();

    EXPECT_TRUE(outbound.Completed());
    EXPECT_TRUE(inbound.Completed());
}

TEST_F(ControlTest, InboundDisabled) {
    MockInterface<winss::MockWindowsInterface> windows;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;
    winss::Control control(winss::NotOwned(&multiplexer));

    NiceMock<winss::MockOutboundPipeClient> outbound_client(
        winss::PipeClientConfig{ winss::MockPipeName("inbound"),
        winss::NotOwned(&multiplexer)
    });

    std::vector<char> commands{ 'a', 'b', 'c' };
    winss::OutboundControlItem outbound(winss::NotOwned(&multiplexer),
        winss::NotOwned(&control), winss::NotOwned(&outbound_client),
        commands, "");

    NiceMock<winss::MockInboundPipeClient> inbound_client(
        winss::PipeClientConfig{ winss::MockPipeName("outbound"),
        winss::NotOwned(&multiplexer)
    });

    NiceMock<winss::MockInboundControlItemListener> listener;
    EXPECT_CALL(listener, IsEnabled()).WillOnce(Return(false));
    winss::InboundControlItem inbound(winss::NotOwned(&multiplexer),
        winss::NotOwned(&control), winss::NotOwned(&inbound_client),
        winss::NotOwned(&listener), "");

    EXPECT_CALL(outbound_client, Connect()).Times(1);
    EXPECT_CALL(inbound_client, Connect()).Times(0);
    EXPECT_CALL(multiplexer, Stop(_)).Times(1);
    EXPECT_CALL(outbound_client, Send(_)).Times(1);

    outbound.Init();

    EXPECT_FALSE(control.IsStarted());
    outbound.Connected();
    EXPECT_TRUE(control.IsStarted());

    outbound.WriteComplete();
    outbound.Disconnected();
}

TEST_F(ControlTest, OutboundDisconnected) {
    MockInterface<winss::MockWindowsInterface> windows;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;
    winss::Control control(winss::NotOwned(&multiplexer));

    NiceMock<winss::MockOutboundPipeClient> outbound_client(
        winss::PipeClientConfig{ winss::MockPipeName("inbound"),
        winss::NotOwned(&multiplexer)
    });

    std::vector<char> commands{ 'a', 'b', 'c' };
    winss::OutboundControlItem outbound(winss::NotOwned(&multiplexer),
        winss::NotOwned(&control), winss::NotOwned(&outbound_client),
        commands, "");

    // Stops the multiplexer when disconnected without sending
    EXPECT_CALL(multiplexer, Stop(_)).Times(1);

    outbound.Disconnected();
}

TEST_F(ControlTest, InboundCannotStart) {
    MockInterface<winss::MockWindowsInterface> windows;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;
    winss::Control control(winss::NotOwned(&multiplexer));

    NiceMock<winss::MockInboundPipeClient> inbound_client(
        winss::PipeClientConfig{ winss::MockPipeName("outbound"),
        winss::NotOwned(&multiplexer)
    });

    NiceMock<winss::MockInboundControlItemListener> listener;
    EXPECT_CALL(listener, IsEnabled()).WillOnce(Return(true));
    winss::InboundControlItem inbound(winss::NotOwned(&multiplexer),
        winss::NotOwned(&control), winss::NotOwned(&inbound_client),
        winss::NotOwned(&listener), "");

    EXPECT_CALL(multiplexer, Stop(_)).Times(1);
    EXPECT_CALL(listener, CanStart()).WillOnce(Return(false));

    inbound.Init();
    inbound.Start();
    inbound.Disconnected();
}
}  // namespace winss
