#include <filesystem>
#include "winss/winss.hpp"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "winss/svscan/service_process.hpp"
#include "../mock_process.hpp"

namespace fs = std::experimental::filesystem;

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace winss {
class ServiceProcessTest : public testing::Test {
};
class MockedServiceProcess :
    public winss::ServiceProcessTmpl<winss::NiceMockProcess> {
 public:
    MockedServiceProcess(const fs::path& service_dir, bool is_log) :
        winss::ServiceProcessTmpl<winss::NiceMockProcess>
        ::ServiceProcessTmpl(service_dir, is_log) {}

    MockedServiceProcess(const MockedServiceProcess&) = delete;

    MockedServiceProcess(MockedServiceProcess&& p) :
        winss::ServiceProcessTmpl<winss::NiceMockProcess>
        ::ServiceProcessTmpl(std::move(p)) {}

    winss::MockProcess* GetProcess() {
        return &proc;
    }

    void SetFlagged(bool flagged) {
        this->flagged = flagged;
    }

    void operator=(const MockedServiceProcess&) = delete;

    MockedServiceProcess& operator=(MockedServiceProcess&& p) {
        winss::ServiceProcessTmpl<NiceMockProcess>::operator=(std::move(p));
        return *this;
    }
};

TEST_F(ServiceProcessTest, Start) {
    MockedServiceProcess service_process(".", false);

    EXPECT_CALL(*service_process.GetProcess(), Create(_))
        .WillOnce(Return(true));
    EXPECT_CALL(*service_process.GetProcess(), IsCreated())
        .WillOnce(Return(false))
        .WillOnce(Return(true));

    EXPECT_FALSE(service_process.IsFlagged());
    service_process.Start(winss::ServicePipes{});
    EXPECT_TRUE(service_process.IsFlagged());
    service_process.Start(winss::ServicePipes{});
    EXPECT_TRUE(service_process.IsFlagged());
}

TEST_F(ServiceProcessTest, StartLog) {
    MockedServiceProcess service_process(".", true);

    EXPECT_CALL(*service_process.GetProcess(), Create(_))
        .WillOnce(Return(true));
    EXPECT_CALL(*service_process.GetProcess(), IsCreated())
        .WillOnce(Return(false))
        .WillOnce(Return(true));

    EXPECT_FALSE(service_process.IsFlagged());
    service_process.Start(winss::ServicePipes{
        winss::HandleWrapper(reinterpret_cast<HANDLE>(10000), false),
        winss::HandleWrapper(reinterpret_cast<HANDLE>(20000), false)
    });
    EXPECT_TRUE(service_process.IsFlagged());
    service_process.Start(winss::ServicePipes{});
    EXPECT_TRUE(service_process.IsFlagged());
}

TEST_F(ServiceProcessTest, Close) {
    MockedServiceProcess service_process(".", true);

    EXPECT_CALL(*service_process.GetProcess(), SendBreak()).Times(2);
    EXPECT_CALL(*service_process.GetProcess(), IsCreated())
        .WillRepeatedly(Return(true));

    EXPECT_FALSE(service_process.Close(false));
    service_process.SetFlagged(true);
    EXPECT_TRUE(service_process.Close(false));
    EXPECT_FALSE(service_process.Close(true));
}

TEST_F(ServiceProcessTest, Move) {
    MockedServiceProcess service_process1("C:\\1", true);
    MockedServiceProcess service_process2("C:\\2", false);

    EXPECT_FALSE(service_process2.IsLog());
    EXPECT_EQ(fs::path("C:\\2"), service_process2.GetServiceDir());
    service_process2 = std::move(service_process1);
    EXPECT_TRUE(service_process2.IsLog());
    EXPECT_EQ(fs::path("C:\\1"), service_process2.GetServiceDir());

    MockedServiceProcess service_process3(std::move(service_process2));
    EXPECT_TRUE(service_process3.IsLog());
    EXPECT_EQ(fs::path("C:\\1"), service_process3.GetServiceDir());
}
}  // namespace winss
