#include <filesystem>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "winss/winss.hpp"
#include "winss/svscan/service.hpp"
#include "../mock_interface.hpp"
#include "../mock_windows_interface.hpp"
#include "../mock_filesystem_interface.hpp"
#include "mock_service_process.hpp"

namespace fs = std::experimental::filesystem;

using ::testing::_;
using ::testing::NiceMock;
using ::testing::DoAll;
using ::testing::SetArgPointee;
using ::testing::Return;
using ::testing::ReturnRef;

namespace winss {
class ServiceTest : public testing::Test {
};
class MockedService :
    public winss::ServiceTmpl<winss::NiceMockServiceProcess> {
 public:
    MockedService(std::string name, const fs::path& service_dir) :
        winss::ServiceTmpl<winss::NiceMockServiceProcess>
        ::ServiceTmpl(name, service_dir) {}

    MockedService(const MockedService&) = delete;

    MockedService(MockedService&& s) :
        winss::ServiceTmpl<winss::NiceMockServiceProcess>
        ::ServiceTmpl(std::move(s)) {}

    winss::NiceMockServiceProcess* GetMain() {
        return &main;
    }

    winss::NiceMockServiceProcess* GetLog() {
        return &log;
    }

    void operator=(const MockedService&) = delete;

    MockedService& operator=(MockedService&& p) {
        winss::ServiceTmpl<winss::NiceMockServiceProcess>
            ::operator=(std::move(p));
        return *this;
    }
};

TEST_F(ServiceTest, Check) {
    MockInterface<winss::MockFilesystemInterface> file;

    EXPECT_CALL(*file, DirectoryExists(_))
        .WillOnce(Return(false));

    MockedService service("test", ".");

    EXPECT_CALL(*service.GetMain(), Start(_)).Times(1);
    EXPECT_CALL(*service.GetLog(), Start(_)).Times(0);
    fs::path log(".\\log");
    EXPECT_CALL(*service.GetLog(), GetServiceDir())
        .WillOnce(ReturnRef(log));

    service.Check();

    EXPECT_EQ("test", service.GetName());
}

TEST_F(ServiceTest, CheckLog) {
    MockInterface<winss::MockWindowsInterface> windows;
    MockInterface<winss::MockFilesystemInterface> file;

    EXPECT_CALL(*windows, CreatePipe(_, _, _, _))
        .WillOnce(DoAll(SetArgPointee<0>(reinterpret_cast<HANDLE>(10000)),
            SetArgPointee<1>(reinterpret_cast<HANDLE>(20000)), Return(true)));

    EXPECT_CALL(*file, DirectoryExists(_))
        .WillOnce(Return(true));

    MockedService service("test", ".");

    EXPECT_CALL(*service.GetMain(), Start(_)).Times(1);
    EXPECT_CALL(*service.GetLog(), Start(_)).Times(1);
    fs::path log(".\\log");
    EXPECT_CALL(*service.GetLog(), GetServiceDir())
        .WillOnce(ReturnRef(log));

    service.Check();

    EXPECT_EQ("test", service.GetName());
}

TEST_F(ServiceTest, Reset) {
    MockedService service("test", ".");

    EXPECT_CALL(*service.GetMain(), Reset()).Times(1);
    EXPECT_CALL(*service.GetLog(), Reset()).Times(1);

    service.Reset();
}

TEST_F(ServiceTest, Close) {
    MockedService service("test", ".");

    EXPECT_CALL(*service.GetMain(), Close(false)).WillOnce(Return(true));
    EXPECT_CALL(*service.GetLog(), Close(false)).WillOnce(Return(true));

    EXPECT_TRUE(service.Close(false));
}

TEST_F(ServiceTest, CloseFlagged) {
    MockedService service("test", ".");

    EXPECT_CALL(*service.GetMain(), Close(false)).WillOnce(Return(false));
    EXPECT_CALL(*service.GetLog(), Close(true)).WillOnce(Return(false));

    EXPECT_FALSE(service.Close(false));
}

TEST_F(ServiceTest, CloseIgnore) {
    MockedService service("test", ".");

    EXPECT_CALL(*service.GetMain(), Close(true)).WillOnce(Return(false));
    EXPECT_CALL(*service.GetLog(), Close(true)).WillOnce(Return(false));

    EXPECT_FALSE(service.Close(true));
}

TEST_F(ServiceTest, Move) {
    MockedService service1("test1", ".\\1");
    MockedService service2("test2", ".\\2");

    service2 = std::move(service1);
    EXPECT_EQ("test1", service2.GetName());

    MockedService service3(std::move(service2));
    EXPECT_EQ("test1", service3.GetName());
}
}  // namespace winss
