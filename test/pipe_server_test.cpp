#include <winss/winss.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <winss/not_owning_ptr.hpp>
#include <winss/pipe_server.hpp>
#include <winss/handle_wrapper.hpp>
#include <vector>
#include <map>
#include "mock_interface.hpp"
#include "mock_windows_interface.hpp"
#include "mock_wait_multiplexer.hpp"
#include "mock_pipe_name.hpp"
#include "mock_pipe_instance.hpp"
#include "mock_pipe_server.hpp"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::An;

namespace winss {
class PipeServerTest : public testing::Test {
 public:
    template<typename TInstance>
    TInstance* GetOther(std::map<winss::HandleWrapper, TInstance>* instances,
        const winss::HandleWrapper& handle) {
        TInstance* other = nullptr;
        for (auto it = instances->begin();
            it != instances->end(); ++it) {
            if (it->first != handle) {
                other = &it->second;
                break;
            }
        }

        return other;
    }
};
class MockedPipeServer :
    public winss::PipeServer<winss::NiceMockPipeInstance> {
 public:
    explicit MockedPipeServer(const PipeServerConfig& config) :
        winss::PipeServer<winss::NiceMockPipeInstance>::PipeServer(config) {}

    std::map<winss::HandleWrapper, winss::NiceMockPipeInstance>*
        GetInstances() {
        return &instances;
    }
};
class MockedOutboundPipeServer : public winss::OutboundPipeServerTmpl<
    winss::NiceMockOutboundPipeInstance> {
 public:
    explicit MockedOutboundPipeServer(const PipeServerConfig& config) :
        winss::OutboundPipeServerTmpl<winss::NiceMockOutboundPipeInstance>
        ::OutboundPipeServerTmpl(config) {}

    std::map<winss::HandleWrapper, winss::NiceMockOutboundPipeInstance>*
        GetInstances() {
        return &instances;
    }
};
class MockedInboundPipeServer : public winss::InboundPipeServerTmpl<
    winss::NiceMockInboundPipeInstance> {
 public:
    explicit MockedInboundPipeServer(const PipeServerConfig& config) :
        winss::InboundPipeServerTmpl<winss::NiceMockInboundPipeInstance>
        ::InboundPipeServerTmpl(config) {}

    std::map<winss::HandleWrapper, winss::NiceMockInboundPipeInstance>*
        GetInstances() {
        return &instances;
    }
};

TEST_F(PipeServerTest, Init) {
    MockInterface<winss::MockWindowsInterface> windows;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;

    EXPECT_CALL(multiplexer, AddInitCallback(_)).Times(1);
    EXPECT_CALL(multiplexer, AddStopCallback(_)).Times(1);

    winss::MockPipeName pipe_name("test");
    MockedPipeServer server({
        pipe_name, winss::NotOwned(&multiplexer)
    });

    EXPECT_FALSE(server.IsAccepting());

    multiplexer.mock_init_callbacks.at(0)(multiplexer);

    EXPECT_TRUE(server.IsAccepting());
}

TEST_F(PipeServerTest, Triggered) {
    MockInterface<winss::MockWindowsInterface> windows;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;

    winss::MockPipeName pipe_name("test");
    MockedPipeServer server({
        pipe_name, winss::NotOwned(&multiplexer)
    });

    multiplexer.mock_init_callbacks.at(0)(multiplexer);

    winss::NiceMockPipeInstance* instance =
        &server.GetInstances()->begin()->second;
    auto handle = instance->GetHandle();

    EXPECT_CALL(*instance, GetOverlappedResult()).WillOnce(Return(CONTINUE));
    EXPECT_CALL(*instance, SetConnected()).WillOnce(Return(true));

    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle);

    EXPECT_TRUE(server.IsAccepting());
    EXPECT_EQ(2, server.InstanceCount());
}

TEST_F(PipeServerTest, TriggeredDisconnectedOpen) {
    MockInterface<winss::MockWindowsInterface> windows;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;

    winss::MockPipeName pipe_name("test");
    MockedPipeServer server({
        pipe_name, winss::NotOwned(&multiplexer)
    });

    multiplexer.mock_init_callbacks.at(0)(multiplexer);

    winss::MockPipeInstance* instance =
        &server.GetInstances()->begin()->second;
    auto handle = instance->GetHandle();

    EXPECT_CALL(*instance, GetOverlappedResult())
        .WillOnce(Return(CONTINUE))
        .WillOnce(Return(REMOVE));
    EXPECT_CALL(*instance, SetConnected()).WillOnce(Return(true));
    EXPECT_CALL(*instance, IsConnected()).WillOnce(Return(true));

    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle);
    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle);

    EXPECT_TRUE(server.IsAccepting());
    EXPECT_EQ(1, server.InstanceCount());
}

TEST_F(PipeServerTest, TriggeredDisconnectedNotOpen) {
    MockInterface<winss::MockWindowsInterface> windows;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;

    winss::MockPipeName pipe_name("test");
    MockedPipeServer server({
        pipe_name, winss::NotOwned(&multiplexer)
    });

    multiplexer.mock_init_callbacks.at(0)(multiplexer);

    winss::MockPipeInstance* instance =
        &server.GetInstances()->begin()->second;
    auto handle = instance->GetHandle();

    EXPECT_CALL(*instance, GetOverlappedResult()).WillOnce(Return(REMOVE));

    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle);

    EXPECT_TRUE(server.IsAccepting());
    EXPECT_EQ(1, server.InstanceCount());
}

TEST_F(PipeServerTest, TriggeredDisconnectedSkip) {
    MockInterface<winss::MockWindowsInterface> windows;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;

    winss::MockPipeName pipe_name("test");
    MockedPipeServer server({
        pipe_name, winss::NotOwned(&multiplexer)
    });

    multiplexer.mock_init_callbacks.at(0)(multiplexer);

    winss::MockPipeInstance* instance =
        &server.GetInstances()->begin()->second;
    auto handle = instance->GetHandle();

    EXPECT_CALL(*instance, GetOverlappedResult()).WillOnce(Return(SKIP));

    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle);

    EXPECT_TRUE(server.IsAccepting());
    EXPECT_EQ(1, server.InstanceCount());
}

TEST_F(PipeServerTest, Stop) {
    MockInterface<winss::MockWindowsInterface> windows;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;

    winss::MockPipeName pipe_name("test");
    MockedPipeServer server({
        pipe_name, winss::NotOwned(&multiplexer)
    });

    multiplexer.mock_init_callbacks.at(0)(multiplexer);

    winss::MockPipeInstance* instance =
        &server.GetInstances()->begin()->second;
    auto handle = instance->GetHandle();

    EXPECT_CALL(*instance, GetOverlappedResult())
        .WillOnce(Return(CONTINUE))
        .WillOnce(Return(REMOVE));
    EXPECT_CALL(*instance, SetConnected()).WillOnce(Return(true));

    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle);

    EXPECT_TRUE(server.IsAccepting());
    EXPECT_EQ(2, server.InstanceCount());

    EXPECT_CALL(*instance, Closing()).Times(0);
    EXPECT_CALL(*instance, IsConnected())
        .WillOnce(Return(true))
        .WillOnce(Return(true));

    winss::MockPipeInstance* other = GetOther(server.GetInstances(), handle);
    ASSERT_NE(nullptr, other);

    EXPECT_CALL(*other, Closing()).Times(1);
    EXPECT_CALL(*other, IsConnected())
        .WillOnce(Return(false))
        .WillOnce(Return(false));
    EXPECT_CALL(*other, GetOverlappedResult())
        .WillOnce(Return(REMOVE));

    multiplexer.mock_stop_callbacks.at(0)(multiplexer);
    multiplexer.mock_stop_callbacks.at(0)(multiplexer);

    EXPECT_EQ(2, server.InstanceCount());
    EXPECT_TRUE(server.IsStopping());

    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle);
    multiplexer.mock_triggered_callbacks.at(0)(multiplexer,
        other->GetHandle());
}

TEST_F(PipeServerTest, OutboundConnected) {
    MockInterface<winss::MockWindowsInterface> windows;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;

    winss::MockPipeName pipe_name("test");
    MockedOutboundPipeServer server({
        pipe_name, winss::NotOwned(&multiplexer)
    });

    multiplexer.mock_init_callbacks.at(0)(multiplexer);

    winss::MockOutboundPipeInstance* instance =
        &server.GetInstances()->begin()->second;
    auto handle = instance->GetHandle();

    EXPECT_CALL(*instance, GetOverlappedResult()).WillOnce(Return(CONTINUE));
    EXPECT_CALL(*instance, SetConnected()).WillOnce(Return(true));
    EXPECT_CALL(*instance, Queue(_)).WillOnce(Return(true));
    EXPECT_CALL(*instance, Write()).WillOnce(Return(true));

    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle);
}

TEST_F(PipeServerTest, OutboundWrittenAll) {
    MockInterface<winss::MockWindowsInterface> windows;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;

    winss::MockPipeName pipe_name("test");
    MockedOutboundPipeServer server({
        pipe_name, winss::NotOwned(&multiplexer)
    });

    multiplexer.mock_init_callbacks.at(0)(multiplexer);

    winss::MockOutboundPipeInstance* instance =
        &server.GetInstances()->begin()->second;
    auto handle1 = instance->GetHandle();

    EXPECT_CALL(*instance, GetOverlappedResult())
        .WillRepeatedly(Return(CONTINUE));
    EXPECT_CALL(*instance, SetConnected())
        .WillOnce(Return(true))
        .WillOnce(Return(false))
        .WillOnce(Return(false));

    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle1);

    winss::MockOutboundPipeInstance* other =
        GetOther(server.GetInstances(), handle1);
    ASSERT_NE(nullptr, other);

    EXPECT_CALL(*other, GetOverlappedResult())
        .WillRepeatedly(Return(CONTINUE));
    EXPECT_CALL(*other, SetConnected())
        .WillOnce(Return(true))
        .WillOnce(Return(false))
        .WillOnce(Return(false));

    auto handle2 = other->GetHandle();
    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle2);

    EXPECT_CALL(*instance, Queue(_)).Times(1).WillRepeatedly(Return(true));
    EXPECT_CALL(*instance, Write()).Times(2).WillRepeatedly(Return(true));

    EXPECT_CALL(*instance, FinishWrite())
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    EXPECT_CALL(*other, Queue(_)).Times(1).WillRepeatedly(Return(true));
    EXPECT_CALL(*other, Write()).Times(2).WillRepeatedly(Return(true));

    EXPECT_CALL(*other, FinishWrite())
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    server.Send({ '1', '2' });

    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle1);
    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle2);

    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle1);
    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle2);
}

TEST_F(PipeServerTest, InboundRead) {
    MockInterface<winss::MockWindowsInterface> windows;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;
    NiceMock<winss::MockPipeServerRecieveListener> listener;

    EXPECT_CALL(listener, Recieved(_))
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    winss::MockPipeName pipe_name("test");
    MockedInboundPipeServer server({
        pipe_name, winss::NotOwned(&multiplexer)
    });

    server.AddListener(winss::NotOwned(&listener));
    multiplexer.mock_init_callbacks.at(0)(multiplexer);

    winss::MockInboundPipeInstance* instance =
        &server.GetInstances()->begin()->second;
    auto handle1 = instance->GetHandle();

    EXPECT_CALL(*instance, GetOverlappedResult())
        .WillRepeatedly(Return(CONTINUE));
    EXPECT_CALL(*instance, SetConnected())
        .WillOnce(Return(true))
        .WillOnce(Return(false))
        .WillOnce(Return(false));

    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle1);

    winss::MockInboundPipeInstance* other =
        GetOther(server.GetInstances(), handle1);
    ASSERT_NE(nullptr, other);

    EXPECT_CALL(*other, GetOverlappedResult())
        .WillRepeatedly(Return(CONTINUE));
    EXPECT_CALL(*other, SetConnected())
        .WillOnce(Return(true))
        .WillOnce(Return(false))
        .WillOnce(Return(false));

    auto handle2 = other->GetHandle();
    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle2);

    EXPECT_CALL(*instance, FinishRead())
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    EXPECT_CALL(*instance, SwapBuffer())
        .WillOnce(Return(std::vector<char>{'1'}));

    EXPECT_CALL(*other, FinishRead())
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    EXPECT_CALL(*other, SwapBuffer())
        .WillOnce(Return(std::vector<char>{'1'}));

    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle1);
    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle2);

    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle1);
    multiplexer.mock_triggered_callbacks.at(0)(multiplexer, handle2);
}
}  // namespace winss
