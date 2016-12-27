#include <winss/winss.hpp>
#include <wincrypt.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <winss/sha256.hpp>
#include <algorithm>
#include <string>
#include "mock_interface.hpp"
#include "mock_windows_interface.hpp"

using ::testing::_;
using ::testing::NiceMock;

namespace winss {
class SHA256Test : public testing::Test {
};

TEST_F(SHA256Test, CalculateDigest) {
    MockInterface<winss::MockWindowsInterface> windows;
    windows->SetupDefaults();

    EXPECT_CALL(*windows, CryptReleaseContext(_, _)).Times(1);
    EXPECT_CALL(*windows, CryptDestroyHash(_)).Times(1);

    std::string hash = winss::SHA256::CalculateDigest("this is a test");
    std::transform(hash.begin(), hash.end(), hash.begin(), ::tolower);
    EXPECT_EQ(
        "2e99758548972a8e8822ad47fa1017ff72f06f3ff6a016851f45c398732bc50c",
        hash);
}
}  // namespace winss
