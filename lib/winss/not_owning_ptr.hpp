#ifndef LIB_WINSS_NOT_OWNING_PTR_HPP_
#define LIB_WINSS_NOT_OWNING_PTR_HPP_

#include <stdexcept>

namespace winss {
template <class T>
class NotOwningPtr {
 private:
    T* p;

 public:
    explicit NotOwningPtr(T* p) : p(p) {
        if (p == nullptr) {
            throw std::invalid_argument("cannot own a null pointer");
        }
    }

    template <class V>
    NotOwningPtr(const NotOwningPtr<V>& ptr) : p(ptr.Get()) {}

    T* Get() {
        return p;
    }

    T* Get() const {
        return p;
    }

    T& operator*() {
        return *p;
    }

    T& operator*() const {
        return *p;
    }

    T* operator->() {
        return p;
    }

    T* operator->() const {
        return p;
    }

    bool operator==(const NotOwningPtr& ptr) const {
        return p == ptr.p;
    }

    bool operator!=(const NotOwningPtr& ptr) const {
        return p != ptr.p;
    }

    void operator=(const NotOwningPtr& ptr) {
        p = ptr.p;
    }
};

template <class T>
NotOwningPtr<T> NotOwned(T* p) {
    return NotOwningPtr<T>(p);
}
}  // namespace winss

#endif  // LIB_WINSS_NOT_OWNING_PTR_HPP_
