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

#ifndef LIB_WINSS_NOT_OWNING_PTR_HPP_
#define LIB_WINSS_NOT_OWNING_PTR_HPP_

#include <stdexcept>

namespace winss {
/**
 * A container for pointers where the lifetime should be owned by the caller.
 *
 * Any function/class requiring a NotOwningPtr will be required to manage the
 * lifetime of the object. If the object is deleted before the function/class
 * has finished with it then unexpected errors will occur.
 *
 * \tparam T The type of pointer which is not owned.
 */
template <class T>
class NotOwningPtr {
 private:
    T* p;  /**< The pointer. */

 public:
    /**
     * Create a new NotOwningPtr with a pointer to the actual object.
     */
    explicit NotOwningPtr(T* p) : p(p) {
        if (p == nullptr) {
            throw std::invalid_argument("cannot own a null pointer");
        }
    }

    /**
     * Copy constructor to allow copying the pointer.
     */
    template <class V>
    NotOwningPtr(const NotOwningPtr<V>& ptr) : p(ptr.Get()) {}

    /**
     * Gets the not owned pointer.
     *
     * \return The raw pointer.
     */
    T* Get() {
        return p;
    }

    /**
     * Gets the not owned pointer as a constant.
     *
     * \return The raw pointer as a constant.
     */
    T* Get() const {
        return p;
    }

    /**
     * Gets the dereferenced object.
     *
     * \return The dereferenced object.
     */
    T& operator*() {
        return *p;
    }

    /**
     * Gets the dereferenced object as a constant.
     *
     * \return The dereferenced object as a constant.
     */
    T& operator*() const {
        return *p;
    }

    /**
     * Gets the not owned pointer.
     *
     * \return The raw pointer.
     */
    T* operator->() {
        return p;
    }

    /**
     * Gets the not owned pointer as a constant.
     *
     * \return The raw pointer as a constant.
     */
    T* operator->() const {
        return p;
    }

    /**
     * Checks equality of this not owned pointer and another.
     *
     * \param[in] ptr The other not owned pointer.
     * \return True if the pointers are the same otherwise false.
     */
    bool operator==(const NotOwningPtr& ptr) const {
        return p == ptr.p;
    }

     /**
     * Checks inequality of this not owned pointer and another.
     *
     * \param[in] ptr The other not owned pointer.
     * \return True if the pointers are not the same otherwise false.
     */
    bool operator!=(const NotOwningPtr& ptr) const {
        return p != ptr.p;
    }

    /**
     * Assigns the pointer to another pointer.
     *
     * \param[in] ptr The other not owned pointer.
     * \return This pointer.
     */
    NotOwningPtr& operator=(const NotOwningPtr& ptr) {
        p = ptr.p;
        return *this;
    }
};

/**
 * Helper method to make it easier to create a not owned pointer.
 *
 * \tparam T The type of pointer which is not owned.
 * \param[in] p The object pointer.
 * \return A not owned pointer object with the type of p.
 */
template <class T>
NotOwningPtr<T> NotOwned(T* p) {
    return NotOwningPtr<T>(p);
}
}  // namespace winss

#endif  // LIB_WINSS_NOT_OWNING_PTR_HPP_
