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

#ifndef LIB_WINSS_LOG_LOG_SETTINGS_PARSER_HPP_
#define LIB_WINSS_LOG_LOG_SETTINGS_PARSER_HPP_

#include <vector>
#include <string>
#include "log_settings.hpp"

namespace winss {
/**
 * Parser for the log settings
 */
class LogSettingsParser {
 public:
    /**
     * Parser for the log settings.
     *
     * Given the list of directives, compose the logger settings required to
     * build the logger.
     *
     * \param[in] directives The logger script directives.
     * \return The log settings
     * \see LogSettings
     */
    winss::LogSettings Parse(const std::vector<std::string>& directives) const;
};
}  // namespace winss

#endif  // LIB_WINSS_LOG_LOG_SETTINGS_PARSER_HPP_
