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

#ifndef LIB_WINSS_UTILS_HPP_
#define LIB_WINSS_UTILS_HPP_

#include <chrono>
#include <string>
#include <map>
#include "case_ignore.hpp"

namespace winss {
/**
 * The environment mapping.
 */
typedef std::map<std::string, std::string, winss::case_ignore> env_t;

/**
 * Utility functions.
 */
class Utils {
 private:
    /**
     * Disable creating instances.
     */
    Utils() {}

 public:
    /**
     * Expand the given string with environment variables.
     *
     * Strings like %ENV_KEY% will be replaces with the environment
     * variable value.
     *
     * \param value The string to replace environment variables.
     * \return A new string with the replacements filled in.
     */
    static std::string ExpandEnvironmentVariables(const std::string& value);

    /**
     * Gets a mapping of the current environment variables.
     *
     * \return The environment as a mapping of keys to values.
     */
    static env_t GetEnvironmentVariables();

    /**
     * Convert the time to a ISO string.
     *
     * \param time_point The time point to convert.
     * \return The ISO string.
     */
    static std::string ConvertToISOString(
        const std::chrono::system_clock::time_point& time_point);

    /**
     * Convert an ISO string to a time point.
     *
     * \param iso_string The ISO string to convert.
     * \return The time point.
     */
    static std::chrono::system_clock::time_point ConvertFromISOString(
        const std::string& iso_string);
};
}  // namespace winss

#endif  // LIB_WINSS_UTILS_HPP_
