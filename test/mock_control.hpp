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

#ifndef TEST_MOCK_CONTROL_HPP_
#define TEST_MOCK_CONTROL_HPP_

#include <vector>
#include "gmock/gmock.h"
#include "winss/control.hpp"

namespace winss {
class MockInboundControlItemListener
    : public winss::InboundControlItemListener {
 public:
    MOCK_METHOD0(IsEnabled, bool());
    MOCK_METHOD0(CanStart, bool());
    MOCK_METHOD0(HandleConnected, void());
    MOCK_METHOD1(HandleReceived, bool(const std::vector<char>& message));
};
}  // namespace winss

#endif  // TEST_MOCK_CONTROL_HPP_
