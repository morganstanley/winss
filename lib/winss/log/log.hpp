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
#include "log_settings.hpp"
#include "log_stream_wrapper.hpp"

namespace fs = std::experimental::filesystem;

namespace winss {
struct LogArchiveFile {
    fs::path file;
    unsigned __int64 time;
};
template<typename TMutex>
class LogTmpl {
 protected:
    winss::NotOwningPtr<winss::LogStreamReader> reader;
    winss::NotOwningPtr<winss::LogStreamWriter> writer;
    const winss::LogSettings& settings;
    fs::path current;
    TMutex mutex;
    std::regex pattern;

    bool Rotate() const {
        writer->Close();

        auto now = std::chrono::system_clock::now();
        std::ostringstream os;
        os << kArchivePrefix << now.time_since_epoch().count() << ".u";

        fs::path archive = settings.log_dir / os.str();

        FILESYSTEM.Rename(current, archive);

        return writer->Open(current);
    }

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
    static const int kMutexTaken = 100;
    static const int kFatalExitCode = 111;
    static constexpr const char kCurrentLog[8] = "current";
    static constexpr const char kArchivePrefix[2] = "@";
    static constexpr const char kMutexName[4] = "log";

    LogTmpl(winss::NotOwningPtr<winss::LogStreamReader> reader,
        winss::NotOwningPtr<winss::LogStreamWriter> writer,
        const winss::LogSettings& settings) : reader(reader), writer(writer),
        settings(settings), mutex(settings.log_dir, kMutexName),
        pattern("^" + std::string(kArchivePrefix) + "([\\d]+)\\.\\w$") {
        current = settings.log_dir / fs::path(kCurrentLog);
    }

    LogTmpl(const LogTmpl&) = delete;
    LogTmpl(LogTmpl&&) = delete;

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

            writer->Write(reader->GetLine());
        }

        writer->Close();
        return 0;
    }

    void operator=(const LogTmpl&) = delete;
    LogTmpl& operator=(LogTmpl&&) = delete;
};
typedef LogTmpl<winss::PathMutex> Log;
}  // namespace winss

#endif  // LIB_WINSS_LOG_LOG_HPP_
