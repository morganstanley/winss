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

#ifndef TEST_MOCK_PATH_MUTEX_HPP_
#define TEST_MOCK_PATH_MUTEX_HPP_

#include <filesystem>
#include <string>
#include "gmock/gmock.h"
#include "winss/path_mutex.hpp"

namespace fs = std::experimental::filesystem;

namespace winss {
class MockPathMutex : winss::PathMutex {
 public:
    MockPathMutex(fs::path path, std::string name) {}

    MOCK_METHOD0(Lock, bool());

    MOCK_CONST_METHOD0(CanLock, bool());
    MOCK_CONST_METHOD0(HasLock, bool());
    MOCK_CONST_METHOD0(GetName, const std::string&());
};
}  // namespace winss

#endif  // TEST_MOCK_PATH_MUTEX_HPP_
