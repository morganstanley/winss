#include <vector>
#include <utility>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "winss/winss.hpp"
#include "winss/pipe_instance.hpp"
#include "mock_interface.hpp"
#include "mock_windows_interface.hpp"
#include "mock_pipe_name.hpp"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SetArgumentPointee;
using ::testing::DoAll;

namespace winss {
class PipeInstanceTest : public testing::Test {
};

TEST_F(PipeInstanceTest, InitialState) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateEvent(_, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(9000)));

    winss::PipeInstance instance;

    EXPECT_FALSE(instance.IsPendingIO());
    EXPECT_FALSE(instance.IsConnected());
    EXPECT_TRUE(instance.GetHandle().HasHandle());
}

TEST_F(PipeInstanceTest, CreateNamedPipe) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateEvent(_, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(9000)));

    EXPECT_CALL(*windows, CreateNamedPipe(_, _, _, _, _, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(10000)));

    EXPECT_CALL(*windows, ConnectNamedPipe(_, _))
        .WillOnce(Return(false));

    EXPECT_CALL(*windows, GetLastError())
        .WillOnce(Return(ERROR_IO_PENDING));

    winss::PipeInstance instance;
    winss::MockPipeName pipe_name("test");

    EXPECT_TRUE(instance.CreateNamedPipe(pipe_name));
    EXPECT_TRUE(instance.IsPendingIO());
    EXPECT_FALSE(instance.IsConnected());
    EXPECT_TRUE(instance.GetHandle().HasHandle());
}

TEST_F(PipeInstanceTest, CreateNamedPipeFailed) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateEvent(_, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(9000)));

    EXPECT_CALL(*windows, CreateNamedPipe(_, _, _, _, _, _, _, _))
        .WillOnce(Return(INVALID_HANDLE_VALUE));

    winss::PipeInstance instance;
    winss::MockPipeName pipe_name("test");

    EXPECT_FALSE(instance.CreateNamedPipe(pipe_name));
    EXPECT_FALSE(instance.IsPendingIO());
    EXPECT_FALSE(instance.IsConnected());
    EXPECT_TRUE(instance.GetHandle().HasHandle());
}

TEST_F(PipeInstanceTest, CreateNamedPipeConnectFailed) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateEvent(_, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(9000)));

    EXPECT_CALL(*windows, CreateNamedPipe(_, _, _, _, _, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(10000)));

    EXPECT_CALL(*windows, ConnectNamedPipe(_, _))
        .WillOnce(Return(true));

    winss::PipeInstance instance;
    winss::MockPipeName pipe_name("test");

    EXPECT_FALSE(instance.CreateNamedPipe(pipe_name));
    EXPECT_FALSE(instance.IsPendingIO());
    EXPECT_FALSE(instance.IsConnected());
    EXPECT_TRUE(instance.GetHandle().HasHandle());
}

TEST_F(PipeInstanceTest, CreateNamedPipeSynchronous) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateEvent(_, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(9000)));

    EXPECT_CALL(*windows, CreateNamedPipe(_, _, _, _, _, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(10000)));

    EXPECT_CALL(*windows, ConnectNamedPipe(_, _))
        .WillOnce(Return(false));

    EXPECT_CALL(*windows, GetLastError())
        .WillOnce(Return(ERROR_PIPE_CONNECTED));

    EXPECT_CALL(*windows, SetEvent(_)).WillOnce(Return(true));

    winss::PipeInstance instance;
    winss::MockPipeName pipe_name("test");

    EXPECT_TRUE(instance.CreateNamedPipe(pipe_name));
    EXPECT_FALSE(instance.IsPendingIO());
    EXPECT_FALSE(instance.IsConnected());
    EXPECT_TRUE(instance.GetHandle().HasHandle());
}

TEST_F(PipeInstanceTest, CreateFile) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateEvent(_, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(9000)));

    EXPECT_CALL(*windows, CreateFile(_, _, _, _, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(10000)));

    winss::PipeInstance instance;
    winss::MockPipeName pipe_name("test");

    EXPECT_TRUE(instance.CreateFile(pipe_name));
    EXPECT_FALSE(instance.IsPendingIO());
    EXPECT_FALSE(instance.IsConnected());
    EXPECT_TRUE(instance.GetHandle().HasHandle());
}

TEST_F(PipeInstanceTest, Connected) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateEvent(_, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(9000)));

    EXPECT_CALL(*windows, CreateFile(_, _, _, _, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(10000)));

    winss::PipeInstance instance;
    winss::MockPipeName pipe_name("test");

    EXPECT_TRUE(instance.CreateFile(pipe_name));
    EXPECT_FALSE(instance.IsConnected());
    EXPECT_TRUE(instance.SetConnected());
    EXPECT_TRUE(instance.IsConnected());
    EXPECT_FALSE(instance.SetConnected());
    EXPECT_TRUE(instance.IsConnected());
}

TEST_F(PipeInstanceTest, GetOverlappedResult) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateNamedPipe(_, _, _, _, _, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(10000)));

    EXPECT_CALL(*windows, ConnectNamedPipe(_, _))
        .WillOnce(Return(false));

    EXPECT_CALL(*windows, GetLastError())
        .WillRepeatedly(Return(ERROR_IO_PENDING));

    EXPECT_CALL(*windows, GetOverlappedResult(_, _, _, _))
        .WillOnce(Return(true));

    winss::PipeInstance instance;
    winss::MockPipeName pipe_name("test");

    EXPECT_TRUE(instance.CreateNamedPipe(pipe_name));
    EXPECT_EQ(CONTINUE, instance.GetOverlappedResult());
}

TEST_F(PipeInstanceTest, GetOverlappedResultNotCreated) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, GetOverlappedResult(_, _, _, _))
        .Times(0);

    winss::PipeInstance instance;

    EXPECT_EQ(REMOVE, instance.GetOverlappedResult());
}

TEST_F(PipeInstanceTest, GetOverlappedResultIncomplete) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateNamedPipe(_, _, _, _, _, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(10000)));

    EXPECT_CALL(*windows, ConnectNamedPipe(_, _))
        .WillOnce(Return(false));

    EXPECT_CALL(*windows, GetLastError())
        .WillOnce(Return(ERROR_IO_PENDING))
        .WillOnce(Return(ERROR_IO_INCOMPLETE));

    EXPECT_CALL(*windows, GetOverlappedResult(_, _, _, _))
        .WillOnce(Return(false));

    winss::PipeInstance instance;
    winss::MockPipeName pipe_name("test");

    EXPECT_TRUE(instance.CreateNamedPipe(pipe_name));
    EXPECT_EQ(SKIP, instance.GetOverlappedResult());
}

TEST_F(PipeInstanceTest, GetOverlappedResultClosed) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateNamedPipe(_, _, _, _, _, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(10000)));

    EXPECT_CALL(*windows, ConnectNamedPipe(_, _))
        .WillOnce(Return(false));

    EXPECT_CALL(*windows, GetLastError())
        .WillOnce(Return(ERROR_IO_PENDING))
        .WillOnce(Return(ERROR_BROKEN_PIPE));

    EXPECT_CALL(*windows, GetOverlappedResult(_, _, _, _))
        .WillOnce(Return(false));

    winss::PipeInstance instance;
    winss::MockPipeName pipe_name("test");

    EXPECT_TRUE(instance.CreateNamedPipe(pipe_name));
    EXPECT_TRUE(instance.SetConnected());
    EXPECT_EQ(REMOVE, instance.GetOverlappedResult());
}

TEST_F(PipeInstanceTest, GetOverlappedResultClosing) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateEvent(_, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(9000)));

    EXPECT_CALL(*windows, CreateFile(_, _, _, _, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(10000)));

    EXPECT_CALL(*windows, GetOverlappedResult(_, _, _, _))
        .Times(0);

    EXPECT_CALL(*windows, ResetEvent(_))
        .Times(1);

    winss::PipeInstance instance;
    winss::MockPipeName pipe_name("test");

    EXPECT_TRUE(instance.CreateFile(pipe_name));
    EXPECT_TRUE(instance.SetConnected());
    instance.Closing();
    EXPECT_EQ(REMOVE, instance.GetOverlappedResult());
}

TEST_F(PipeInstanceTest, GetOverlappedResultNotPendingIO) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateEvent(_, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(9000)));

    EXPECT_CALL(*windows, CreateFile(_, _, _, _, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(10000)));

    EXPECT_CALL(*windows, GetOverlappedResult(_, _, _, _))
        .Times(0);

    EXPECT_CALL(*windows, ResetEvent(_))
        .Times(1);

    winss::PipeInstance instance;
    winss::MockPipeName pipe_name("test");

    EXPECT_TRUE(instance.CreateFile(pipe_name));
    EXPECT_TRUE(instance.SetConnected());
    EXPECT_EQ(CONTINUE, instance.GetOverlappedResult());
}

TEST_F(PipeInstanceTest, Closing) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateEvent(_, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(9000)));

    EXPECT_CALL(*windows, CreateFile(_, _, _, _, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(10000)));

    EXPECT_CALL(*windows, SetEvent(_)).Times(1);

    winss::PipeInstance instance;
    winss::MockPipeName pipe_name("test");

    EXPECT_TRUE(instance.CreateFile(pipe_name));
    EXPECT_TRUE(instance.SetConnected());
    instance.Closing();
    instance.Closing();
    EXPECT_TRUE(instance.IsClosing());
}

TEST_F(PipeInstanceTest, ClosingNotCreated) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, SetEvent(_)).Times(0);

    winss::PipeInstance instance;

    instance.Closing();
    EXPECT_FALSE(instance.IsClosing());
}

TEST_F(PipeInstanceTest, DisconnectNamedPipe) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateEvent(_, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(9000)));

    EXPECT_CALL(*windows, CreateFile(_, _, _, _, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(10000)));

    EXPECT_CALL(*windows, DisconnectNamedPipe(_)).Times(1);

    winss::PipeInstance instance;
    winss::MockPipeName pipe_name("test");

    EXPECT_TRUE(instance.CreateFile(pipe_name));
    EXPECT_TRUE(instance.SetConnected());
    EXPECT_TRUE(instance.IsConnected());
    instance.DisconnectNamedPipe();
    EXPECT_FALSE(instance.IsConnected());
}

TEST_F(PipeInstanceTest, DisconnectNamedPipeNotCreated) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, DisconnectNamedPipe(_)).Times(0);

    winss::PipeInstance instance;

    instance.DisconnectNamedPipe();
}

TEST_F(PipeInstanceTest, Close) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateEvent(_, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(9000)));

    EXPECT_CALL(*windows, CreateFile(_, _, _, _, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(10000)));

    EXPECT_CALL(*windows, CloseHandle(_)).Times(2);

    winss::PipeInstance instance;
    winss::MockPipeName pipe_name("test");

    EXPECT_TRUE(instance.CreateFile(pipe_name));
    EXPECT_TRUE(instance.SetConnected());
    instance.Close();
    EXPECT_FALSE(instance.IsConnected());
}

TEST_F(PipeInstanceTest, Move) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateEvent(_, _, _, _))
        .Times(6)
        .WillRepeatedly(Return(reinterpret_cast<HANDLE>(9000)));

    EXPECT_CALL(*windows, CreateFile(_, _, _, _, _, _, _))
        .WillRepeatedly(Return(reinterpret_cast<HANDLE>(10000)));

    winss::PipeInstance instance1;
    winss::MockPipeName pipe_name("test");

    EXPECT_TRUE(instance1.CreateFile(pipe_name));
    EXPECT_TRUE(instance1.GetHandle().HasHandle());
    EXPECT_TRUE(instance1.SetConnected());

    winss::PipeInstance instance2 = std::move(instance1);
    EXPECT_FALSE(instance1.IsConnected());
    EXPECT_FALSE(instance1.GetHandle().HasHandle());
    EXPECT_TRUE(instance2.IsConnected());
    EXPECT_TRUE(instance2.GetHandle().HasHandle());

    winss::PipeInstance instance3;
    instance3.SetConnected();
    instance3 = std::move(instance2);
    EXPECT_FALSE(instance2.IsConnected());
    EXPECT_FALSE(instance2.GetHandle().HasHandle());
    EXPECT_TRUE(instance3.IsConnected());
    EXPECT_TRUE(instance3.GetHandle().HasHandle());

    winss::OutboundPipeInstance outbound1;
    outbound1.CreateFile(pipe_name);
    outbound1.SetConnected();
    outbound1.Queue({ '1' });
    EXPECT_TRUE(outbound1.HasMessages());
    EXPECT_TRUE(outbound1.IsConnected());
    winss::OutboundPipeInstance outbound2(std::move(outbound1));
    EXPECT_FALSE(outbound1.HasMessages());
    EXPECT_FALSE(outbound1.IsConnected());
    EXPECT_TRUE(outbound2.HasMessages());
    EXPECT_TRUE(outbound2.IsConnected());
    winss::OutboundPipeInstance outbound3;
    outbound3.CreateFile(pipe_name);
    outbound3.SetConnected();
    outbound3 = std::move(outbound2);
    EXPECT_FALSE(outbound2.HasMessages());
    EXPECT_FALSE(outbound2.IsConnected());
    EXPECT_TRUE(outbound3.HasMessages());
    EXPECT_TRUE(outbound3.IsConnected());

    winss::InboundPipeInstance inbound1;
    inbound1.CreateFile(pipe_name);
    inbound1.SetConnected();
    winss::InboundPipeInstance inbound2(std::move(inbound1));
    winss::InboundPipeInstance inbound3;
    inbound3.CreateFile(pipe_name);
    inbound3.SetConnected();
    inbound3 = std::move(inbound2);
    EXPECT_TRUE(inbound3.IsConnected());
}

TEST_F(PipeInstanceTest, OutboundQueueEmpty) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateEvent(_, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(9000)));

    EXPECT_CALL(*windows, CreateFile(_, _, _, _, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(10000)));

    winss::OutboundPipeInstance outbound;
    winss::MockPipeName pipe_name("test");

    EXPECT_TRUE(outbound.CreateFile(pipe_name));
    outbound.SetConnected();
    EXPECT_FALSE(outbound.Queue({}));
}

TEST_F(PipeInstanceTest, OutboundQueueNotConnected) {
    winss::OutboundPipeInstance outbound;

    EXPECT_FALSE(outbound.Queue({'0'}));
}

TEST_F(PipeInstanceTest, OutboundQueueSmallPayload) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateEvent(_, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(9000)));

    EXPECT_CALL(*windows, CreateFile(_, _, _, _, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(10000)));

    EXPECT_CALL(*windows, WriteFile(_, _, 3, _, _))
        .WillOnce(Return(true));

    winss::OutboundPipeInstance outbound;
    winss::MockPipeName pipe_name("test");

    EXPECT_TRUE(outbound.CreateFile(pipe_name));
    EXPECT_TRUE(outbound.SetConnected());
    EXPECT_TRUE(outbound.Queue({'1', '2', '3'}));
    EXPECT_TRUE(outbound.HasMessages());
    EXPECT_TRUE(outbound.Write());
    EXPECT_TRUE(outbound.IsWriting());
    EXPECT_FALSE(outbound.FinishWrite());
}

TEST_F(PipeInstanceTest, OutboundQueueLargePayload) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateEvent(_, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(9000)));

    EXPECT_CALL(*windows, CreateFile(_, _, _, _, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(10000)));

    EXPECT_CALL(*windows, WriteFile(_, _, 4096, _, _))
        .WillOnce(Return(true))
        .WillOnce(Return(true));

    EXPECT_CALL(*windows, WriteFile(_, _, 1000, _, _))
        .WillOnce(Return(true));

    DWORD number = 4096 * 2 + 1000;
    std::vector<char> to_send(number);
    std::fill(to_send.begin(), to_send.end(), 'a');

    winss::OutboundPipeInstance outbound;
    winss::MockPipeName pipe_name("test");

    EXPECT_TRUE(outbound.CreateFile(pipe_name));
    EXPECT_TRUE(outbound.SetConnected());
    EXPECT_FALSE(outbound.HasMessages());
    EXPECT_FALSE(outbound.FinishWrite());
    EXPECT_FALSE(outbound.Write());
    EXPECT_TRUE(outbound.Queue(to_send));
    EXPECT_TRUE(outbound.HasMessages());
    EXPECT_TRUE(outbound.Write());
    EXPECT_TRUE(outbound.IsWriting());
    EXPECT_TRUE(outbound.FinishWrite());
    EXPECT_TRUE(outbound.HasMessages());
    EXPECT_TRUE(outbound.Write());
    EXPECT_TRUE(outbound.IsWriting());
    EXPECT_TRUE(outbound.FinishWrite());
    EXPECT_TRUE(outbound.HasMessages());
    EXPECT_TRUE(outbound.Write());
    EXPECT_TRUE(outbound.IsWriting());
    EXPECT_FALSE(outbound.FinishWrite());
}

TEST_F(PipeInstanceTest, OutboundQueuePending) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateEvent(_, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(9000)));

    EXPECT_CALL(*windows, CreateFile(_, _, _, _, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(10000)));

    EXPECT_CALL(*windows, WriteFile(_, _, 3, _, _))
        .WillOnce(Return(false));

    EXPECT_CALL(*windows, GetLastError())
        .WillOnce(Return(ERROR_IO_PENDING));

    winss::OutboundPipeInstance outbound;
    winss::MockPipeName pipe_name("test");

    EXPECT_TRUE(outbound.CreateFile(pipe_name));
    EXPECT_TRUE(outbound.SetConnected());
    EXPECT_TRUE(outbound.Queue({ '1', '2', '3' }));
    EXPECT_TRUE(outbound.Write());
    EXPECT_TRUE(outbound.IsWriting());
    EXPECT_TRUE(outbound.IsPendingIO());
}

TEST_F(PipeInstanceTest, OutboundQueueDisconnected) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateEvent(_, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(9000)));

    EXPECT_CALL(*windows, CreateFile(_, _, _, _, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(10000)));

    EXPECT_CALL(*windows, WriteFile(_, _, 3, _, _))
        .WillOnce(Return(false));

    EXPECT_CALL(*windows, GetLastError())
        .WillOnce(Return(ERROR_BROKEN_PIPE));

    winss::OutboundPipeInstance outbound;
    winss::MockPipeName pipe_name("test");

    EXPECT_TRUE(outbound.CreateFile(pipe_name));
    EXPECT_TRUE(outbound.SetConnected());
    EXPECT_TRUE(outbound.Queue({ '1', '2', '3' }));
    EXPECT_FALSE(outbound.Write());
    EXPECT_TRUE(outbound.IsClosing());
}

TEST_F(PipeInstanceTest, OutboundQueueError) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateEvent(_, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(9000)));

    EXPECT_CALL(*windows, CreateFile(_, _, _, _, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(10000)));

    EXPECT_CALL(*windows, WriteFile(_, _, 3, _, _))
        .WillOnce(Return(false));

    EXPECT_CALL(*windows, GetLastError())
        .WillOnce(Return(ERROR_CANCELLED));

    winss::OutboundPipeInstance outbound;
    winss::MockPipeName pipe_name("test");

    EXPECT_TRUE(outbound.CreateFile(pipe_name));
    EXPECT_TRUE(outbound.SetConnected());
    EXPECT_TRUE(outbound.Queue({ '1', '2', '3' }));
    EXPECT_FALSE(outbound.Write());
    EXPECT_TRUE(outbound.IsClosing());
}

TEST_F(PipeInstanceTest, OutboundRead) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateEvent(_, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(9000)));

    EXPECT_CALL(*windows, CreateFile(_, _, _, _, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(10000)));

    EXPECT_CALL(*windows, ReadFile(_, _, 0, _, _))
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    EXPECT_CALL(*windows, GetLastError())
        .WillOnce(Return(ERROR_IO_PENDING));

    EXPECT_CALL(*windows, SetEvent(_)).Times(1);

    winss::OutboundPipeInstance outbound;
    winss::MockPipeName pipe_name("test");

    EXPECT_TRUE(outbound.CreateFile(pipe_name));
    EXPECT_TRUE(outbound.SetConnected());
    outbound.Read();
    outbound.Read();
}

TEST_F(PipeInstanceTest, OutboundReadClosing) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateEvent(_, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(9000)));

    EXPECT_CALL(*windows, CreateFile(_, _, _, _, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(10000)));

    EXPECT_CALL(*windows, ReadFile(_, _, 0, _, _))
        .Times(0);

    winss::OutboundPipeInstance outbound;
    winss::MockPipeName pipe_name("test");

    EXPECT_TRUE(outbound.CreateFile(pipe_name));
    EXPECT_TRUE(outbound.SetConnected());
    outbound.Closing();
    outbound.Read();
}

TEST_F(PipeInstanceTest, OutboundReadNotConnected) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, ReadFile(_, _, 0, _, _))
        .Times(0);

    winss::OutboundPipeInstance outbound;

    outbound.Read();
}

TEST_F(PipeInstanceTest, OutboundReadDisconnected) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateEvent(_, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(9000)));

    EXPECT_CALL(*windows, CreateFile(_, _, _, _, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(10000)));

    EXPECT_CALL(*windows, GetLastError())
        .WillOnce(Return(ERROR_BROKEN_PIPE));

    winss::OutboundPipeInstance outbound;
    winss::MockPipeName pipe_name("test");

    EXPECT_TRUE(outbound.CreateFile(pipe_name));
    EXPECT_TRUE(outbound.SetConnected());
    outbound.Read();
}

TEST_F(PipeInstanceTest, OutboundReadError) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateEvent(_, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(9000)));

    EXPECT_CALL(*windows, CreateFile(_, _, _, _, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(10000)));

    EXPECT_CALL(*windows, GetLastError())
        .WillOnce(Return(ERROR_CANCELLED));

    winss::OutboundPipeInstance outbound;
    winss::MockPipeName pipe_name("test");

    EXPECT_TRUE(outbound.CreateFile(pipe_name));
    EXPECT_TRUE(outbound.SetConnected());
    outbound.Read();
}

TEST_F(PipeInstanceTest, InboundRead) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateEvent(_, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(9000)));

    EXPECT_CALL(*windows, CreateFile(_, _, _, _, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(10000)));

    EXPECT_CALL(*windows, ReadFile(_, _,
        winss::InboundPipeInstance::kBufferSize, _, _))
        .WillOnce(DoAll(SetArgumentPointee<3>(5), Return(true)))
        .WillOnce(DoAll(SetArgumentPointee<3>(6), Return(true)))
        .WillOnce(DoAll(SetArgumentPointee<3>(0), Return(true)));

    EXPECT_CALL(*windows, SetEvent(_)).Times(3);

    winss::InboundPipeInstance inbound;
    winss::MockPipeName pipe_name("test");

    EXPECT_TRUE(inbound.CreateFile(pipe_name));
    EXPECT_TRUE(inbound.SetConnected());
    EXPECT_FALSE(inbound.FinishRead());
    inbound.Read();
    EXPECT_TRUE(inbound.FinishRead());
    inbound.Read();
    EXPECT_TRUE(inbound.FinishRead());
    inbound.Read();
    EXPECT_FALSE(inbound.FinishRead());
}

TEST_F(PipeInstanceTest, InboundReadClosing) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateEvent(_, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(9000)));

    EXPECT_CALL(*windows, CreateFile(_, _, _, _, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(10000)));

    EXPECT_CALL(*windows, ReadFile(_, _,
        winss::InboundPipeInstance::kBufferSize, _, _)).Times(0);

    winss::InboundPipeInstance inbound;
    winss::MockPipeName pipe_name("test");

    EXPECT_TRUE(inbound.CreateFile(pipe_name));
    EXPECT_TRUE(inbound.SetConnected());
    inbound.Closing();
    inbound.Read();
}

TEST_F(PipeInstanceTest, InboundReadNotConnected) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, ReadFile(_, _,
        winss::InboundPipeInstance::kBufferSize, _, _)).Times(0);

    winss::InboundPipeInstance inbound;

    inbound.Read();
    EXPECT_FALSE(inbound.FinishRead());
}

TEST_F(PipeInstanceTest, InboundSwapBuffer) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CreateEvent(_, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(9000)));

    EXPECT_CALL(*windows, CreateFile(_, _, _, _, _, _, _))
        .WillOnce(Return(reinterpret_cast<HANDLE>(10000)));

    EXPECT_CALL(*windows, ReadFile(_, _,
        winss::InboundPipeInstance::kBufferSize, _, _))
        .WillOnce(DoAll(SetArgumentPointee<3>(5), Return(true)));

    EXPECT_CALL(*windows, SetEvent(_)).Times(1);

    winss::InboundPipeInstance inbound;
    winss::MockPipeName pipe_name("test");

    EXPECT_TRUE(inbound.CreateFile(pipe_name));
    EXPECT_TRUE(inbound.SetConnected());
    inbound.Read();
    EXPECT_TRUE(inbound.FinishRead());
    std::vector<char> buff = inbound.SwapBuffer();
    EXPECT_EQ(5, buff.size());
}
}  // namespace winss
