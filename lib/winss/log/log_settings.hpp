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

#ifndef LIB_WINSS_LOG_LOG_SETTINGS_HPP_
#define LIB_WINSS_LOG_LOG_SETTINGS_HPP_

#include <filesystem>

namespace fs = std::experimental::filesystem;

namespace winss {
/**
 * Settings for the logger.
 */
struct LogSettings {
    unsigned int number = 10;  /**< The number of archives to keep. */
    unsigned int file_size = 99999;  /**< The max file size in bytes. */
    fs::path log_dir = ".";  /**< The log directory. */
};
}  // namespace winss

#endif  // LIB_WINSS_LOG_LOG_SETTINGS_HPP_
