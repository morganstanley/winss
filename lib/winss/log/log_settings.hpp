#ifndef LIB_WINSS_LOG_LOG_SETTINGS_HPP_
#define LIB_WINSS_LOG_LOG_SETTINGS_HPP_

#include <filesystem>

namespace fs = std::experimental::filesystem;

namespace winss {
struct LogSettings {
    unsigned int number = 10;
    unsigned int file_size = 99999;
    fs::path log_dir = ".";
};
}  // namespace winss

#endif  // LIB_WINSS_LOG_LOG_SETTINGS_HPP_
