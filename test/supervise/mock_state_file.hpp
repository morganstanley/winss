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

#ifndef TEST_SUPERVISE_MOCK_STATE_FILE_HPP_
#define TEST_SUPERVISE_MOCK_STATE_FILE_HPP_

#include <filesystem>
#include <string>
#include "gmock/gmock.h"
#include "winss/supervise/state_file.hpp"
#include "winss/supervise/supervise.hpp"

namespace fs = std::experimental::filesystem;

namespace winss {
class MockSuperviseStateFile : public winss::SuperviseStateFile {
 public:
    explicit MockSuperviseStateFile(fs::path service_dir) :
        winss::SuperviseStateFile::SuperviseStateFile(service_dir) {}

    MockSuperviseStateFile(const MockSuperviseStateFile&) = delete;
    MockSuperviseStateFile(MockSuperviseStateFile&&) = delete;

    MOCK_CONST_METHOD0(GetPath, const fs::path&());
    MOCK_METHOD2(Notify, bool(winss::SuperviseNotification notification,
        const winss::SuperviseState& state));
    MOCK_CONST_METHOD1(Read, bool(winss::SuperviseState* state));
    MOCK_CONST_METHOD1(Format, std::string(winss::SuperviseState* state));

    void operator=(const MockSuperviseStateFile&) = delete;
    MockSuperviseStateFile& operator=(MockSuperviseStateFile&&) = delete;
};
}  // namespace winss

#endif  // TEST_SUPERVISE_MOCK_STATE_FILE_HPP_
