#ifndef TEST_SVSCAN_MOCK_SERVICE_PROCESS_HPP_
#define TEST_SVSCAN_MOCK_SERVICE_PROCESS_HPP_

#include <windows.h>
#include <gmock/gmock.h>
#include <winss/svscan/service_process.hpp>
#include <filesystem>
#include <utility>
#include "../mock_process.hpp"

namespace fs = std::experimental::filesystem;

using ::testing::NiceMock;
using ::testing::ReturnRef;

namespace winss {
class MockServiceProcess : public virtual winss::ServiceProcess {
 public:
    MockServiceProcess() {
        ON_CALL(*this, GetServiceDir()).WillByDefault(ReturnRef(service_dir));
    }

    MockServiceProcess(const fs::path& service_dir, bool is_log) :
        winss::ServiceProcess::ServiceProcessTmpl(service_dir, is_log) {}

    MockServiceProcess(const MockServiceProcess&) = delete;

    MockServiceProcess(MockServiceProcess&& p) :
        winss::ServiceProcess::ServiceProcessTmpl(std::move(p)) {}

    MOCK_CONST_METHOD0(IsFlagged, bool());
    MOCK_CONST_METHOD0(GetServiceDir, const fs::path&());
    MOCK_CONST_METHOD0(Reset, void());

    MOCK_METHOD1(Start, void(const winss::ServicePipes& pipes));
    MOCK_METHOD1(Close, bool(bool ignore_flagged));

    void operator=(const MockServiceProcess&) = delete;

    MockServiceProcess& operator=(MockServiceProcess&& p) {
        winss::ServiceProcess::operator=(std::move(p));
        return *this;
    }
};
class NiceMockServiceProcess : public NiceMock<MockServiceProcess> {
 public:
    NiceMockServiceProcess() {}

    NiceMockServiceProcess(const fs::path& service_dir, bool is_log) :
        winss::ServiceProcess::ServiceProcessTmpl(service_dir, is_log) {}

    NiceMockServiceProcess(const NiceMockServiceProcess&) = delete;

    NiceMockServiceProcess(NiceMockServiceProcess&& p) :
        winss::ServiceProcess::ServiceProcessTmpl(std::move(p)) {}

    void operator=(const NiceMockServiceProcess&) = delete;

    NiceMockServiceProcess& operator=(NiceMockServiceProcess&& p) {
        winss::ServiceProcess::operator=(std::move(p));
        return *this;
    }
};
}  // namespace winss

#endif  // TEST_SVSCAN_MOCK_SERVICE_PROCESS_HPP_
