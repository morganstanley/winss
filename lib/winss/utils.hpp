#ifndef LIB_WINSS_UTILS_HPP_
#define LIB_WINSS_UTILS_HPP_

#include <chrono>
#include <string>
#include <map>
#include "case_ignore.hpp"

namespace winss {
typedef std::map<std::string, std::string, winss::case_ignore> env_t;
class Utils {
 private:
    Utils() {}

 public:
    static std::string ExpandEnvironmentVariables(const std::string& value);
    static env_t GetEnvironmentVariables();
    static std::string ConvertToISOString(
        const std::chrono::system_clock::time_point& time_point);
    static std::chrono::system_clock::time_point ConvertFromISOString(
        const std::string& iso_string);
};
}  // namespace winss

#endif  // LIB_WINSS_UTILS_HPP_
