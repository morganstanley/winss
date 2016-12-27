#ifndef LIB_WINSS_LOG_LOG_STREAM_WRAPPER_HPP_
#define LIB_WINSS_LOG_LOG_STREAM_WRAPPER_HPP_

#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::experimental::filesystem;

namespace winss {
class LogStreamReader {
 private:
    bool eof = false;

 public:
    LogStreamReader() {}
    LogStreamReader(const LogStreamReader&) = delete;
    LogStreamReader(LogStreamReader&&) = delete;

    virtual bool IsEOF() const;
    virtual std::string GetLine();

    void operator=(const LogStreamReader&) = delete;
    LogStreamReader& operator=(LogStreamReader&&) = delete;
};
class LogStreamWriter {
 private:
    std::ofstream file_stream;

 public:
    LogStreamWriter() {}
    LogStreamWriter(const LogStreamWriter&) = delete;
    LogStreamWriter(LogStreamWriter&&) = delete;

    virtual bool Open(fs::path log_path);
    virtual void Write(const std::string& line);
    virtual std::streamoff GetPos();
    virtual void Close();

    void operator=(const LogStreamWriter&) = delete;
    LogStreamWriter& operator=(LogStreamWriter&&) = delete;

    virtual ~LogStreamWriter();
};
}  // namespace winss

#endif  // LIB_WINSS_LOG_LOG_STREAM_WRAPPER_HPP_
