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

#include "path_mutex.hpp"
#include <windows.h>
#include <string>
#include "easylogging/easylogging++.hpp"
#include "windows_interface.hpp"
#include "filesystem_interface.hpp"
#include "sha256.hpp"

namespace fs = std::experimental::filesystem;

winss::PathMutex::PathMutex(fs::path path, std::string name) {
    path = FILESYSTEM.CanonicalUncPath(path);
    mutex_name = "Global\\" +
        winss::SHA256::CalculateDigest(path.string());

    if (!name.empty()) {
        mutex_name += "_" + name;
    }
}

bool winss::PathMutex::Lock() {
    if (HasLock()) {
        return true;
    }

    VLOG(5) << "Trying to lock named mutex " << mutex_name;

    HANDLE mutex = WINDOWS.CreateMutex(nullptr, false, mutex_name.c_str());
    bool locked = WINDOWS.WaitForSingleObject(mutex, 0) == WAIT_OBJECT_0;

    if (locked) {
        VLOG(5) << "Obtained named mutex " << mutex_name;
        lock = mutex;
        return true;
    } else {
        VLOG(1)
            << "Failed to obtain named mutex "
            << mutex_name
            << ": "
            << WINDOWS.GetLastError();
        WINDOWS.CloseHandle(mutex);
        return false;
    }
}

bool winss::PathMutex::CanLock() const {
    if (HasLock()) {
        return true;
    }

    VLOG(5) << "Checking named mutex exists " << mutex_name;

    HANDLE handle = WINDOWS.OpenMutex(MUTEX_ALL_ACCESS, false,
        mutex_name.c_str());

    if (handle == nullptr) {
        VLOG(5) << "Named mutex does not exist " << mutex_name;
        return true;
    } else {
        VLOG(5) << "Named mutex exists " << mutex_name;
        WINDOWS.CloseHandle(lock);
        return false;
    }
}

bool winss::PathMutex::HasLock() const {
    return lock != nullptr;
}

const std::string& winss::PathMutex::GetName() const {
    return mutex_name;
}

winss::PathMutex::~PathMutex() {
    if (HasLock()) {
        WINDOWS.ReleaseMutex(lock);
        WINDOWS.CloseHandle(lock);
        lock = nullptr;
    }
}
