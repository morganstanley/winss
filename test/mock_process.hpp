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

#ifndef TEST_MOCK_PROCESS_HPP_
#define TEST_MOCK_PROCESS_HPP_

#include <windows.h>
#include <utility>
#include "gmock/gmock.h"
#include "winss/process.hpp"
#include "winss/handle_wrapper.hpp"

using ::testing::NiceMock;

namespace winss {
class MockProcess : virtual public winss::Process {
 public:
    MockProcess() : winss::Process::Process() {}
    MockProcess(const MockProcess&) = delete;
    MockProcess(MockProcess&& p) : winss::Process::Process(std::move(p)) {}

    MOCK_CONST_METHOD0(GetProcessId, DWORD());
    MOCK_CONST_METHOD0(GetExitCode, DWORD());
    MOCK_CONST_METHOD0(IsCreated, bool());
    MOCK_CONST_METHOD0(IsActive, bool());
    MOCK_CONST_METHOD0(GetHandle, winss::HandleWrapper());

    MOCK_METHOD1(Create, bool(const ProcessParams& params));
    MOCK_METHOD0(SendBreak, void());
    MOCK_METHOD0(Terminate, void());
    MOCK_METHOD0(Close, void());

    MockProcess& operator=(const MockProcess&) = delete;

    MockProcess& operator=(MockProcess&& p) {
        winss::Process::operator=(std::move(p));
        return *this;
    }
};
class NiceMockProcess : virtual public NiceMock<MockProcess> {
 public:
    NiceMockProcess() : winss::Process::Process() {}
    NiceMockProcess(const NiceMockProcess&) = delete;
    NiceMockProcess(NiceMockProcess&& p) :
        winss::Process::Process(std::move(p)) {}

    NiceMockProcess& operator=(const NiceMockProcess&) = delete;

    NiceMockProcess& operator=(NiceMockProcess&& instance) {
        winss::Process::operator=(std::move(instance));
        return *this;
    }
};
}  // namespace winss

#endif  // TEST_MOCK_PROCESS_HPP_
