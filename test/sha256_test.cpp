/*
* Copyright 2016-2017 Morgan Stanley
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <windows.h>
#include <wincrypt.h>
#include <algorithm>
#include <string>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "winss/winss.hpp"
#include <winss/sha256.hpp>
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
