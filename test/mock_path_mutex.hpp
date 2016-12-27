#ifndef TEST_MOCK_PATH_MUTEX_HPP_
#define TEST_MOCK_PATH_MUTEX_HPP_

#include <windows.h>
#include <gmock/gmock.h>
#include <winss/path_mutex.hpp>
#include <filesystem>
#include <string>

namespace fs = std::experimental::filesystem;

namespace winss {
class MockPathMutex : winss::PathMutex {
 public:
    MockPathMutex(fs::path path, std::string name) {}

    MOCK_METHOD0(Lock, bool());

    MOCK_CONST_METHOD0(CanLock, bool());
    MOCK_CONST_METHOD0(HasLock, bool());
    MOCK_CONST_METHOD0(GetName, const std::string&());
};
}  // namespace winss

#endif  // TEST_MOCK_PATH_MUTEX_HPP_
