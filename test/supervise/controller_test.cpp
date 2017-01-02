#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "winss/winss.hpp"
#include "winss/supervise/controller.hpp"
#include "winss/supervise/supervise.hpp"
#include "winss/not_owning_ptr.hpp"
#include "../mock_pipe_server.hpp"
#include "../mock_pipe_name.hpp"
#include "../mock_wait_multiplexer.hpp"
#include "mock_supervise.hpp"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::InSequence;

namespace winss {
class SuperviseControllerTest : public testing::Test {
};

TEST_F(SuperviseControllerTest, Notify) {
    NiceMock<winss::MockWaitMultiplexer> multiplexer;
    NiceMock<winss::MockSupervise> supervise(winss::NotOwned(&multiplexer),
        "test");
    NiceMock<winss::MockOutboundPipeServer> outbound(winss::PipeServerConfig{
        winss::MockPipeName("inbound"),
        winss::NotOwned(&multiplexer)
    });
    NiceMock<winss::MockInboundPipeServer> inbound(winss::PipeServerConfig{
        winss::MockPipeName("inbound"),
        winss::NotOwned(&multiplexer)
    });

    winss::SuperviseController controller(winss::NotOwned(&supervise),
        winss::NotOwned(&outbound), winss::NotOwned(&inbound));

    EXPECT_CALL(outbound, Send(_)).Times(5);

    winss::SuperviseState state{};
    EXPECT_TRUE(controller.Notify(winss::SuperviseController::GetNotification(
        winss::SuperviseController::kSuperviseStart), state));
    EXPECT_TRUE(controller.Notify(winss::SuperviseController::GetNotification(
        winss::SuperviseController::kSuperviseRun), state));
    EXPECT_TRUE(controller.Notify(winss::SuperviseController::GetNotification(
        winss::SuperviseController::kSuperviseEnd), state));
    EXPECT_TRUE(controller.Notify(winss::SuperviseController::GetNotification(
        winss::SuperviseController::kSuperviseFinished), state));
    EXPECT_TRUE(controller.Notify(winss::SuperviseController::GetNotification(
        winss::SuperviseController::kSuperviseExit), state));
}

TEST_F(SuperviseControllerTest, Recieved) {
    NiceMock<winss::MockWaitMultiplexer> multiplexer;
    NiceMock<winss::MockSupervise> supervise(winss::NotOwned(&multiplexer),
        "test");
    NiceMock<winss::MockOutboundPipeServer> outbound(winss::PipeServerConfig{
        winss::MockPipeName("inbound"),
        winss::NotOwned(&multiplexer)
    });
    NiceMock<winss::MockInboundPipeServer> inbound(winss::PipeServerConfig{
        winss::MockPipeName("inbound"),
        winss::NotOwned(&multiplexer)
    });

    winss::SuperviseController controller(winss::NotOwned(&supervise),
        winss::NotOwned(&outbound), winss::NotOwned(&inbound));

    InSequence dummy;
    EXPECT_CALL(supervise, Up()).Times(1);
    EXPECT_CALL(supervise, Once()).Times(1);
    EXPECT_CALL(supervise, OnceAtMost()).Times(1);
    EXPECT_CALL(supervise, Down()).Times(1);
    EXPECT_CALL(supervise, Kill()).Times(1);
    EXPECT_CALL(supervise, Term()).Times(1);
    EXPECT_CALL(supervise, Exit()).Times(1);

    winss::SuperviseState state{};
    EXPECT_TRUE(controller.Recieved({ winss::SuperviseController::kSvcUp }));
    EXPECT_TRUE(controller.Recieved({ winss::SuperviseController::kSvcOnce }));
    EXPECT_TRUE(controller.Recieved(
    { winss::SuperviseController::kSvcOnceAtMost }));
    EXPECT_TRUE(controller.Recieved({ winss::SuperviseController::kSvcDown }));
    EXPECT_TRUE(controller.Recieved({ winss::SuperviseController::kSvcKill }));
    EXPECT_TRUE(controller.Recieved({ winss::SuperviseController::kSvcTerm }));
    EXPECT_TRUE(controller.Recieved({ winss::SuperviseController::kSvcExit }));
}
}  // namespace winss
