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

#include "log_settings_parser.hpp"
#include <filesystem>
#include <vector>
#include <string>
#include "easylogging/easylogging++.hpp"
#include "../filesystem_interface.hpp"
#include "log_settings.hpp"

namespace fs = std::experimental::filesystem;

winss::LogSettings winss::LogSettingsParser::Parse(
    const std::vector<std::string>& directives) const {
    winss::LogSettings settings;

    for (const std::string& directive : directives) {
        if (directive.length() > 0) {
            const char id = directive.at(0);
            std::string value;

            if (directive.length() > 1) {
                value = directive.substr(1);
            }

            switch (id) {
            case 'n':
                try {
                    unsigned int number = std::stoul(value, nullptr, 10);

                    VLOG(3) << "Log number set to " << number;
                    settings.number = number;
                } catch (const std::exception&) {
                    LOG(WARNING) << "The number '" << value << "' is invalid";
                }
                break;
            case 's':
                try {
                    unsigned int file_size = std::stoul(value, nullptr, 10);

                    if (file_size < 4096) {
                        file_size = 4096;
                    }

                    VLOG(3) << "Log file size set to " << file_size;
                    settings.file_size = file_size;
                } catch (const std::exception&) {
                    LOG(WARNING) << "File size '" << value << "' is invalid";
                }
                break;
            case 'T':
                settings.timestamp = true;
                VLOG(3) << "Prepend ISO 8601 timestamp";
                break;
            case '.':
                settings.log_dir = directive;
                VLOG(3) << "Log dir set to " << directive;
                break;
            default:
                if (directive.length() > 2 && directive[1] == ':') {
                    settings.log_dir = directive;
                    VLOG(3) << "Log dir set to " << directive;
                } else {
                    LOG(WARNING) << "Unrecognized script directive ";
                }
                break;
            }
        }
    }

    settings.log_dir = FILESYSTEM.Absolute(settings.log_dir);

    return settings;
}
