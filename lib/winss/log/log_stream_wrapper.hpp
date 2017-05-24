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

#ifndef LIB_WINSS_LOG_LOG_STREAM_WRAPPER_HPP_
#define LIB_WINSS_LOG_LOG_STREAM_WRAPPER_HPP_

#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::experimental::filesystem;

namespace winss {
/**
 * A stream reader for reading logs.
 */
class LogStreamReader {
 private:
    bool eof = false;  /**< End of file flag. */

 public:
    /**
     * Log stream reader constructor.
     */
    LogStreamReader() {}
    LogStreamReader(const LogStreamReader&) = delete;  /**< No copy. */
    LogStreamReader(LogStreamReader&&) = delete;  /**< No move. */

    /**
     * Gets the end of file state.
     *
     * \return True if is end of file otherwise false.
     */
    virtual bool IsEOF() const;

    /**
     * Blocks for the next log line.
     *
     * This function will block the current thread until a new line character
     * occurs or the stream reaches the end.
     *
     * \return The next log line as a string.
     */
    virtual std::string GetLine();

    void operator=(const LogStreamReader&) = delete;  /**< No copy. */
    LogStreamReader& operator=(LogStreamReader&&) = delete; /** No move. */
};

/**
 * A stream writer for writing logs.
 */
class LogStreamWriter {
 private:
    std::ofstream file_stream;

 public:
    /**
     * Log stream writer constructor.
     */
    LogStreamWriter() {}
    LogStreamWriter(const LogStreamWriter&) = delete;  /**< No copy. */
    LogStreamWriter(LogStreamWriter&&) = delete;  /**< No move. */

     /**
     * Opens the file stream at the given location.
     *
     * \param[in] log_path The location of the log file.
     * \return True if the open was successful otherwise false.
     */
    virtual bool Open(fs::path log_path);

    /**
     * Writes the given string to the log stream.
     *
     * \param[in] line The log line as a string.
     */
    virtual void Write(const std::string& line);

    /**
    * Writes a line terminator to the stream.
    */
    virtual void WriteLine();

    /**
     * Gets the current position in the stream.
     *
     * \return The current stream position.
     */
    virtual std::streamoff GetPos();

     /**
     * Closes the currently open stream.
     */
    virtual void Close();

    void operator=(const LogStreamWriter&) = delete;  /**< No copy. */
    LogStreamWriter& operator=(LogStreamWriter&&) = delete;  /**< No move. */

    /**
     * Log stream writer destructor.
     */
    virtual ~LogStreamWriter();
};
}  // namespace winss

#endif  // LIB_WINSS_LOG_LOG_STREAM_WRAPPER_HPP_
