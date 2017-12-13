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

#ifndef LIB_WINSS_PATH_MUTEX_HPP_
#define LIB_WINSS_PATH_MUTEX_HPP_

#include <windows.h>
#include <filesystem>
#include <string>

namespace fs = std::experimental::filesystem;

namespace winss {
/**
 * A global mutex where the key is derived from a file path and service name.
 */
class PathMutex {
 protected:
    std::string mutex_name;  /**< The service name. */
    HANDLE lock = nullptr;   /**< A handle to the Windows mutex. */

    PathMutex() {}  /**< Hide the default constructor. */

 public:
    /**
     * Create a path mutex with a path and a name.
     *
     * \param path The path under lock.
     * \param name The name of the service.
     */
    PathMutex(fs::path path, std::string name);

    PathMutex(const PathMutex&) = delete;  /**< No copy. */
    PathMutex(PathMutex&&) = delete;  /**< No move. */

    /**
     * Attempts to get the mutex.
     *
     * \return True if getting the mutex was successful otherwise false.
     */
    virtual bool Lock();

    /**
     * Checks if the lock will fail or not.
     *
     * \return True if getting the mutex will be successful otherwise false.
     */
    virtual bool CanLock() const;

    /**
     * Checks if this instance owns the lock.
     *
     * \return True if this instance owns the lock otherwise false.
     */
    virtual bool HasLock() const;

    /**
     * Gets the name of the service.
     *
     * \return The name of the service.
     */
    virtual const std::string& GetName() const;

    PathMutex& operator=(const PathMutex&) = delete;  /**< No copy. */
    PathMutex& operator=(PathMutex&&) = delete;  /**< No move. */

    virtual ~PathMutex();  /**< Default destructor. */
};
}  // namespace winss

#endif  // LIB_WINSS_PATH_MUTEX_HPP_
