#ifndef TEST_MOCK_PIPE_SERVER_HPP_
#define TEST_MOCK_PIPE_SERVER_HPP_

#include <windows.h>
#include <gmock/gmock.h>
#include <winss/pipe_server.hpp>
#include <winss/handle_wrapper.hpp>
#include <vector>

using ::testing::_;

namespace winss {
class MockOutboundPipeServer : public winss::OutboundPipeServer {
 public:
    explicit MockOutboundPipeServer(const winss::PipeServerConfig& config) :
        winss::OutboundPipeServer::OutboundPipeServerTmpl(config) {}

    MockOutboundPipeServer(const MockOutboundPipeServer&) = delete;
    MockOutboundPipeServer(MockOutboundPipeServer&&) = delete;

    MOCK_CONST_METHOD0(IsAccepting, bool());
    MOCK_CONST_METHOD0(IsStopping, bool());
    MOCK_CONST_METHOD0(InstanceCount, size_t());
    MOCK_METHOD1(Send, bool(const std::vector<char>& data));

    void operator=(const MockOutboundPipeServer&) = delete;
    MockOutboundPipeServer& operator=(MockOutboundPipeServer&&) = delete;
};
class MockInboundPipeServer : public winss::InboundPipeServer {
 public:
    explicit MockInboundPipeServer(const winss::PipeServerConfig& config) :
        winss::InboundPipeServer::InboundPipeServerTmpl(config) {}

    MockInboundPipeServer(const MockInboundPipeServer&) = delete;
    MockInboundPipeServer(MockInboundPipeServer&&) = delete;

    MOCK_CONST_METHOD0(IsAccepting, bool());
    MOCK_CONST_METHOD0(IsStopping, bool());
    MOCK_CONST_METHOD0(InstanceCount, size_t());
    MOCK_METHOD1(AddListener, void(winss::PipeServerRecieveListener* listener));

    void operator=(const MockInboundPipeServer&) = delete;
    MockInboundPipeServer& operator=(MockInboundPipeServer&&) = delete;
};
class MockPipeServerRecieveListener : public winss::PipeServerRecieveListener {
 public:
    MOCK_METHOD1(Recieved, bool(const std::vector<char>& data));
};
}  // namespace winss

#endif  // TEST_MOCK_PIPE_SERVER_HPP_
