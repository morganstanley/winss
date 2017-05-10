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

#include "environment.hpp"
#include <windows.h>
#include <filesystem>
#include <vector>
#include <iterator>
#include <algorithm>
#include <string>
#include "easylogging/easylogging++.hpp"
#include "windows_interface.hpp"
#include "filesystem_interface.hpp"
#include "utils.hpp"

namespace fs = std::experimental::filesystem;

winss::EnvironmentDir::EnvironmentDir(fs::path env_dir) : env_dir(env_dir) {}

std::vector<char> winss::EnvironmentDir::ReadEnv() {
    winss::env_t env = winss::Utils::GetEnvironmentVariables();

    std::vector<char> env_string;
    if (!FILESYSTEM.DirectoryExists(env_dir)) {
        VLOG(4) << "Env folder does not exist in service dir";
        return env_string;
    }

    VLOG(3) << "Using env folder in service dir";

    for (auto& file : FILESYSTEM.GetFiles(env_dir)) {
        std::string key = file.filename().string();

        if (key.front() == L'.' || key.find('=') != std::string::npos) {
            VLOG(4) << "Skipping file " << file;
            continue;
        }

        VLOG(4) << "Found env file " << file;

        std::string value = FILESYSTEM.Read(file);

        if (value.empty()) {
            env.erase(key);
        } else {
            env[key] = winss::Utils::ExpandEnvironmentVariables(value);
        }
    }

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
