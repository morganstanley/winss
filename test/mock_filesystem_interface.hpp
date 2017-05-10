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

#ifndef TEST_MOCK_FILESYSTEM_INTERFACE_HPP_
#define TEST_MOCK_FILESYSTEM_INTERFACE_HPP_

#include <filesystem>
#include <string>
#include <vector>
#include "gmock/gmock.h"
#include "winss/filesystem_interface.hpp"

namespace fs = std::experimental::filesystem;

namespace winss {
class MockFilesystemInterface : public winss::FilesystemInterface {
 public:
    using winss::FilesystemInterface::instance;

    MockFilesystemInterface() {}
    MockFilesystemInterface(const MockFilesystemInterface&) = delete;
    MockFilesystemInterface(MockFilesystemInterface&&) = delete;

    MOCK_CONST_METHOD1(Read, std::string(const fs::path& path));
    MOCK_CONST_METHOD2(Write, bool(const fs::path& path,
        const std::string& content));
    MOCK_CONST_METHOD1(ChangeDirectory, bool(const fs::path& dir));
    MOCK_CONST_METHOD1(DirectoryExists, bool(const fs::path& dir));
    MOCK_CONST_METHOD1(CreateDirectory, bool(const fs::path& dir));
    MOCK_CONST_METHOD2(Rename, bool(const fs::path& fro1, const fs::path& to));
    MOCK_CONST_METHOD1(Remove, bool(const fs::path& path));
    MOCK_CONST_METHOD1(FileExists, bool(const fs::path& path));
    MOCK_CONST_METHOD1(Absolute, fs::path(const fs::path& path));
    MOCK_CONST_METHOD1(GetDirectories, std::vector<fs::path>(
        const fs::path& path));
    MOCK_CONST_METHOD1(GetFiles, std::vector<fs::path>(
        const fs::path& path));

    void operator=(const MockFilesystemInterface&) = delete;
    MockFilesystemInterface& operator=(MockFilesystemInterface&&) = delete;
};
}  // namespace winss

#endif  // TEST_MOCK_FILESYSTEM_INTERFACE_HPP_
