#ifndef TEST_MOCK_PIPE_CLIENT_HPP_
#define TEST_MOCK_PIPE_CLIENT_HPP_

#include <windows.h>
#include <gmock/gmock.h>
#include <winss/pipe_client.hpp>
#include <winss/handle_wrapper.hpp>
#include <vector>

using ::testing::_;

namespace winss {
class MockOutboundPipeClient : public winss::OutboundPipeClient {
 public:
    explicit MockOutboundPipeClient(const winss::PipeClientConfig& config) :
        winss::OutboundPipeClient::OutboundPipeClientTmpl(config) {}

    MOCK_METHOD1(AddListener, void(winss::OutboundPipeInstance* listener));
    MOCK_CONST_METHOD0(IsStopping, bool());
    MOCK_METHOD0(Connect, void());
    MOCK_METHOD0(Stop, void());
    MOCK_METHOD1(Send, bool(const std::vector<char>& data));
};
class MockInboundPipeClient : public winss::InboundPipeClient {
 public:
    explicit MockInboundPipeClient(const winss::PipeClientConfig& config) :
        winss::InboundPipeClient::InboundPipeClientTmpl(config) {}

    MOCK_METHOD1(AddListener, void(winss::OutboundPipeInstance* listener));
    MOCK_CONST_METHOD0(IsStopping, bool());
    MOCK_METHOD0(Connect, void());
    MOCK_METHOD0(Stop, void());
};
class MockPipeClientSendRecieveListener :
    public winss::PipeClientSendListener,
    public winss::PipeClientRecieveListener {
 public:
    MOCK_METHOD0(Connected, bool());
    MOCK_METHOD0(Disconnected, bool());
    MOCK_METHOD0(WriteComplete, bool());
    MOCK_METHOD1(Recieved, bool(const std::vector<char>& data));
};
}  // namespace winss

#endif  // TEST_MOCK_PIPE_CLIENT_HPP_
