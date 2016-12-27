#include "utils.hpp"
#include <windows.h>
#include <sstream>
#include <iomanip>
#include <string>
#include <memory>
#include <vector>
#include <iterator>
#include <chrono>
#include <ctime>
#include <map>
#include "windows_interface.hpp"

std::string winss::Utils::ExpandEnvironmentVariables(
    const std::string& value) {
    const char* str = value.c_str();
    DWORD size = WINDOWS.ExpandEnvironmentStrings(str, nullptr, 0);

    if (size) {
        std::vector<char> expanded(size);
        if (0 == WINDOWS.ExpandEnvironmentStrings(str, &expanded[0], size)) {
            return value;
        }

        return std::string(&expanded[0]);
    }

    return value;
}

winss::env_t winss::Utils::GetEnvironmentVariables() {
    winss::env_t env;

    auto free = [](LPTCH p) { WINDOWS.FreeEnvironmentStrings(p); };
    auto env_block = std::unique_ptr<TCHAR, decltype(free)>{
        WINDOWS.GetEnvironmentStrings(), free };

    for (LPTCH i = env_block.get(); *i != '\0'; ++i) {
        std::string key;
        std::string value;

        for (; *i != '='; ++i)
            key += *i;
        ++i;
        for (; *i != '\0'; ++i)
            value += *i;

        env[key] = value;
    }

    return env;
}

std::string winss::Utils::ConvertToISOString(
    const std::chrono::system_clock::time_point& time_point) {
    std::time_t time = std::chrono::system_clock::to_time_t(time_point);
    std::tm tm;
    std::stringstream ss;
    if (!gmtime_s(&tm, &time)) {
        ss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
    }
    return ss.str();
}

std::chrono::system_clock::time_point winss::Utils::ConvertFromISOString(
    const std::string& iso_string) {
    std::tm tm = {};
    std::stringstream ss(iso_string);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    return std::chrono::system_clock::from_time_t(_mkgmtime(&tm));
}
