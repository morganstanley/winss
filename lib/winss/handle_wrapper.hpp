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

#ifndef LIB_WINSS_HANDLE_WRAPPER_HPP_
#define LIB_WINSS_HANDLE_WRAPPER_HPP_

#include <windows.h>
#include <vector>
#include "easylogging/easylogging++.hpp"

namespace winss {
/**
 * The wait result state for when waiting on handles.
 */
enum WaitResultState {
    SUCCESS,  /**< The wait was a success. */
    TIMEOUT,  /**< The wait timed out. */
    FAILED  /**< The wait failed. */
};

struct WaitResult;

/**
 * A wrapper for a Windows HANDLE.
 */
class HandleWrapper {
 protected:
    bool owned;  /**< If this instance owns the handle. */
    HANDLE handle;  /**< The wrapped handle. */
    DWORD dup_rights;  /**< The duplicate rights. */

    /**
     * Waits for an event to happen on the given list of handles.
     *
     * \param[in] timeout The wait timeout.
     * \param[in] handles The list of handles to wait for.
     * \return The wait result.
     */
    static WaitResult Wait(DWORD timeout, const std::vector<HANDLE>& handles);

    /**
     * Close the handle if the instance owns it.
     */
    void CloseHandle();

 public:
    /**
     * Create an empty handle wrapper.
     */
    HandleWrapper();

    /**
     * Create a new handle wrapper.
     *
     * \param handle The handle to wrap.
     * \param owned If the instance should own the handle.
     * \param dup_rights The duplicate rights on the handle.
     */
    explicit HandleWrapper(HANDLE handle, bool owned = true,
        DWORD dup_rights = 0);

    /**
     * Copies the handle wrapper.
     *
     * Any ownership rights will not be copied.
     *
     * \param h The handle wrapper to copy.
     */
    HandleWrapper(const HandleWrapper& h);

    /**
     * Move the handle wrapper.
     *
     * Any ownership rights will be moved
     *
     * \param h The handle wrapper to move.
     */
    HandleWrapper(HandleWrapper&& h);

    /**
     * Gets if the wrapper contains a handle.
     *
     * \return True if the wrapper contains a handle otherwise false.
     */
    bool HasHandle() const;

    /**
     * Gets if this instance owns the handle.
     *
     * \return True if the instance owns the handle otherwise false.
     */
    bool IsOwner() const;

    /**
     * Gets the duplicate rights of the handle.
     *
     * \return The duplicate rights as an unsigned int.
     */
    DWORD GetDuplicateRights() const;

     /**
     * Waits for an event on the handle.
     *
     * \param[in] timeout The wait timeout.
     * \return The wait result.
     */
    WaitResult Wait(DWORD timeout) const;

    /**
     * Waits for an event on a sequence of handles.
     *
     *
     * \tparam Iterator The iterator type.
     * \param[in] timeout The wait timeout.
     * \param[in] begin The start of the handle sequence.
     * \param[in] end The end of the handle sequence.
     * \return The wait result.
     */
    template<typename Iterator>
    static WaitResult Wait(DWORD timeout, Iterator begin, Iterator end) {
        std::vector<HANDLE> handles;

        for (auto it = begin; it != end; ++it) {
            handles.push_back(it->handle);
        }

        WaitResult result = Wait(timeout, handles);
        VLOG(7) << "Wait status:" << result.state;
        return result;
    }

    /**
     * Duplicates the handle.
     *
     * \param[in] inherit If the duplicated handle should be inheritable.
     * \return The new raw handle.
     */
    HANDLE Duplicate(bool inherit) const;

    /**
     * Checks the handles are equal.
     *
     * \param[in] rhs The other handle.
     * \return True if handles are equal.
     */
    bool operator==(const HandleWrapper& rhs) const;

    /**
     * Checks the handles are not equal.
     *
     * \param[in] rhs The other handle.
     * \return True if handles are not equal.
     */
    bool operator!=(const HandleWrapper& rhs) const;

    /**
     * Checks this handle is less than the rhs.
     *
     * \param[in] rhs The other handle.
     * \return True if this handle is less than the rhs.
     */
    bool operator<(const HandleWrapper& rhs) const;

    /**
     * Checks this handle is less than or equal to the rhs.
     *
     * \param[in] rhs The other handle.
     * \return True if this handle is less than or equal to the rhs.
     */
    bool operator<=(const HandleWrapper& rhs) const;

    /**
     * Checks this handle is greater than the rhs.
     *
     * \param[in] rhs The other handle.
     * \return True if this handle is greater than the rhs.
     */
    bool operator>(const HandleWrapper& rhs) const;

    /**
     * Checks this handle is greater than or equal to the rhs.
     *
     * \param[in] rhs The other handle.
     * \return True if this handle is greater than or equal to the rhs.
     */
    bool operator>=(const HandleWrapper& rhs) const;

    /**
     * Checks the handles are equal.
     *
     * \param[in] lhs The first handle.
     * \param[in] rhs The second handle.
     * \return True if handles are equal.
     */
    friend bool operator==(const HandleWrapper &lhs, const HANDLE &rhs);

     /**
     * Checks the handles are equal.
     *
     * \param[in] lhs The first handle.
     * \param[in] rhs The second handle.
     * \return True if handles are equal.
     */
    friend bool operator==(const HANDLE &lhs, const HandleWrapper &rhs);

     /**
     * Checks the handles are not equal.
     *
     * \param[in] lhs The first handle.
     * \param[in] rhs The second handle.
     * \return True if handles are not equal.
     */
    friend bool operator!=(const HandleWrapper &lhs, const HANDLE &rhs);

    /**
     * Checks the handles are not equal.
     *
     * \param[in] lhs The first handle.
     * \param[in] rhs The second handle.
     * \return True if handles are not equal.
     */
    friend bool operator!=(const HANDLE &lhs, const HandleWrapper &rhs);

    /**
     * Checks the lhs handle is less than the rhs.
     *
     * \param[in] lhs The first handle.
     * \param[in] rhs The other handle.
     * \return True if the lhs handle is less than the rhs.
     */
    friend bool operator<(const HandleWrapper &lhs, const HANDLE &rhs);

    /**
     * Checks the lhs handle is less than the rhs.
     *
     * \param[in] lhs The first handle.
     * \param[in] rhs The other handle.
     * \return True if the lhs handle is less than the rhs.
     */
    friend bool operator<(const HANDLE &lhs, const HandleWrapper &rhs);

    /**
     * Checks the lhs handle is less than or equal to the rhs.
     *
     * \param[in] lhs The first handle.
     * \param[in] rhs The other handle.
     * \return True if the lhs handle is less than or equal to the rhs.
     */
    friend bool operator<=(const HandleWrapper &lhs, const HANDLE &rhs);

    /**
     * Checks the lhs handle is less than or equal to the rhs.
     *
     * \param[in] lhs The first handle.
     * \param[in] rhs The other handle.
     * \return True if the lhs handle is less than or equal to the rhs.
     */
    friend bool operator<=(const HANDLE &lhs, const HandleWrapper &rhs);

    /**
     * Checks the lhs handle is greater than the rhs.
     *
     * \param[in] lhs The first handle.
     * \param[in] rhs The other handle.
     * \return True if the lhs handle is greater than the rhs.
     */
    friend bool operator>(const HandleWrapper &lhs, const HANDLE &rhs);

    /**
     * Checks the lhs handle is greater than the rhs.
     *
     * \param[in] lhs The first handle.
     * \param[in] rhs The other handle.
     * \return True if the lhs handle is greater than the rhs.
     */
    friend bool operator>(const HANDLE &lhs, const HandleWrapper &rhs);

    /**
     * Checks the lhs handle is greater than or equal to the rhs.
     *
     * \param[in] lhs The first handle.
     * \param[in] rhs The other handle.
     * \return True if the lhs handle is greater than or equal to the rhs.
     */
    friend bool operator>=(const HandleWrapper &lhs, const HANDLE &rhs);
    friend bool operator>=(const HANDLE &lhs, const HandleWrapper &rhs);

    /**
     * Copies the handle wrapper.
     *
     * Any ownership rights will not be copied.
     *
     * \param h The handle wrapper to copy.
     */
    void operator=(const HandleWrapper& h);

     /**
     * Move the handle wrapper.
     *
     * * Any ownership rights will be moved
     *
     * \param h The handle wrapper to move.
     * \return This handle wrapper.
     */
    HandleWrapper& operator=(HandleWrapper&& h);

    /**
     * Cleans up the handle.
     */
    ~HandleWrapper();
};

/**
 * The wait result for when waiting on handles.
 */
struct WaitResult {
    WaitResultState state;
    HandleWrapper handle;
};
}  // namespace winss

#endif  // LIB_WINSS_HANDLE_WRAPPER_HPP_
