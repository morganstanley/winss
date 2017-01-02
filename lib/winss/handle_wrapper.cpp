#include "handle_wrapper.hpp"
#include <windows.h>
#include <vector>
#include "easylogging/easylogging++.hpp"
#include "windows_interface.hpp"

winss::HandleWrapper::HandleWrapper() :
    handle(nullptr), owned(false), dup_rights(0) {}

winss::HandleWrapper::HandleWrapper(HANDLE handle, bool owned,
    DWORD dup_rights) : handle(handle), owned(owned), dup_rights(dup_rights) {}

winss::HandleWrapper::HandleWrapper(const winss::HandleWrapper& h) :
    handle(h.handle), owned(false), dup_rights(h.dup_rights) {}

winss::HandleWrapper::HandleWrapper(winss::HandleWrapper&& h) :
    handle(h.handle), owned(h.owned), dup_rights(h.dup_rights) {
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

void winss::HandleWrapper::operator=(const winss::HandleWrapper& h) {
    if (this != &h) {
        CloseHandle();
        handle = h.handle;
        owned = false;
        dup_rights = h.dup_rights;
    }
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

bool winss::operator==(const winss::HandleWrapper &lhs, const HANDLE &rhs) {
    return lhs.handle == rhs;
}

bool winss::operator==(const HANDLE &lhs, const winss::HandleWrapper &rhs) {
    return lhs == rhs.handle;
}

bool winss::operator!=(const winss::HandleWrapper &lhs, const HANDLE &rhs) {
    return lhs.handle != rhs;
}

bool winss::operator!=(const HANDLE &lhs, const winss::HandleWrapper &rhs) {
    return lhs != rhs.handle;
}

bool winss::operator<(const winss::HandleWrapper &lhs, const HANDLE &rhs) {
    return lhs.handle < rhs;
}

bool winss::operator<(const HANDLE &lhs, const winss::HandleWrapper &rhs) {
    return lhs < rhs.handle;
}

bool winss::operator<=(const winss::HandleWrapper &lhs, const HANDLE &rhs) {
    return lhs.handle <= rhs;
}

bool winss::operator<=(const HANDLE &lhs, const winss::HandleWrapper &rhs) {
    return lhs <= rhs.handle;
}

bool winss::operator>(const winss::HandleWrapper &lhs, const HANDLE &rhs) {
    return lhs.handle > rhs;
}

bool winss::operator>(const HANDLE &lhs, const winss::HandleWrapper &rhs) {
    return lhs > rhs.handle;
}

bool winss::operator>=(const winss::HandleWrapper &lhs, const HANDLE &rhs) {
    return lhs.handle >= rhs;
}

bool winss::operator>=(const HANDLE &lhs, const winss::HandleWrapper &rhs) {
    return lhs >= rhs.handle;
}
