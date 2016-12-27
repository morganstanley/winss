#ifndef TEST_SUPERVISE_MOCK_STATE_FILE_HPP_
#define TEST_SUPERVISE_MOCK_STATE_FILE_HPP_

#include <windows.h>
#include <gmock/gmock.h>
#include <winss/supervise/state_file.hpp>
#include <winss/supervise/supervise.hpp>
#include <filesystem>
#include <string>

namespace fs = std::experimental::filesystem;

namespace winss {
class MockSuperviseStateFile : public winss::SuperviseStateFile {
 public:
    explicit MockSuperviseStateFile(fs::path service_dir) :
        winss::SuperviseStateFile::SuperviseStateFile(service_dir) {}

    MockSuperviseStateFile(const MockSuperviseStateFile&) = delete;
    MockSuperviseStateFile(MockSuperviseStateFile&&) = delete;

    MOCK_CONST_METHOD0(GetPath, const fs::path&());
    MOCK_METHOD2(Notify, bool(winss::SuperviseNotification notification,
        const winss::SuperviseState& state));
    MOCK_CONST_METHOD1(Read, bool(winss::SuperviseState* state));
    MOCK_CONST_METHOD1(Format, std::string(winss::SuperviseState* state));

    void operator=(const MockSuperviseStateFile&) = delete;
    MockSuperviseStateFile& operator=(MockSuperviseStateFile&&) = delete;
};
}  // namespace winss

#endif  // TEST_SUPERVISE_MOCK_STATE_FILE_HPP_
