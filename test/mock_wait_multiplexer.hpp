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

#ifndef TEST_MOCK_WAIT_MULTIPLEXER_HPP_
#define TEST_MOCK_WAIT_MULTIPLEXER_HPP_

#include <windows.h>
#include <functional>
#include <vector>
#include <string>
#include "gmock/gmock.h"
#include "winss/wait_multiplexer.hpp"

using ::testing::_;
using ::testing::Invoke;

namespace winss {
class MockWaitMultiplexer : public winss::WaitMultiplexer {
 public:
    std::vector<winss::Callback> mock_init_callbacks;
    std::vector<winss::TriggeredCallback> mock_triggered_callbacks;
    std::vector<winss::Callback> mock_timeout_callbacks;
    std::vector<winss::Callback> mock_stop_callbacks;

    MockWaitMultiplexer() : winss::WaitMultiplexer::WaitMultiplexer() {
        auto add_init = [&](winss::Callback callback) {
            mock_init_callbacks.push_back(callback);
        };
        ON_CALL(*this, AddInitCallback(_)).WillByDefault(Invoke(add_init));

        auto add_triggered = [&](const winss::HandleWrapper& handle,
            winss::TriggeredCallback callback) {
            mock_triggered_callbacks.push_back(callback);
        };
        ON_CALL(*this, AddTriggeredCallback(_, _))
            .WillByDefault(Invoke(add_triggered));

        auto add_timeout = [&](DWORD timeout, winss::Callback callback,
            std::string group) {
            mock_timeout_callbacks.push_back(callback);
        };
        ON_CALL(*this, AddTimeoutCallback(_, _, _))
            .WillByDefault(Invoke(add_timeout));

        auto add_stop = [&](winss::Callback callback) {
            mock_stop_callbacks.push_back(callback);
        };
        ON_CALL(*this, AddStopCallback(_)).WillByDefault(Invoke(add_stop));
    }

    MOCK_METHOD1(AddInitCallback, void(winss::Callback callback));
    MOCK_METHOD2(AddTriggeredCallback, void(const winss::HandleWrapper& handle,
        winss::TriggeredCallback callback));
    MOCK_METHOD3(AddTimeoutCallback, void(DWORD timeout,
        winss::Callback callback, std::string group));
    MOCK_METHOD1(AddStopCallback, void(winss::Callback callback));

    MOCK_METHOD1(RemoveTriggeredCallback, bool(
        const winss::HandleWrapper& handle));
    MOCK_METHOD1(RemoveTimeoutCallback, bool(std::string group));

    MOCK_CONST_METHOD0(GetTimeout, DWORD());

    MOCK_METHOD0(Start, int());
    MOCK_METHOD1(Stop, void(int code));

    MOCK_METHOD2(AddCloseEvent, void(const winss::EventWrapper& close_event,
        DWORD return_code));

    MOCK_CONST_METHOD0(IsStopping, bool());
    MOCK_CONST_METHOD0(HasStarted, bool());
    MOCK_CONST_METHOD0(GetReturnCode, int());
};
}  // namespace winss

#endif  // TEST_MOCK_WAIT_MULTIPLEXER_HPP_
