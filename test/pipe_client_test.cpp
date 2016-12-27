#include <winss/winss.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <winss/not_owning_ptr.hpp>
#include <winss/pipe_client.hpp>
#include <winss/handle_wrapper.hpp>
#include <vector>
#include "mock_interface.hpp"
#include "mock_windows_interface.hpp"
#include "mock_wait_multiplexer.hpp"
#include "mock_pipe_name.hpp"
#include "mock_pipe_instance.hpp"
#include "mock_pipe_client.hpp"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace winss {
class PipeClientTest : public testing::Test {
};
class MockedOutboundPipeClient : public winss::OutboundPipeClientTmpl<
    winss::NiceMockOutboundPipeInstance> {
 public:
    explicit MockedOutboundPipeClient(const PipeClientConfig& config) :
        winss::OutboundPipeClientTmpl<winss::NiceMockOutboundPipeInstance>
        ::OutboundPipeClientTmpl(config) {}

    winss::NiceMockOutboundPipeInstance* GetInstance() {
        return &instance;
    }
};
class MockedInboundPipeClient : public winss::InboundPipeClientTmpl<
    winss::NiceMockInboundPipeInstance> {
 public:
    explicit MockedInboundPipeClient(const PipeClientConfig& config) :
        winss::InboundPipeClientTmpl<winss::NiceMockInboundPipeInstance>
        ::InboundPipeClientTmpl(config) {}

    winss::NiceMockInboundPipeInstance* GetInstance() {
        return &instance;
    }
};

TEST_F(PipeClientTest, OuboundConnect) {
    MockInterface<winss::MockWindowsInterface> windows;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;
    NiceMock<winss::MockPipeClientSendRecieveListener> listener;

    EXPECT_CALL(listener, Connected()).WillOnce(Return(true));

    winss::MockPipeName pipe_name("test");
    MockedOutboundPipeClient client({
        pipe_name, winss::NotOwned(&multiplexer)
    });

    client.AddListener(winss::NotOwned(&listener));

    EXPECT_CALL(*client.GetInstance(), SetConnected())
        .WillOnce(Return(true));

    client.Connect();
}

TEST_F(PipeClientTest, OuboundConnectFailed) {
    MockInterface<winss::MockWindowsInterface> windows;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;
    NiceMock<winss::MockPipeClientSendRecieveListener> listener;

    EXPECT_CALL(listener, Disconnected()).WillOnce(Return(true));

    winss::MockPipeName pipe_name("test");
    MockedOutboundPipeClient client({
        pipe_name, winss::NotOwned(&multiplexer)
    });

    client.AddListener(winss::NotOwned(&listener));

    EXPECT_CALL(*client.GetInstance(), CreateFile(_)).WillOnce(Return(false));

    client.Connect();
}

TEST_F(PipeClientTest, OuboundRemove) {
    MockInterface<winss::MockWindowsInterface> windows;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;
    NiceMock<winss::MockPipeClientSendRecieveListener> listener;

    EXPECT_CALL(listener, Connected()).WillOnce(Return(true));
    EXPECT_CALL(listener, Disconnected()).WillOnce(Return(true));

    winss::MockPipeName pipe_name("test");
    MockedOutboundPipeClient client({
        pipe_name, winss::NotOwned(&multiplexer)
    });

    client.AddListener(winss::NotOwned(&listener));

    EXPECT_CALL(*client.GetInstance(), SetConnected())
        .WillOnce(Return(true));

    client.Connect();

    EXPECT_CALL(*client.GetInstance(), GetOverlappedResult())
        .WillOnce(Return(REMOVE));

    EXPECT_CALL(*client.GetInstance(), Close())
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    multiplexer.mock_triggered_callbacks.at(0)(multiplexer,
        client.GetInstance()->GetHandle());
}

TEST_F(PipeClientTest, OuboundStop) {
    MockInterface<winss::MockWindowsInterface> windows;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;
    NiceMock<winss::MockPipeClientSendRecieveListener> listener;

    EXPECT_CALL(listener, Connected()).WillOnce(Return(true));
    EXPECT_CALL(listener, Disconnected()).Times(0);

    winss::MockPipeName pipe_name("test");
    MockedOutboundPipeClient client({
        pipe_name, winss::NotOwned(&multiplexer)
    });

    client.AddListener(winss::NotOwned(&listener));

    EXPECT_CALL(*client.GetInstance(), SetConnected())
        .WillOnce(Return(true));

    client.Connect();

    EXPECT_CALL(*client.GetInstance(), Closing()).Times(1);

    client.Stop();

    EXPECT_TRUE(client.IsStopping());
}

TEST_F(PipeClientTest, OuboundSend) {
    MockInterface<winss::MockWindowsInterface> windows;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;
    NiceMock<winss::MockPipeClientSendRecieveListener> listener;

    EXPECT_CALL(listener, Connected()).WillOnce(Return(true));
    EXPECT_CALL(listener, WriteComplete()).WillOnce(Return(false));

    winss::MockPipeName pipe_name("test");
    MockedOutboundPipeClient client({
        pipe_name, winss::NotOwned(&multiplexer)
    });

    client.AddListener(winss::NotOwned(&listener));

    EXPECT_CALL(*client.GetInstance(), SetConnected())
        .WillOnce(Return(true));

    client.Connect();

    EXPECT_CALL(*client.GetInstance(), GetOverlappedResult())
        .WillOnce(Return(CONTINUE))
        .WillOnce(Return(SKIP))
        .WillOnce(Return(CONTINUE));

    EXPECT_CALL(*client.GetInstance(), Queue(_)).WillOnce(Return(true));
    EXPECT_CALL(*client.GetInstance(), Write())
        .WillOnce(Return(true))
        .WillOnce(Return(true));

    EXPECT_CALL(*client.GetInstance(), FinishWrite())
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    EXPECT_CALL(*client.GetInstance(), IsWriting())
        .WillOnce(Return(true));

    client.Send({ '1', '2' });

    multiplexer.mock_triggered_callbacks.at(0)(multiplexer,
        client.GetInstance()->GetHandle());
    multiplexer.mock_triggered_callbacks.at(0)(multiplexer,
        client.GetInstance()->GetHandle());
    multiplexer.mock_triggered_callbacks.at(0)(multiplexer,
        client.GetInstance()->GetHandle());
}

TEST_F(PipeClientTest, InboundRead) {
    MockInterface<winss::MockWindowsInterface> windows;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;
    NiceMock<winss::MockPipeClientSendRecieveListener> listener;

    EXPECT_CALL(listener, Recieved(_)).WillOnce(Return(false));

    winss::MockPipeName pipe_name("test");
    MockedInboundPipeClient client({
        pipe_name, winss::NotOwned(&multiplexer)
    });

    client.AddListener(winss::NotOwned(&listener));

    EXPECT_CALL(*client.GetInstance(), SetConnected())
        .WillOnce(Return(true));

    client.Connect();

    EXPECT_CALL(*client.GetInstance(), GetOverlappedResult())
        .WillRepeatedly(Return(CONTINUE));

    EXPECT_CALL(*client.GetInstance(), FinishRead())
        .WillOnce(Return(true))
        .WillOnce(Return(false))
        .WillOnce(Return(true));

    EXPECT_CALL(*client.GetInstance(), SwapBuffer())
        .WillOnce(Return(std::vector<char>{0}))
        .WillOnce(Return(std::vector<char>{'1'}));

    // Expect it after connect to make sure null byte nogotiate works.
    EXPECT_CALL(listener, Connected()).WillOnce(Return(true));

    multiplexer.mock_triggered_callbacks.at(0)(multiplexer,
        client.GetInstance()->GetHandle());
    multiplexer.mock_triggered_callbacks.at(0)(multiplexer,
        client.GetInstance()->GetHandle());
    multiplexer.mock_triggered_callbacks.at(0)(multiplexer,
        client.GetInstance()->GetHandle());
}
}  // namespace winss
