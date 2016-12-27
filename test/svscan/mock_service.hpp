#ifndef TEST_SVSCAN_MOCK_SERVICE_HPP_
#define TEST_SVSCAN_MOCK_SERVICE_HPP_

#include <windows.h>
#include <gmock/gmock.h>
#include <winss/svscan/service.hpp>
#include <filesystem>
#include <string>
#include <utility>
#include "mock_service_process.hpp"

namespace fs = std::experimental::filesystem;

using ::testing::NiceMock;
using ::testing::ReturnRef;

namespace winss {
class MockService : public virtual winss::Service {
 public:
    MockService() {
        ON_CALL(*this, GetName()).WillByDefault(ReturnRef(name));
    }

    MockService(std::string name, const fs::path& service_dir) :
        winss::Service::ServiceTmpl(name, service_dir) {}

    MockService(const MockService&) = delete;

    MockService(MockService&& s) :
        winss::Service::ServiceTmpl(std::move(s)) {}

    MOCK_CONST_METHOD0(GetName, const std::string&());
    MOCK_METHOD0(Reset, void());
    MOCK_METHOD0(Check, void());
    MOCK_METHOD1(Close, bool(bool ignore_flagged));

    void operator=(const MockService&) = delete;

    MockService& operator=(MockService&& p) {
        winss::Service::operator=(std::move(p));
        return *this;
    }
};
class NiceMockService : public NiceMock<MockService> {
 public:
    NiceMockService() {}

    NiceMockService(std::string name, const fs::path& service_dir) :
        winss::Service::ServiceTmpl(name, service_dir) {}

    NiceMockService(const NiceMockService&) = delete;

    NiceMockService(NiceMockService&& p) :
        winss::Service::ServiceTmpl(std::move(p)) {}

    void operator=(const NiceMockService&) = delete;

    NiceMockService& operator=(NiceMockService&& p) {
        winss::Service::operator=(std::move(p));
        return *this;
    }
};
}  // namespace winss

#endif  // TEST_SVSCAN_MOCK_SERVICE_HPP_
