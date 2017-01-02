#ifndef LIB_WINSS_FILESYSTEM_INTERFACE_HPP_
#define LIB_WINSS_FILESYSTEM_INTERFACE_HPP_

#include <windows.h>
#include <filesystem>
#include <vector>
#include <memory>
#include <string>

#define FILESYSTEM winss::FilesystemInterface::GetInstance()

namespace fs = std::experimental::filesystem;

namespace winss {
class FilesystemInterface {
 protected:
    static std::shared_ptr<FilesystemInterface> instance;

 public:
    FilesystemInterface() {}
    FilesystemInterface(const FilesystemInterface&) = delete;
    FilesystemInterface(FilesystemInterface&&) = delete;

    virtual std::string Read(const fs::path& path) const;
    virtual bool Write(const fs::path& path, const std::string& content) const;
    virtual bool ChangeDirectory(const fs::path& dir) const;
    virtual bool DirectoryExists(const fs::path& dir) const;
    virtual bool CreateDirectory(const fs::path& dir) const;
    virtual bool Rename(const fs::path& from, const fs::path& to) const;
    virtual bool Remove(const fs::path& path) const;
    virtual bool FileExists(const fs::path& path) const;
    virtual fs::path Absolute(const fs::path& path) const;
    virtual std::vector<fs::path> GetDirectories(const fs::path& path) const;
    virtual std::vector<fs::path> GetFiles(const fs::path& path) const;

    static const FilesystemInterface& GetInstance();

    void operator=(const FilesystemInterface&) = delete;
    FilesystemInterface& operator=(FilesystemInterface&&) = delete;
};
}  // namespace winss

#endif  // LIB_WINSS_FILESYSTEM_INTERFACE_HPP_
