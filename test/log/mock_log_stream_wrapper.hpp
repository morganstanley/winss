#ifndef TEST_LOG_MOCK_LOG_STREAM_WRAPPER_HPP_
#define TEST_LOG_MOCK_LOG_STREAM_WRAPPER_HPP_

#include <gmock/gmock.h>
#include <winss/log/log_stream_wrapper.hpp>
#include <filesystem>
#include <fstream>
#include <string>

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
