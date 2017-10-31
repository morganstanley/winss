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

#ifndef TEST_MOCK_PIPE_INSTANCE_HPP_
#define TEST_MOCK_PIPE_INSTANCE_HPP_

#include <windows.h>
#include <cstdlib>
#include <vector>
#include <utility>
#include "gmock/gmock.h"
#include "winss/pipe_instance.hpp"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace winss {
class MockPipeInstance : public virtual winss::PipeInstance {
 protected:
    HANDLE handle = reinterpret_cast<HANDLE>(
        static_cast<intptr_t>(std::rand()));

 public:
    MockPipeInstance() : winss::PipeInstance::PipeInstance() {
        ON_CALL(*this, GetHandle()).WillByDefault(Return(
            winss::HandleWrapper(handle, false)));
        ON_CALL(*this, CreateNamedPipe(_)).WillByDefault(Return(true));
        ON_CALL(*this, CreateFile(_)).WillByDefault(Return(true));
    }

    MockPipeInstance(const MockPipeInstance&) = delete;

    MockPipeInstance(MockPipeInstance&& instance) :
        winss::PipeInstance::PipeInstance(std::move(instance)) {
        handle = instance.handle;
        ON_CALL(*this, GetHandle()).WillByDefault(Return(
            winss::HandleWrapper(handle, false)));
    }

    MOCK_CONST_METHOD0(IsPendingIO, bool());
    MOCK_CONST_METHOD0(IsConnected, bool());
    MOCK_CONST_METHOD0(IsClosing, bool());
    MOCK_CONST_METHOD0(GetHandle, winss::HandleWrapper());

    MOCK_METHOD0(SetConnected, bool());
    MOCK_METHOD0(GetOverlappedResult, OverlappedResult());

    MOCK_METHOD1(CreateNamedPipe, bool(const winss::PipeName& pipe_name));
    MOCK_METHOD1(CreateFile, bool(const winss::PipeName& pipe_name));

    MOCK_METHOD0(Closing, void());
    MOCK_METHOD0(DisconnectNamedPipe, void());
    MOCK_METHOD0(Close, bool());

    MockPipeInstance& operator=(const MockPipeInstance&) = delete;

    MockPipeInstance& operator=(MockPipeInstance&& instance) {
        winss::PipeInstance::operator=(std::move(instance));
        handle = instance.handle;
        ON_CALL(*this, GetHandle()).WillByDefault(Return(
            winss::HandleWrapper(handle, false)));
        return *this;
    }
};
class NiceMockPipeInstance : public NiceMock<MockPipeInstance> {
 public:
     NiceMockPipeInstance() : winss::PipeInstance::PipeInstance() {
         ON_CALL(*this, GetHandle()).WillByDefault(Return(
             winss::HandleWrapper(handle, false)));
         ON_CALL(*this, CreateNamedPipe(_)).WillByDefault(Return(true));
         ON_CALL(*this, CreateFile(_)).WillByDefault(Return(true));
     }

     NiceMockPipeInstance(const NiceMockPipeInstance&) = delete;

     NiceMockPipeInstance(NiceMockPipeInstance&& other) :
         winss::PipeInstance::PipeInstance(std::move(other)) {
         handle = other.handle;
         ON_CALL(*this, GetHandle()).WillByDefault(Return(
             winss::HandleWrapper(handle, false)));
     }

     NiceMockPipeInstance& operator=(const NiceMockPipeInstance&) = delete;

     NiceMockPipeInstance& operator=(NiceMockPipeInstance&& instance) {
         winss::PipeInstance::operator=(std::move(instance));
         handle = instance.handle;
         ON_CALL(*this, GetHandle()).WillByDefault(Return(
             winss::HandleWrapper(handle, false)));
         return *this;
     }
};
class MockOutboundPipeInstance : public virtual winss::OutboundPipeInstance {
 protected:
    HANDLE handle = reinterpret_cast<HANDLE>(
        static_cast<intptr_t>(std::rand()));

 public:
    MockOutboundPipeInstance() :
        winss::OutboundPipeInstance::OutboundPipeInstance() {
        ON_CALL(*this, GetHandle()).WillByDefault(Return(
            winss::HandleWrapper(handle, false)));
        ON_CALL(*this, CreateNamedPipe(_)).WillByDefault(Return(true));
        ON_CALL(*this, CreateFile(_)).WillByDefault(Return(true));
    }

    MockOutboundPipeInstance(const MockOutboundPipeInstance&) = delete;

    MockOutboundPipeInstance(MockOutboundPipeInstance&& other) :
        winss::OutboundPipeInstance::OutboundPipeInstance(std::move(other)) {
        handle = other.handle;
        ON_CALL(*this, GetHandle()).WillByDefault(Return(
            winss::HandleWrapper(handle, false)));
    }

    MOCK_CONST_METHOD0(IsPendingIO, bool());
    MOCK_CONST_METHOD0(IsConnected, bool());
    MOCK_CONST_METHOD0(IsClosing, bool());
    MOCK_CONST_METHOD0(GetHandle, winss::HandleWrapper());

    MOCK_METHOD0(SetConnected, bool());
    MOCK_METHOD0(GetOverlappedResult, OverlappedResult());

    MOCK_METHOD1(CreateNamedPipe, bool(const winss::PipeName& pipe_name));
    MOCK_METHOD1(CreateFile, bool(const winss::PipeName& pipe_name));

    MOCK_METHOD0(Closing, void());
    MOCK_METHOD0(DisconnectNamedPipe, void());
    MOCK_METHOD0(Close, bool());

    MOCK_METHOD1(Queue, bool(const std::vector<char>& data));

    MOCK_CONST_METHOD0(HasMessages, bool());
    MOCK_CONST_METHOD0(IsWriting, bool());

    MOCK_METHOD0(FinishWrite, bool());
    MOCK_METHOD0(Write, bool());
    MOCK_METHOD0(Read, bool());

    MockOutboundPipeInstance& operator=(
        const MockOutboundPipeInstance&) = delete;

    MockOutboundPipeInstance& operator=(MockOutboundPipeInstance&& instance) {
        winss::PipeInstance::operator=(std::move(instance));
        handle = instance.handle;
        ON_CALL(*this, GetHandle()).WillByDefault(Return(
            winss::HandleWrapper(handle, false)));
        return *this;
    }
};
class NiceMockOutboundPipeInstance :
    public NiceMock<MockOutboundPipeInstance> {
 public:
    NiceMockOutboundPipeInstance() :
        winss::OutboundPipeInstance::OutboundPipeInstance() {
        ON_CALL(*this, GetHandle()).WillByDefault(Return(
            winss::HandleWrapper(handle, false)));
        ON_CALL(*this, CreateNamedPipe(_)).WillByDefault(Return(true));
        ON_CALL(*this, CreateFile(_)).WillByDefault(Return(true));
    }

    NiceMockOutboundPipeInstance(const NiceMockOutboundPipeInstance&) = delete;

    NiceMockOutboundPipeInstance(NiceMockOutboundPipeInstance&& other) :
        winss::OutboundPipeInstance::OutboundPipeInstance(std::move(other)) {
        handle = other.handle;
        ON_CALL(*this, GetHandle()).WillByDefault(Return(
            winss::HandleWrapper(handle, false)));
    }

    NiceMockOutboundPipeInstance& operator=(
        const NiceMockOutboundPipeInstance&) = delete;

    NiceMockOutboundPipeInstance& operator=(
        NiceMockOutboundPipeInstance&& instance) {
        winss::PipeInstance::operator=(std::move(instance));
        handle = instance.handle;
        ON_CALL(*this, GetHandle()).WillByDefault(Return(
            winss::HandleWrapper(handle, false)));
        return *this;
    }
};
class MockInboundPipeInstance : public virtual winss::InboundPipeInstance {
 protected:
    HANDLE handle = reinterpret_cast<HANDLE>(
        static_cast<intptr_t>(std::rand()));

 public:
    MockInboundPipeInstance() :
        winss::InboundPipeInstance::InboundPipeInstance() {
        ON_CALL(*this, GetHandle()).WillByDefault(Return(
            winss::HandleWrapper(handle, false)));
        ON_CALL(*this, CreateNamedPipe(_)).WillByDefault(Return(true));
        ON_CALL(*this, CreateFile(_)).WillByDefault(Return(true));
    }

    MockInboundPipeInstance(const MockInboundPipeInstance&) = delete;

    MockInboundPipeInstance(MockInboundPipeInstance&& other) :
        winss::InboundPipeInstance::InboundPipeInstance(std::move(other)) {
        handle = other.handle;
        ON_CALL(*this, GetHandle()).WillByDefault(Return(
            winss::HandleWrapper(handle, false)));
    }

    MOCK_CONST_METHOD0(IsPendingIO, bool());
    MOCK_CONST_METHOD0(IsConnected, bool());
    MOCK_CONST_METHOD0(IsClosing, bool());
    MOCK_CONST_METHOD0(GetHandle, winss::HandleWrapper());

    MOCK_METHOD0(SetConnected, bool());
    MOCK_METHOD0(GetOverlappedResult, OverlappedResult());

    MOCK_METHOD1(CreateNamedPipe, bool(const winss::PipeName& pipe_name));
    MOCK_METHOD1(CreateFile, bool(const winss::PipeName& pipe_name));

    MOCK_METHOD0(Closing, void());
    MOCK_METHOD0(DisconnectNamedPipe, void());
    MOCK_METHOD0(Close, bool());

    MOCK_METHOD0(FinishRead, bool());
    MOCK_METHOD0(Read, bool());
    MOCK_METHOD0(SwapBuffer, std::vector<char>());

    MockInboundPipeInstance& operator=(
        const MockInboundPipeInstance&) = delete;

    MockInboundPipeInstance& operator=(MockInboundPipeInstance&& instance) {
        winss::PipeInstance::operator=(std::move(instance));
        handle = instance.handle;
        ON_CALL(*this, GetHandle()).WillByDefault(Return(
            winss::HandleWrapper(handle, false)));
        return *this;
    }
};
class NiceMockInboundPipeInstance :
    public NiceMock<MockInboundPipeInstance> {
 public:
    NiceMockInboundPipeInstance() :
        winss::InboundPipeInstance::InboundPipeInstance() {
        ON_CALL(*this, GetHandle()).WillByDefault(Return(
            winss::HandleWrapper(handle, false)));
        ON_CALL(*this, CreateNamedPipe(_)).WillByDefault(Return(true));
        ON_CALL(*this, CreateFile(_)).WillByDefault(Return(true));
    }

    NiceMockInboundPipeInstance(const NiceMockInboundPipeInstance&) = delete;

    NiceMockInboundPipeInstance(NiceMockInboundPipeInstance&& other) :
        winss::InboundPipeInstance::InboundPipeInstance(std::move(other)) {
        handle = other.handle;
        ON_CALL(*this, GetHandle()).WillByDefault(Return(
            winss::HandleWrapper(handle, false)));
    }

    NiceMockInboundPipeInstance& operator=(
        const NiceMockInboundPipeInstance&) = delete;

    NiceMockInboundPipeInstance& operator=(
        NiceMockInboundPipeInstance&& instance) {
        winss::PipeInstance::operator=(std::move(instance));
        handle = instance.handle;
        ON_CALL(*this, GetHandle()).WillByDefault(Return(
            winss::HandleWrapper(handle, false)));
        return *this;
    }
};
}  // namespace winss

#endif  // TEST_MOCK_PIPE_INSTANCE_HPP_
