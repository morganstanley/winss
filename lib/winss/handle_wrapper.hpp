#ifndef LIB_WINSS_HANDLE_WRAPPER_HPP_
#define LIB_WINSS_HANDLE_WRAPPER_HPP_

#include <windows.h>
#include <easylogging/easylogging++.hpp>
#include <vector>

namespace winss {
enum WaitResultState {
    SUCCESS,
    TIMEOUT,
    FAILED
};
struct WaitResult;
class HandleWrapper {
 protected:
    bool owned;
    HANDLE handle;
    DWORD dup_rights;

    static WaitResult Wait(DWORD timeout, const std::vector<HANDLE>& handles);
    void CloseHandle();

 public:
    HandleWrapper();
    explicit HandleWrapper(HANDLE handle, bool owned = true,
        DWORD dup_rights = 0);
    HandleWrapper(const HandleWrapper& h);
    HandleWrapper(HandleWrapper&& h);

    bool HasHandle() const;
    bool IsOwner() const;
    DWORD GetDuplicateRights() const;

    WaitResult Wait(DWORD timeout) const;

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

    HANDLE Duplicate(bool inherit) const;

    bool operator==(const HandleWrapper& rhs) const;
    bool operator!=(const HandleWrapper& rhs) const;
    bool operator<(const HandleWrapper& rhs) const;
    bool operator<=(const HandleWrapper& rhs) const;
    bool operator>(const HandleWrapper& rhs) const;
    bool operator>=(const HandleWrapper& rhs) const;
    friend bool operator==(const HandleWrapper &lhs, const HANDLE &rhs);
    friend bool operator==(const HANDLE &lhs, const HandleWrapper &rhs);
    friend bool operator!=(const HandleWrapper &lhs, const HANDLE &rhs);
    friend bool operator!=(const HANDLE &lhs, const HandleWrapper &rhs);
    friend bool operator<(const HandleWrapper &lhs, const HANDLE &rhs);
    friend bool operator<(const HANDLE &lhs, const HandleWrapper &rhs);
    friend bool operator<=(const HandleWrapper &lhs, const HANDLE &rhs);
    friend bool operator<=(const HANDLE &lhs, const HandleWrapper &rhs);
    friend bool operator>(const HandleWrapper &lhs, const HANDLE &rhs);
    friend bool operator>(const HANDLE &lhs, const HandleWrapper &rhs);
    friend bool operator>=(const HandleWrapper &lhs, const HANDLE &rhs);
    friend bool operator>=(const HANDLE &lhs, const HandleWrapper &rhs);

    void operator=(const HandleWrapper& h);
    HandleWrapper& operator=(HandleWrapper&& h);

    ~HandleWrapper();
};
struct WaitResult {
    WaitResultState state;
    HandleWrapper handle;
};
}  // namespace winss

#endif  // LIB_WINSS_HANDLE_WRAPPER_HPP_
