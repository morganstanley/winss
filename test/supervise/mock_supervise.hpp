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

#ifndef TEST_SUPERVISE_MOCK_SUPERVISE_HPP_
#define TEST_SUPERVISE_MOCK_SUPERVISE_HPP_

#include <filesystem>
#include "gmock/gmock.h"
#include "winss/not_owning_ptr.hpp"
#include "winss/wait_multiplexer.hpp"
#include "winss/supervise/supervise.hpp"

namespace fs = std::experimental::filesystem;

namespace winss {
class MockSupervise : public winss::Supervise {
 public:
    MockSupervise(winss::NotOwningPtr<winss::WaitMultiplexer> multiplexer,
        const fs::path& service_dir) :
        winss::Supervise::SuperviseTmpl(multiplexer, service_dir) {}

    MockSupervise(const MockSupervise&) = delete;
    MockSupervise(MockSupervise&&) = delete;

    MOCK_CONST_METHOD0(GetState, const SuperviseState&());
    MOCK_METHOD1(AddListener, bool(winss::SuperviseListener* listener));
    MOCK_METHOD0(Up, void());
    MOCK_METHOD0(Once, void());
    MOCK_METHOD0(OnceAtMost, void());
    MOCK_METHOD0(Down, void());
    MOCK_METHOD0(Kill, void());
    MOCK_METHOD0(Term, void());
    MOCK_METHOD0(Exit, void());

    MockSupervise& operator=(const MockSupervise&) = delete;
    MockSupervise& operator=(MockSupervise&&) = delete;
};
class MockSuperviseListener : public winss::SuperviseListener {
 public:
    MOCK_METHOD2(Notify, bool(SuperviseNotification notification,
        const SuperviseState& state));
};
}  // namespace winss

#endif  // TEST_SUPERVISE_MOCK_SUPERVISE_HPP_
