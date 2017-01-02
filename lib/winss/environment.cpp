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
