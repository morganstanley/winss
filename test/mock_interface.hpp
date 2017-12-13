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

#ifndef TEST_MOCK_INTERFACE_HPP_
#define TEST_MOCK_INTERFACE_HPP_

#include <memory>
#include "gmock/gmock.h"

using ::testing::NiceMock;

namespace winss {
template<typename TInterface>
class MockInterface {
 private:
    std::shared_ptr<NiceMock<TInterface>> instance;
 public:
    MockInterface() : instance(std::make_shared<NiceMock<TInterface>>()) {
        TInterface::instance = instance;
    }

    TInterface& operator*() const {
        return instance.operator*();
    }

    TInterface* operator->() const {
        return instance.operator->();
    }

    ~MockInterface() {
        TInterface::instance = nullptr;
    }
};
}  // namespace winss

#endif  // TEST_MOCK_INTERFACE_HPP_
