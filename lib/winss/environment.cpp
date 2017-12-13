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

std::vector<char> winss::Environment::ReadEnv() {
    auto env_source = this->ReadEnvSource();
    if (env_source.empty()) {
        return std::vector<char>{};
    }

    winss::env_t env = winss::Utils::GetEnvironmentVariables();

    for (auto const &kv : env_source) {
        if (kv.second.empty()) {
            env.erase(kv.first);
        } else {
            env[kv.first] =
                winss::Utils::ExpandEnvironmentVariables(kv.second);
        }
    }

    return winss::Utils::GetEnvironmentString(env);
}

winss::EnvironmentDir::EnvironmentDir(fs::path env_dir) : env_dir(env_dir) {}

winss::env_t winss::EnvironmentDir::ReadEnvSource() {
    winss::env_t env;

    std::vector<fs::path> dirs;
    std::string env_file = FILESYSTEM.Read(env_dir);
    if (!env_file.empty()) {
        VLOG(5) << "Found env file rather than dir: " << env_file;
        for (const std::string& str : winss::Utils::SplitString(env_file)) {
            dirs.push_back(fs::path(str));
        }
    } else {
        dirs.push_back(env_dir);
    }

    for (const auto& dir : dirs) {
        VLOG(5) << "Inspecting env dir: " << dir;

        for (auto& file : FILESYSTEM.GetFiles(dir)) {
            std::string key = file.filename().string();

            if (key.front() == L'.' || key.find('=') != std::string::npos) {
                VLOG(4) << "Skipping file " << file;
                continue;
            }

            VLOG(4) << "Found env file " << file;

            env[key] = FILESYSTEM.Read(file);
        }
    }

    return env;
}
