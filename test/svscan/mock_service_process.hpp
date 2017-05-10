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

#ifndef TEST_SVSCAN_MOCK_SERVICE_PROCESS_HPP_
#define TEST_SVSCAN_MOCK_SERVICE_PROCESS_HPP_

#include <filesystem>
#include <utility>
#include "gmock/gmock.h"
#include "winss/svscan/service_process.hpp"

namespace fs = std::experimental::filesystem;

using ::testing::NiceMock;
using ::testing::ReturnRef;

namespace winss {
class MockServiceProcess : public virtual winss::ServiceProcess {
 public:
    MockServiceProcess() {
        ON_CALL(*this, GetServiceDir()).WillByDefault(ReturnRef(service_dir));
    }

    MockServiceProcess(const fs::path& service_dir, bool is_log) :
        winss::ServiceProcess::ServiceProcessTmpl(service_dir, is_log) {}

    MockServiceProcess(const MockServiceProcess&) = delete;

    MockServiceProcess(MockServiceProcess&& p) :
        winss::ServiceProcess::ServiceProcessTmpl(std::move(p)) {}

    MOCK_CONST_METHOD0(IsFlagged, bool());
    MOCK_CONST_METHOD0(GetServiceDir, const fs::path&());
    MOCK_CONST_METHOD0(Reset, void());

    MOCK_METHOD1(Start, void(const winss::ServicePipes& pipes));
    MOCK_METHOD1(Close, bool(bool ignore_flagged));

    void operator=(const MockServiceProcess&) = delete;

    MockServiceProcess& operator=(MockServiceProcess&& p) {
        winss::ServiceProcess::operator=(std::move(p));
        return *this;
    }
};
class NiceMockServiceProcess : public NiceMock<MockServiceProcess> {
 public:
    NiceMockServiceProcess() {}

    NiceMockServiceProcess(const fs::path& service_dir, bool is_log) :
        winss::ServiceProcess::ServiceProcessTmpl(service_dir, is_log) {}

    NiceMockServiceProcess(const NiceMockServiceProcess&) = delete;

    NiceMockServiceProcess(NiceMockServiceProcess&& p) :
        winss::ServiceProcess::ServiceProcessTmpl(std::move(p)) {}

    void operator=(const NiceMockServiceProcess&) = delete;

    NiceMockServiceProcess& operator=(NiceMockServiceProcess&& p) {
        winss::ServiceProcess::operator=(std::move(p));
        return *this;
    }
};
}  // namespace winss

#endif  // TEST_SVSCAN_MOCK_SERVICE_PROCESS_HPP_
