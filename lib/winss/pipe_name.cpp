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

#include "pipe_name.hpp"
#include <filesystem>
#include <string>
#include "easylogging/easylogging++.hpp"
#include "filesystem_interface.hpp"
#include "sha256.hpp"

namespace fs = std::experimental::filesystem;

winss::PipeName::PipeName(fs::path path) {
    path = FILESYSTEM.CanonicalUncPath(path);
    name = "\\\\.\\pipe\\" + winss::SHA256::CalculateDigest(path.string());
}

winss::PipeName::PipeName(fs::path path, std::string name) : PipeName(path) {
    if (!name.empty()) {
        this->name += "_" + name;
    }
}

winss::PipeName::PipeName(const winss::PipeName& p) : name(p.name) {}

winss::PipeName::PipeName(winss::PipeName&& p) : name(std::move(p.name)) {}

winss::PipeName winss::PipeName::Append(const std::string& name) const {
    winss::PipeName pipe_name;

    pipe_name.name = this->name;
    if (!name.empty()) {
        pipe_name.name += "_" + name;
    }

    return pipe_name;
}

const std::string& winss::PipeName::Get() const {
    VLOG(5) << "Using pipe name: " << name;
    return name;
}

void winss::PipeName::operator=(const winss::PipeName& p) {
    name = p.name;
}

winss::PipeName& winss::PipeName::operator=(winss::PipeName&& p) {
    name = std::move(p.name);
    return *this;
}
