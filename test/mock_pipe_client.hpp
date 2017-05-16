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

#ifndef TEST_MOCK_PIPE_CLIENT_HPP_
#define TEST_MOCK_PIPE_CLIENT_HPP_

#include <vector>
#include "gmock/gmock.h"
#include "winss/pipe_client.hpp"

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
class MockPipeClientSendReceiveListener :
    public winss::PipeClientSendListener,
    public winss::PipeClientReceiveListener {
 public:
    MOCK_METHOD0(Connected, bool());
    MOCK_METHOD0(Disconnected, bool());
    MOCK_METHOD0(WriteComplete, bool());
    MOCK_METHOD1(Received, bool(const std::vector<char>& data));
};
}  // namespace winss

#endif  // TEST_MOCK_PIPE_CLIENT_HPP_
