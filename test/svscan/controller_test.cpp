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
#include "winss/svscan/controller.hpp"
#include "winss/not_owning_ptr.hpp"
#include "../mock_filesystem_interface.hpp"
#include "../mock_interface.hpp"
#include "../mock_wait_multiplexer.hpp"
#include "../mock_pipe_server.hpp"
#include "../mock_pipe_name.hpp"
#include "mock_svscan.hpp"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::InSequence;
using ::testing::Return;

namespace winss {
class SvScanControllerTest : public testing::Test {
};

TEST_F(SvScanControllerTest, Received) {
    MockInterface<winss::MockFilesystemInterface> file;
    NiceMock<winss::MockWaitMultiplexer> multiplexer;

    EXPECT_CALL(*file, CanonicalUncPath(_))
        .WillRepeatedly(Return(fs::path(".")));

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

    EXPECT_TRUE(controller.Received({
        winss::SvScanController::kAlarm,
        winss::SvScanController::kAbort,
        winss::SvScanController::kNuke,
        winss::SvScanController::kQuit,
        0
    }));
}
}  // namespace winss
