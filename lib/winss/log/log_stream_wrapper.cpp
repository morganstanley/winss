#include "log_stream_wrapper.hpp"
#include <easylogging/easylogging++.hpp>
#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::experimental::filesystem;

bool winss::LogStreamReader::IsEOF() const {
    return eof;
}

std::string winss::LogStreamReader::GetLine() {
    std::string line;
    if (!std::getline(std::cin, line)) {
        eof = true;
    }
    return line;
}

bool winss::LogStreamWriter::Open(fs::path log_path) {
    try {
        file_stream.open(log_path,
            std::ios::out | std::ios::in | std::ios::app);
    } catch (const std::exception&) {
        LOG(ERROR) << "Could not open log file " << log_path;
        return false;
    }

    return true;
}

void winss::LogStreamWriter::Write(const std::string& line) {
    file_stream << line << std::endl;
}

std::streamoff winss::LogStreamWriter::GetPos() {
    return file_stream.tellp();
}

void winss::LogStreamWriter::Close() {
    file_stream.close();
}

winss::LogStreamWriter::~LogStreamWriter() {
    Close();
}
