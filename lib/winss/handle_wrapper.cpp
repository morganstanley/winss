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

#include "handle_wrapper.hpp"
#include <windows.h>
#include <vector>
#include "easylogging/easylogging++.hpp"
#include "windows_interface.hpp"

winss::HandleWrapper::HandleWrapper() :
    owned(false), handle(nullptr), dup_rights(0) {}

winss::HandleWrapper::HandleWrapper(HANDLE handle, bool owned,
    DWORD dup_rights) : owned(owned), handle(handle), dup_rights(dup_rights) {}

winss::HandleWrapper::HandleWrapper(const winss::HandleWrapper& h) :
    owned(false), handle(h.handle), dup_rights(h.dup_rights) {}

winss::HandleWrapper::HandleWrapper(winss::HandleWrapper&& h) :
    owned(h.owned), handle(h.handle), dup_rights(h.dup_rights) {
    h.owned = false;
}

bool winss::HandleWrapper::HasHandle() const {
    return handle != nullptr;
}

bool winss::HandleWrapper::IsOwner() const {
    return owned;
}

DWORD winss::HandleWrapper::GetDuplicateRights() const {
    return dup_rights;
}

winss::WaitResult winss::HandleWrapper::Wait(DWORD timeout,
    const std::vector<HANDLE>& handles) {
    DWORD size = static_cast<DWORD>(handles.size());
    DWORD result_code = WINDOWS.WaitForMultipleObjects(size,
        handles.data(), false, timeout);

    if (result_code == WAIT_TIMEOUT) {
        VLOG(7) << "HANDLE Wait timeout";
        return { TIMEOUT };
    } else if (result_code == WAIT_FAILED) {
        VLOG(1) << "HANDLE Wait failed: " << WINDOWS.GetLastError();
    } else {
        int index = result_code - WAIT_OBJECT_0;
        try {
            HANDLE handle = handles.at(index);
            VLOG(7) << "HANDLE " << handle << "/" << size << " fired";
            return { SUCCESS, winss::HandleWrapper(handle, false) };
        } catch (const std::out_of_range&) {
            VLOG(1) << "HANDLE index " << index << " out of range " << size;
        }
    }

    return { FAILED };
}

winss::WaitResult winss::HandleWrapper::Wait(DWORD timeout) const {
    std::vector<HANDLE> handles = { handle };
    return Wait(timeout, handles);
}

HANDLE winss::HandleWrapper::Duplicate(bool inherit) const {
    HANDLE h = nullptr;

    if (handle != nullptr) {
        DWORD options = 0;
        if (dup_rights == 0) {
            options = DUPLICATE_SAME_ACCESS;
        }

        WINDOWS.DuplicateHandle(GetCurrentProcess(), handle,
            GetCurrentProcess(), &h, dup_rights, inherit, options);
    }

    return h;
}

bool winss::HandleWrapper::operator==(const winss::HandleWrapper& rhs) const {
    return handle == rhs.handle;
}

bool winss::HandleWrapper::operator!=(const winss::HandleWrapper& rhs) const {
    return handle != rhs.handle;
}

bool winss::HandleWrapper::operator<(const winss::HandleWrapper& rhs) const {
    return handle < rhs.handle;
}

bool winss::HandleWrapper::operator<=(const winss::HandleWrapper& rhs) const {
    return handle <= rhs.handle;
}

bool winss::HandleWrapper::operator>(const winss::HandleWrapper& rhs) const {
    return handle > rhs.handle;
}

bool winss::HandleWrapper::operator>=(const winss::HandleWrapper& rhs) const {
    return handle >= rhs.handle;
}

void winss::HandleWrapper::CloseHandle() {
    if (owned && handle != nullptr) {
        WINDOWS.CloseHandle(handle);
    }
}

winss::HandleWrapper& winss::HandleWrapper::operator=(
    const winss::HandleWrapper& h) {
    if (this != &h) {
        CloseHandle();
        handle = h.handle;
        owned = false;
        dup_rights = h.dup_rights;
    }
    return *this;
}

winss::HandleWrapper& winss::HandleWrapper::operator=(
    winss::HandleWrapper&& h) {
    if (this != &h) {
        CloseHandle();
        handle = h.handle;
        owned = h.owned;
        dup_rights = h.dup_rights;
        h.owned = false;
    }
    return *this;
}

winss::HandleWrapper::~HandleWrapper() {
    CloseHandle();
}

/**
 * \relates HandleWrapper
 */
bool winss::operator==(const HandleWrapper & lhs, const HANDLE & rhs) {
    return lhs.handle == rhs;
}

/**
 * \relates HandleWrapper
 */
bool winss::operator==(const HANDLE &lhs, const winss::HandleWrapper &rhs) {
    return lhs == rhs.handle;
}

/**
 * \relates HandleWrapper
 */
bool winss::operator!=(const winss::HandleWrapper &lhs, const HANDLE &rhs) {
    return lhs.handle != rhs;
}

/**
 * \relates HandleWrapper
 */
bool winss::operator!=(const HANDLE &lhs, const winss::HandleWrapper &rhs) {
    return lhs != rhs.handle;
}

/**
 * \relates HandleWrapper
 */
bool winss::operator<(const winss::HandleWrapper &lhs, const HANDLE &rhs) {
    return lhs.handle < rhs;
}

/**
 * \relates HandleWrapper
 */
bool winss::operator<(const HANDLE &lhs, const winss::HandleWrapper &rhs) {
    return lhs < rhs.handle;
}

/**
 * \relates HandleWrapper
 */
bool winss::operator<=(const winss::HandleWrapper &lhs, const HANDLE &rhs) {
    return lhs.handle <= rhs;
}

/**
 * \relates HandleWrapper
 */
bool winss::operator<=(const HANDLE &lhs, const winss::HandleWrapper &rhs) {
    return lhs <= rhs.handle;
}

/**
 * \relates HandleWrapper
 */
bool winss::operator>(const winss::HandleWrapper &lhs, const HANDLE &rhs) {
    return lhs.handle > rhs;
}

/**
 * \relates HandleWrapper
 */
bool winss::operator>(const HANDLE &lhs, const winss::HandleWrapper &rhs) {
    return lhs > rhs.handle;
}

/**
 * \relates HandleWrapper
 */
bool winss::operator>=(const winss::HandleWrapper &lhs, const HANDLE &rhs) {
    return lhs.handle >= rhs;
}

/**
 * \relates HandleWrapper
 */
bool winss::operator>=(const HANDLE &lhs, const winss::HandleWrapper &rhs) {
    return lhs >= rhs.handle;
}

winss::TrustedHandleWrapper::TrustedHandleWrapper(HANDLE handle,
    DWORD dup_rights) : HandleWrapper(handle, true, dup_rights) {
}

HANDLE winss::TrustedHandleWrapper::GetHandle() const {
    return handle;
}

winss::HandleWrapper winss::TrustedHandleWrapper::GetHandleWrapper() const {
    return winss::HandleWrapper(handle, false, dup_rights);
}
