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

TEST_F(SuperviseControllerTest, Received) {
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
    EXPECT_TRUE(controller.Received({ winss::SuperviseController::kSvcUp }));
    EXPECT_TRUE(controller.Received({ winss::SuperviseController::kSvcOnce }));
    EXPECT_TRUE(controller.Received(
    { winss::SuperviseController::kSvcOnceAtMost }));
    EXPECT_TRUE(controller.Received({ winss::SuperviseController::kSvcDown }));
    EXPECT_TRUE(controller.Received({ winss::SuperviseController::kSvcKill }));
    EXPECT_TRUE(controller.Received({ winss::SuperviseController::kSvcTerm }));
    EXPECT_TRUE(controller.Received({ winss::SuperviseController::kSvcExit }));
}
}  // namespace winss
