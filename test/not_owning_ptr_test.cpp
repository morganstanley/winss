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

#include <string>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "winss/winss.hpp"
#include "winss/not_owning_ptr.hpp"

using ::testing::_;
using ::testing::NiceMock;

namespace winss {
class NotOwningPtrTest : public testing::Test {
};

TEST_F(NotOwningPtrTest, Get) {
    std::string test = "test";

    auto not_owned_test = winss::NotOwned(&test);

    auto ptr = not_owned_test.Get();
    const auto const_ptr = not_owned_test.Get();

    EXPECT_EQ(&test, ptr);
    EXPECT_EQ(&test, const_ptr);
}

TEST_F(NotOwningPtrTest, Deref) {
    std::string test = "test";

    auto not_owned_test = winss::NotOwned(&test);

    auto str = *not_owned_test;
    const auto const_str = *not_owned_test;

    EXPECT_EQ(test, str);
    EXPECT_EQ(test, const_str);
}

TEST_F(NotOwningPtrTest, Arrow) {
    std::string test = "test";

    auto not_owned_test = winss::NotOwned(&test);

    not_owned_test->append("1");

    EXPECT_EQ("test1", test);
    EXPECT_EQ(test.size(), not_owned_test->size());
}

TEST_F(NotOwningPtrTest, Equality) {
    std::string test1 = "test1";
    std::string test2 = "test2";

    auto not_owned_test1 = winss::NotOwned(&test1);
    auto not_owned_test2 = winss::NotOwned(&test1);
    auto not_owned_test3 = winss::NotOwned(&test2);

    EXPECT_EQ(not_owned_test1, not_owned_test2);
    EXPECT_NE(not_owned_test1, not_owned_test3);
}

TEST_F(NotOwningPtrTest, Copy) {
    std::string test1 = "test1";
    std::string test2 = "test2";

    auto not_owned_test1 = winss::NotOwned(&test1);

    auto not_owned_test2 = winss::NotOwningPtr<std::string>(not_owned_test1);

    auto not_owned_test3 = winss::NotOwned(&test2);

    not_owned_test3 = not_owned_test2;

    EXPECT_EQ(not_owned_test1, not_owned_test2);
    EXPECT_EQ(not_owned_test1, not_owned_test3);
}
}  // namespace winss
