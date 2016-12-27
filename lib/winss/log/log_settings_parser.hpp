#ifndef LIB_WINSS_LOG_LOG_SETTINGS_PARSER_HPP_
#define LIB_WINSS_LOG_LOG_SETTINGS_PARSER_HPP_

#include <vector>
#include <string>
#include "log_settings.hpp"

namespace winss {
struct LogSettingsParser {
 public:
    winss::LogSettings Parse(const std::vector<std::string>& directives) const;
};
}  // namespace winss

#endif  // LIB_WINSS_LOG_LOG_SETTINGS_PARSER_HPP_
