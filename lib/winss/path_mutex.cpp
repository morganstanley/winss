#include "path_mutex.hpp"
#include <windows.h>
#include <easylogging/easylogging++.hpp>
#include <string>
#include "windows_interface.hpp"
#include "filesystem_interface.hpp"
#include "sha256.hpp"

namespace fs = std::experimental::filesystem;

winss::PathMutex::PathMutex(fs::path path, std::string name) {
    path = FILESYSTEM.Absolute(path);
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
