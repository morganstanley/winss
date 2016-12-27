#ifndef TEST_MOCK_FILESYSTEM_INTERFACE_HPP_
#define TEST_MOCK_FILESYSTEM_INTERFACE_HPP_

#include <gmock/gmock.h>
#include <winss/filesystem_interface.hpp>
#include <filesystem>
#include <string>
#include <vector>
#include <memory>

namespace fs = std::experimental::filesystem;

namespace winss {
class MockFilesystemInterface : public winss::FilesystemInterface {
 public:
    using winss::FilesystemInterface::instance;

    MockFilesystemInterface() {}
    MockFilesystemInterface(const MockFilesystemInterface&) = delete;
    MockFilesystemInterface(MockFilesystemInterface&&) = delete;

    MOCK_CONST_METHOD1(Read, std::string(const fs::path& path));
    MOCK_CONST_METHOD2(Write, bool(const fs::path& path,
        const std::string& content));
    MOCK_CONST_METHOD1(ChangeDirectory, bool(const fs::path& dir));
    MOCK_CONST_METHOD1(DirectoryExists, bool(const fs::path& dir));
    MOCK_CONST_METHOD1(CreateDirectory, bool(const fs::path& dir));
    MOCK_CONST_METHOD2(Rename, bool(const fs::path& fro1, const fs::path& to));
    MOCK_CONST_METHOD1(Remove, bool(const fs::path& path));
    MOCK_CONST_METHOD1(FileExists, bool(const fs::path& path));
    MOCK_CONST_METHOD1(Absolute, fs::path(const fs::path& path));
    MOCK_CONST_METHOD1(GetDirectories, std::vector<fs::path>(
        const fs::path& path));
    MOCK_CONST_METHOD1(GetFiles, std::vector<fs::path>(
        const fs::path& path));

    void operator=(const MockFilesystemInterface&) = delete;
    MockFilesystemInterface& operator=(MockFilesystemInterface&&) = delete;
};
}  // namespace winss

#endif  // TEST_MOCK_FILESYSTEM_INTERFACE_HPP_
