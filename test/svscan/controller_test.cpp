#include <winss/winss.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <winss/svscan/controller.hpp>
#include <winss/not_owning_ptr.hpp>
#include "../mock_wait_multiplexer.hpp"
#include "../mock_pipe_server.hpp"
#include "../mock_pipe_name.hpp"
#include "mock_svscan.hpp"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::InSequence;

namespace winss {
class SvScanControllerTest : public testing::Test {
};

TEST_F(SvScanControllerTest, Recieved) {
    NiceMock<winss::MockWaitMultiplexer> multiplexer;
    NiceMock<winss::MockSvScan> svscan(winss::NotOwned(&multiplexer), ".", 0);
    NiceMock<winss::MockInboundPipeServer> inbound(winss::PipeServerConfig{
        winss::MockPipeName("test"),
        winss::NotOwned(&multiplexer)
    });

    InSequence dummy;
    EXPECT_CALL(svscan, Scan(false)).Times(1);
    EXPECT_CALL(svscan, Exit(false)).Times(1);
    EXPECT_CALL(svscan, CloseAllServices(false)).Times(1);
    EXPECT_CALL(svscan, Exit(true)).Times(1);

    winss::SvScanController controller(winss::NotOwned(&svscan),
        winss::NotOwned(&inbound));

    EXPECT_TRUE(controller.Recieved({
        winss::SvScanController::kAlarm,
        winss::SvScanController::kAbort,
        winss::SvScanController::kNuke,
        winss::SvScanController::kQuit,
        0
    }));
}
}  // namespace winss
