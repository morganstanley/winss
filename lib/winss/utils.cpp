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

#define NOMINMAX
#include "utils.hpp"
#include <windows.h>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <memory>
#include <vector>
#include <chrono>
#include <string>
#include <map>
#include "date/date.hpp"
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

std::vector<char> winss::Utils::GetEnvironmentString(
    const winss::env_t& env) {
    std::vector<char> env_string;

    for (auto& kv : env) {
        std::copy(kv.first.begin(), kv.first.end(),
            std::back_inserter(env_string));
        env_string.push_back('=');
        std::copy(kv.second.begin(), kv.second.end(),
            std::back_inserter(env_string));
        env_string.push_back('\0');
    }
    env_string.push_back('\0');

    return env_string;
}

std::vector<std::string> winss::Utils::SplitString(
    const std::string& input) {
    std::vector<std::string> output;

    std::stringstream ss(input);
    std::string to;

    while (std::getline(ss, to)) {
        if (!to.empty()) {
            output.push_back(to);
        }
    }

    return output;
}

std::string winss::Utils::ConvertToISOString(
    const std::chrono::system_clock::time_point& time_point) {
    return date::format("%F %T",
        date::floor<std::chrono::milliseconds>(time_point));
}

std::chrono::system_clock::time_point winss::Utils::ConvertFromISOString(
    const std::string& iso_string) {
    std::istringstream ss;
    ss.str(iso_string);
    std::chrono::system_clock::time_point time_point;
    ss >> date::parse("%F %T", time_point);
    return time_point;
}
