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

#ifndef LIB_WINSS_LOG_LOG_HPP_
#define LIB_WINSS_LOG_LOG_HPP_

#include <filesystem>
#include <algorithm>
#include <vector>
#include <regex>
#include <chrono>
#include <utility>
#include <string>
#include "easylogging/easylogging++.hpp"
#include "../filesystem_interface.hpp"
#include "../not_owning_ptr.hpp"
#include "../path_mutex.hpp"
#include "../utils.hpp"
#include "log_settings.hpp"
#include "log_stream_wrapper.hpp"

namespace fs = std::experimental::filesystem;

namespace winss {

/**
 * An archived log file.
 *
 * Holds information about an archived log file in the log directory.
 */
struct LogArchiveFile {
    fs::path file;  ///< The archive file name.
    unsigned __int64 time;  ///< The time the archive was taken.
};

/**
 * The logger template.
 *
 * Reads from STDIN and writes to a log file. It will occasionally rotate the
 * log file when it gets too big.
 */
template<typename TMutex>
class LogTmpl {
 protected:
    winss::NotOwningPtr<winss::LogStreamReader> reader;  /**< Log input. */
    winss::NotOwningPtr<winss::LogStreamWriter> writer;  /**< Log output. */
    const winss::LogSettings& settings;  /**< Logger settings. */
    fs::path current;    /**< Current log file. */
    TMutex mutex;        /**< Log dir global mutex. */
    std::regex pattern;  /**< Log file pattern when rotating files. */

    /**
     * Rotates the current log file.
     *
     * Current file is closed and renamed then a new current file is opened.
     *
     * \return True if the rotation succeeded and false otherwise.
     */
    bool Rotate() const {
        writer->Close();

        auto now = std::chrono::system_clock::now();
        std::ostringstream os;
        os << kArchivePrefix << now.time_since_epoch().count() << ".u";

        fs::path archive = settings.log_dir / os.str();

        FILESYSTEM.Rename(current, archive);

        return writer->Open(current);
    }

    /**
     * Deletes old archive files.
     *
     * Search the directory given an archive pattern and delete the archives
     * which exceed the total number of archives defined in the settings.
     */
    void CleanArchives() const {
        VLOG(3) << "Cleaning old archives";

        std::vector<fs::path> files = FILESYSTEM.GetFiles(settings.log_dir);
        if (files.size() <= settings.number) {
            VLOG(3) << "Skipping archive clean up";
            return;
        }

        std::vector<winss::LogArchiveFile> archives;
        for (const fs::path& file : files) {
            std::string s = file.filename().string();
            std::smatch match;
            if (std::regex_search(s, match, pattern) && match.size() > 1) {
                std::string m = match[1].str();
                unsigned __int64 time = std::strtoull(m.c_str(), nullptr, 10);
                winss::LogArchiveFile archive{ file, time };
                archives.push_back(std::move(archive));
            }
        }

        std::sort(archives.begin(), archives.end(),
            [](const winss::LogArchiveFile& f1,
                const winss::LogArchiveFile& f2) {
            return f1.time < f2.time;
        });

        __int64 to_delete = archives.size() - settings.number;
        if (to_delete <= 0) {
            VLOG(3)
                << "Not cleaning up any archives because there are only "
                << archives.size();
            return;
        }

        VLOG(3) << "Removing " << to_delete << " archives";
        for (size_t i = 0; to_delete; ++i, --to_delete) {
            FILESYSTEM.Remove(archives.at(i).file);
        }
    }

 public:
    static const int kMutexTaken = 100;  /**< Log dir in use error. */
    static const int kFatalExitCode = 111;  /**< Something went wrong. */
    static constexpr const char kCurrentLog[8] = "current";  /**< Log file. */
    static constexpr const char kArchivePrefix[2] = "@";  /**< File prefix. */
    static constexpr const char kMutexName[4] = "log";  /**< Mutex name. */

    /**
     * Log template constructor.
     *
     * \param reader The log stream reader.
     * \param writer The log stream writer.
     * \param settings The logger settings.
     */
    LogTmpl(winss::NotOwningPtr<winss::LogStreamReader> reader,
        winss::NotOwningPtr<winss::LogStreamWriter> writer,
        const winss::LogSettings& settings) : reader(reader), writer(writer),
        settings(settings), mutex(settings.log_dir, kMutexName),
        pattern("^" + std::string(kArchivePrefix) + "([\\d]+)\\.\\w$") {
        current = settings.log_dir / fs::path(kCurrentLog);
    }

    LogTmpl(const LogTmpl&) = delete;  /**< No copy. */
    LogTmpl(LogTmpl&&) = delete;  /**< No move. */

    /**
     * Starts the logging implementation.
     *
     * Obtains a log on the log dir, starts reading from the reader and
     * writing to the writer until EOF is reached. When a rotation occurs
     * then clean archives will be invoked.
     *
     * \return The return code.
     */
    int Start() {
        if (!FILESYSTEM.DirectoryExists(settings.log_dir)) {
            LOG(ERROR)
                << "Directory "
                << settings.log_dir
                << " does not exist";
            return kFatalExitCode;
        }

        if (!mutex.Lock()) {
            return kMutexTaken;
        }

        if (!writer->Open(current)) {
            return kFatalExitCode;
        }

        while (!reader->IsEOF()) {
            std::streamoff pos = writer->GetPos();

            if (pos > settings.file_size) {
                if (!Rotate()) {
                    return kFatalExitCode;
                }

                CleanArchives();
            }

            std::string line = reader->GetLine();

            if (settings.timestamp) {
                auto now = std::chrono::system_clock::now();
                writer->Write(winss::Utils::ConvertToISOString(now));
                writer->Write(" ");
            }

            writer->Write(line);
            writer->WriteLine();
        }

        writer->Close();
        return 0;
    }

    void operator=(const LogTmpl&) = delete;  /**< No copy. */
    LogTmpl& operator=(LogTmpl&&) = delete;  /**< No move. */
};

/**
 * Concrete log implementation.
 */
typedef LogTmpl<winss::PathMutex> Log;
}  // namespace winss

#endif  // LIB_WINSS_LOG_LOG_HPP_
