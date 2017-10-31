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

#include <vector>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "winss/winss.hpp"
#include "winss/handle_wrapper.hpp"
#include "mock_interface.hpp"
#include "mock_windows_interface.hpp"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace winss {
class HandleWrapperTest : public testing::Test {
};

TEST_F(HandleWrapperTest, CreateEmpty) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CloseHandle(_)).Times(0);

    winss::HandleWrapper handle;

    EXPECT_FALSE(handle.HasHandle());
    EXPECT_FALSE(handle.IsOwner());
    EXPECT_EQ(0, handle.GetDuplicateRights());
}

TEST_F(HandleWrapperTest, Owned) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CloseHandle(_)).Times(1);

    winss::HandleWrapper handle(reinterpret_cast<HANDLE>(10000), true);

    EXPECT_TRUE(handle.HasHandle());
    EXPECT_TRUE(handle.IsOwner());
    EXPECT_EQ(0, handle.GetDuplicateRights());
}

TEST_F(HandleWrapperTest, NotOwned) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CloseHandle(_)).Times(0);

    winss::HandleWrapper handle(reinterpret_cast<HANDLE>(10000), false);

    EXPECT_TRUE(handle.HasHandle());
    EXPECT_FALSE(handle.IsOwner());
    EXPECT_EQ(0, handle.GetDuplicateRights());
}

TEST_F(HandleWrapperTest, Copy) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CloseHandle(_)).Times(1);

    winss::HandleWrapper handle(reinterpret_cast<HANDLE>(10000), true, 1);
    winss::HandleWrapper handle2;

    EXPECT_FALSE(handle2.HasHandle());
    handle2 = handle;

    EXPECT_TRUE(handle.HasHandle());
    EXPECT_TRUE(handle.IsOwner());
    EXPECT_EQ(1, handle.GetDuplicateRights());
    EXPECT_TRUE(handle2.HasHandle());
    EXPECT_FALSE(handle2.IsOwner());
    EXPECT_EQ(1, handle2.GetDuplicateRights());

    winss::HandleWrapper handle3(handle2);
    EXPECT_TRUE(handle3.HasHandle());
    EXPECT_FALSE(handle3.IsOwner());
    EXPECT_EQ(1, handle3.GetDuplicateRights());
}

TEST_F(HandleWrapperTest, Move) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CloseHandle(_)).Times(1);

    winss::HandleWrapper handle(reinterpret_cast<HANDLE>(10000), true, 1);
    winss::HandleWrapper handle2;

    EXPECT_FALSE(handle2.HasHandle());
    handle2 = std::move(handle);

    EXPECT_TRUE(handle.HasHandle());
    EXPECT_FALSE(handle.IsOwner());
    EXPECT_EQ(1, handle.GetDuplicateRights());
    EXPECT_TRUE(handle2.HasHandle());
    EXPECT_TRUE(handle2.IsOwner());
    EXPECT_EQ(1, handle2.GetDuplicateRights());

    winss::HandleWrapper handle3(std::move(handle2));
    EXPECT_TRUE(handle3.HasHandle());
    EXPECT_TRUE(handle3.IsOwner());
    EXPECT_EQ(1, handle3.GetDuplicateRights());
}

TEST_F(HandleWrapperTest, Wait) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, WaitForMultipleObjects(1, _, _, INFINITE))
        .WillOnce(Return(WAIT_OBJECT_0));

    winss::HandleWrapper handle(reinterpret_cast<HANDLE>(10000), true);

    auto result = handle.Wait(INFINITE);
    EXPECT_EQ(SUCCESS, result.state);
    EXPECT_EQ(handle, result.handle);
}

TEST_F(HandleWrapperTest, TIMEOUT) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, WaitForMultipleObjects(1, _, _, 500))
        .WillOnce(Return(WAIT_TIMEOUT));

    winss::HandleWrapper handle(reinterpret_cast<HANDLE>(10000), true);

    auto result = handle.Wait(500);
    EXPECT_EQ(TIMEOUT, result.state);
    EXPECT_NE(handle, result.handle);
}

TEST_F(HandleWrapperTest, WaitInvalid) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, WaitForMultipleObjects(1, _, _, INFINITE))
        .WillOnce(Return(WAIT_FAILED));

    winss::HandleWrapper handle(reinterpret_cast<HANDLE>(10000), true);

    auto result = handle.Wait(INFINITE);
    EXPECT_EQ(FAILED, result.state);
    EXPECT_NE(handle, result.handle);
}

TEST_F(HandleWrapperTest, WaitMany) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, WaitForMultipleObjects(2, _, _, 500))
        .WillOnce(Return(WAIT_OBJECT_0 + 1));

    winss::HandleWrapper handle1(reinterpret_cast<HANDLE>(10000), true);
    winss::HandleWrapper handle2(reinterpret_cast<HANDLE>(20000), true);

    std::vector<winss::HandleWrapper> handles = { handle1, handle2 };
    auto result = winss::HandleWrapper::Wait(500,
        handles.begin(), handles.end());

    EXPECT_EQ(SUCCESS, result.state);
    EXPECT_EQ(handle2, result.handle);
}

TEST_F(HandleWrapperTest, Duplicate) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows,
        DuplicateHandle(_, _, _, _, 0, false, DUPLICATE_SAME_ACCESS)).Times(1);

    winss::HandleWrapper handle(reinterpret_cast<HANDLE>(10000), true);

    handle.Duplicate(false);
}

TEST_F(HandleWrapperTest, DuplicateSecurity) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows,
        DuplicateHandle(_, _, _, _, 1, true, 0)).Times(1);

    winss::HandleWrapper handle(reinterpret_cast<HANDLE>(10000), true, 1);

    handle.Duplicate(true);
}

TEST_F(HandleWrapperTest, Equality) {
    MockInterface<winss::MockWindowsInterface> windows;

    HANDLE handle1 = reinterpret_cast<HANDLE>(10000);
    winss::HandleWrapper wrapper1(handle1);
    winss::HandleWrapper wrapper1copy = wrapper1;

    HANDLE handle2 = reinterpret_cast<HANDLE>(20000);
    winss::HandleWrapper wrapper2(handle2);

    EXPECT_EQ(wrapper1, handle1);
    EXPECT_EQ(handle1, wrapper1);
    EXPECT_EQ(wrapper1copy, wrapper1);
    EXPECT_EQ(wrapper1, wrapper1copy);
    EXPECT_EQ(wrapper1copy, handle1);
    EXPECT_EQ(handle1, wrapper1copy);

    EXPECT_NE(wrapper1, handle2);
    EXPECT_NE(handle2, wrapper1);
    EXPECT_NE(wrapper1, wrapper2);
    EXPECT_NE(wrapper2, wrapper1);
}

TEST_F(HandleWrapperTest, Relational) {
    MockInterface<winss::MockWindowsInterface> windows;

    HANDLE handle1 = reinterpret_cast<HANDLE>(10000);
    winss::HandleWrapper wrapper1(handle1);

    HANDLE handle2 = reinterpret_cast<HANDLE>(20000);
    winss::HandleWrapper wrapper2(handle2);

    EXPECT_GT(wrapper2, wrapper1);
    EXPECT_GT(wrapper2, handle1);
    EXPECT_GT(handle2, wrapper1);
    EXPECT_GE(wrapper2, wrapper2);
    EXPECT_GE(wrapper2, handle2);
    EXPECT_GE(handle2, wrapper2);
    EXPECT_LT(wrapper1, wrapper2);
    EXPECT_LT(wrapper1, handle2);
    EXPECT_LT(handle1, wrapper2);
    EXPECT_LE(wrapper1, handle1);
    EXPECT_LE(wrapper1, wrapper1);
    EXPECT_LE(handle1, wrapper1);
}

TEST_F(HandleWrapperTest, SelfAssignment) {
    MockInterface<winss::MockWindowsInterface> windows;

    HANDLE handle1 = reinterpret_cast<HANDLE>(10000);
    winss::HandleWrapper wrapper1(handle1);

    wrapper1 = wrapper1;
    wrapper1 = std::move(wrapper1);

    EXPECT_TRUE(wrapper1.IsOwner());
}

TEST_F(HandleWrapperTest, CloseOnAssignment) {
    MockInterface<winss::MockWindowsInterface> windows;

    HANDLE handle1 = reinterpret_cast<HANDLE>(10000);
    winss::HandleWrapper wrapper1(handle1);

    HANDLE handle2 = reinterpret_cast<HANDLE>(20000);
    winss::HandleWrapper wrapper2(handle2);

    HANDLE handle3 = reinterpret_cast<HANDLE>(30000);
    winss::HandleWrapper wrapper3(handle3);

    EXPECT_CALL(*windows, CloseHandle(_)).Times(3);

    wrapper1 = wrapper2;
    wrapper1 = wrapper3;
}

TEST_F(HandleWrapperTest, Trusted) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CloseHandle(_)).Times(1);

    HANDLE handle = reinterpret_cast<HANDLE>(10000);
    winss::TrustedHandleWrapper trusted_wrapper(handle);

    EXPECT_EQ(handle, trusted_wrapper.GetHandle());
    // Ensure CloseHandle is not called on handle wrapper
    winss::HandleWrapper wrapper = trusted_wrapper.GetHandleWrapper();

    EXPECT_FALSE(wrapper.IsOwner());
}

TEST_F(HandleWrapperTest, TrustedCopy) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CloseHandle(_)).Times(1);

    winss::TrustedHandleWrapper handle(reinterpret_cast<HANDLE>(10000), 1);
    winss::TrustedHandleWrapper handle2;

    EXPECT_FALSE(handle2.HasHandle());
    handle2 = handle;

    EXPECT_TRUE(handle.HasHandle());
    EXPECT_TRUE(handle.IsOwner());
    EXPECT_EQ(1, handle.GetDuplicateRights());
    EXPECT_TRUE(handle2.HasHandle());
    EXPECT_FALSE(handle2.IsOwner());
    EXPECT_EQ(1, handle2.GetDuplicateRights());

    winss::TrustedHandleWrapper handle3(handle2);
    EXPECT_TRUE(handle3.HasHandle());
    EXPECT_FALSE(handle3.IsOwner());
    EXPECT_EQ(1, handle3.GetDuplicateRights());
}

TEST_F(HandleWrapperTest, TrustedMove) {
    MockInterface<winss::MockWindowsInterface> windows;

    EXPECT_CALL(*windows, CloseHandle(_)).Times(1);

    winss::TrustedHandleWrapper handle(reinterpret_cast<HANDLE>(10000), 1);
    winss::TrustedHandleWrapper handle2;

    EXPECT_FALSE(handle2.HasHandle());
    handle2 = std::move(handle);

    EXPECT_TRUE(handle.HasHandle());
    EXPECT_FALSE(handle.IsOwner());
    EXPECT_EQ(1, handle.GetDuplicateRights());
    EXPECT_TRUE(handle2.HasHandle());
    EXPECT_TRUE(handle2.IsOwner());
    EXPECT_EQ(1, handle2.GetDuplicateRights());

    winss::TrustedHandleWrapper handle3(std::move(handle2));
    EXPECT_TRUE(handle3.HasHandle());
    EXPECT_TRUE(handle3.IsOwner());
    EXPECT_EQ(1, handle3.GetDuplicateRights());
}

TEST_F(HandleWrapperTest, TrustedEquality) {
    MockInterface<winss::MockWindowsInterface> windows;

    HANDLE handle1 = reinterpret_cast<HANDLE>(10000);
    winss::TrustedHandleWrapper wrapper1(handle1);
    winss::TrustedHandleWrapper wrapper1copy = wrapper1;

    HANDLE handle2 = reinterpret_cast<HANDLE>(20000);
    winss::TrustedHandleWrapper wrapper2(handle2);

    EXPECT_EQ(wrapper1, handle1);
    EXPECT_EQ(handle1, wrapper1);
    EXPECT_EQ(wrapper1copy, wrapper1);
    EXPECT_EQ(wrapper1, wrapper1copy);
    EXPECT_EQ(wrapper1copy, handle1);
    EXPECT_EQ(handle1, wrapper1copy);

    EXPECT_NE(wrapper1, handle2);
    EXPECT_NE(handle2, wrapper1);
    EXPECT_NE(wrapper1, wrapper2);
    EXPECT_NE(wrapper2, wrapper1);
}

TEST_F(HandleWrapperTest, TrustedRelational) {
    MockInterface<winss::MockWindowsInterface> windows;

    HANDLE handle1 = reinterpret_cast<HANDLE>(10000);
    winss::TrustedHandleWrapper wrapper1(handle1);

    HANDLE handle2 = reinterpret_cast<HANDLE>(20000);
    winss::TrustedHandleWrapper wrapper2(handle2);

    EXPECT_GT(wrapper2, wrapper1);
    EXPECT_GT(wrapper2, handle1);
    EXPECT_GT(handle2, wrapper1);
    EXPECT_GE(wrapper2, wrapper2);
    EXPECT_GE(wrapper2, handle2);
    EXPECT_GE(handle2, wrapper2);
    EXPECT_LT(wrapper1, wrapper2);
    EXPECT_LT(wrapper1, handle2);
    EXPECT_LT(handle1, wrapper2);
    EXPECT_LE(wrapper1, handle1);
    EXPECT_LE(wrapper1, wrapper1);
    EXPECT_LE(handle1, wrapper1);
}

TEST_F(HandleWrapperTest, TrustedSelfAssignment) {
    MockInterface<winss::MockWindowsInterface> windows;

    HANDLE handle1 = reinterpret_cast<HANDLE>(10000);
    winss::TrustedHandleWrapper wrapper1(handle1);

    wrapper1 = wrapper1;
    wrapper1 = std::move(wrapper1);

    EXPECT_TRUE(wrapper1.IsOwner());
}

TEST_F(HandleWrapperTest, TrustedCloseOnAssignment) {
    MockInterface<winss::MockWindowsInterface> windows;

    HANDLE handle1 = reinterpret_cast<HANDLE>(10000);
    winss::TrustedHandleWrapper wrapper1(handle1);

    HANDLE handle2 = reinterpret_cast<HANDLE>(20000);
    winss::TrustedHandleWrapper wrapper2(handle2);

    HANDLE handle3 = reinterpret_cast<HANDLE>(30000);
    winss::TrustedHandleWrapper wrapper3(handle3);

    EXPECT_CALL(*windows, CloseHandle(_)).Times(3);

    wrapper1 = wrapper2;
    wrapper1 = wrapper3;
}
}  // namespace winss
