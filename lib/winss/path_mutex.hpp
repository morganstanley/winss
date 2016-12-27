#ifndef LIB_WINSS_PATH_MUTEX_HPP_
#define LIB_WINSS_PATH_MUTEX_HPP_

#include <windows.h>
#include <filesystem>
#include <string>

namespace fs = std::experimental::filesystem;

namespace winss {
class PathMutex {
 protected:
    std::string mutex_name;
    HANDLE lock = nullptr;

    PathMutex() {}

 public:
    PathMutex(fs::path path, std::string name);
    PathMutex(const PathMutex&) = delete;
    PathMutex(PathMutex&&) = delete;

    virtual bool Lock();

    virtual bool CanLock() const;
    virtual bool HasLock() const;
    virtual const std::string& GetName() const;

    void operator=(const PathMutex&) = delete;
    PathMutex& operator=(PathMutex&&) = delete;

    virtual ~PathMutex();
};
}  // namespace winss

#endif  // LIB_WINSS_PATH_MUTEX_HPP_
