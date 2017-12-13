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

#ifndef TEST_MOCK_PIPE_SERVER_HPP_
#define TEST_MOCK_PIPE_SERVER_HPP_

#include <vector>
#include "gmock/gmock.h"
#include "winss/pipe_server.hpp"

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

    MockOutboundPipeServer& operator=(const MockOutboundPipeServer&) = delete;
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
    MOCK_METHOD1(AddListener, void(winss::PipeServerReceiveListener* listener));

    MockInboundPipeServer& operator=(const MockInboundPipeServer&) = delete;
    MockInboundPipeServer& operator=(MockInboundPipeServer&&) = delete;
};
class MockPipeServerReceiveListener : public winss::PipeServerReceiveListener {
 public:
    MOCK_METHOD1(Received, bool(const std::vector<char>& data));
};
}  // namespace winss

#endif  // TEST_MOCK_PIPE_SERVER_HPP_
