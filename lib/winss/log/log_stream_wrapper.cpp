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

#include "log_stream_wrapper.hpp"
#include <filesystem>
#include <iostream>
#include <string>
#include "easylogging/easylogging++.hpp"

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
            std::ios::out | std::ios::in | std::ios::app | std::ios::ate);
    } catch (const std::exception&) {
        LOG(ERROR) << "Could not open log file " << log_path;
        return false;
    }

    return true;
}

void winss::LogStreamWriter::Write(const std::string& line) {
    file_stream << line;
}

void winss::LogStreamWriter::WriteLine() {
    file_stream << std::endl;
}

std::streamoff winss::LogStreamWriter::GetPos() {
    return file_stream.tellp();
}

void winss::LogStreamWriter::Close() {
    file_stream.flush();
    file_stream.close();
}

winss::LogStreamWriter::~LogStreamWriter() {
    Close();
}
