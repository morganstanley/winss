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

#ifndef TEST_LOG_MOCK_LOG_STREAM_WRAPPER_HPP_
#define TEST_LOG_MOCK_LOG_STREAM_WRAPPER_HPP_

#include <filesystem>
#include <string>
#include "gmock/gmock.h"
#include "winss/log/log_stream_wrapper.hpp"

namespace fs = std::experimental::filesystem;

namespace winss {
class MockLogStreamReader : public winss::LogStreamReader {
 public:
    MockLogStreamReader() {}
    MockLogStreamReader(const MockLogStreamReader&) = delete;
    MockLogStreamReader(MockLogStreamReader&&) = delete;

    MOCK_CONST_METHOD0(IsEOF, bool());
    MOCK_METHOD0(GetLine, std::string());

    void operator=(const MockLogStreamReader&) = delete;
    MockLogStreamReader& operator=(MockLogStreamReader&&) = delete;
};
class MockLogStreamWriter : public winss::LogStreamWriter {
 public:
    MockLogStreamWriter() {}
    MockLogStreamWriter(const MockLogStreamWriter&) = delete;
    MockLogStreamWriter(MockLogStreamWriter&&) = delete;

    MOCK_METHOD1(Open, bool(fs::path log_path));
    MOCK_METHOD1(Write, void(const std::string& line));
    MOCK_METHOD0(GetPos, std::streamoff());
    MOCK_METHOD0(Close, void());

    void operator=(const MockLogStreamWriter&) = delete;
    MockLogStreamWriter& operator=(MockLogStreamWriter&&) = delete;

    virtual ~MockLogStreamWriter() {}
};
}  // namespace winss

#endif  // TEST_LOG_MOCK_LOG_STREAM_WRAPPER_HPP_
