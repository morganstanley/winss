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

#ifndef TEST_SVSCAN_MOCK_SERVICE_HPP_
#define TEST_SVSCAN_MOCK_SERVICE_HPP_

#include <filesystem>
#include <utility>
#include <string>
#include "gmock/gmock.h"
#include "winss/svscan/service.hpp"

namespace fs = std::experimental::filesystem;

using ::testing::NiceMock;
using ::testing::ReturnRef;

namespace winss {
class MockService : public virtual winss::Service {
 public:
    MockService() {
        ON_CALL(*this, GetName()).WillByDefault(ReturnRef(name));
    }

    MockService(std::string name, const fs::path& service_dir) :
        winss::Service::ServiceTmpl(name, service_dir) {}

    MockService(const MockService&) = delete;

    MockService(MockService&& s) :
        winss::Service::ServiceTmpl(std::move(s)) {}

    MOCK_CONST_METHOD0(GetName, const std::string&());
    MOCK_METHOD0(Reset, void());
    MOCK_METHOD0(Check, void());
    MOCK_METHOD1(Close, bool(bool ignore_flagged));

    void operator=(const MockService&) = delete;

    MockService& operator=(MockService&& p) {
        winss::Service::operator=(std::move(p));
        return *this;
    }
};
class NiceMockService : public NiceMock<MockService> {
 public:
    NiceMockService() {}

    NiceMockService(std::string name, const fs::path& service_dir) :
        winss::Service::ServiceTmpl(name, service_dir) {}

    NiceMockService(const NiceMockService&) = delete;

    NiceMockService(NiceMockService&& p) :
        winss::Service::ServiceTmpl(std::move(p)) {}

    void operator=(const NiceMockService&) = delete;

    NiceMockService& operator=(NiceMockService&& p) {
        winss::Service::operator=(std::move(p));
        return *this;
    }
};
}  // namespace winss

#endif  // TEST_SVSCAN_MOCK_SERVICE_HPP_
