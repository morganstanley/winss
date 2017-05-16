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

#ifndef LIB_WINSS_PIPE_NAME_HPP_
#define LIB_WINSS_PIPE_NAME_HPP_

#include <filesystem>
#include <string>

namespace fs = std::experimental::filesystem;

namespace winss {
/**
 * Pipe names are based on file system paths.
 *
 * This ensures that all pipes are created with a name that is in the same
 * format.
 */
class PipeName {
 protected:
    std::string name;  /** The final pipe name. */

    /** Avoid public access to empty pipe name. */
    PipeName() {}

 public:
    /**
     * Creates a standard pipe name based on only a path.
     *
     * \param path The file system path.
     */
    explicit PipeName(fs::path path);

    /**
     * Creates a standard pipe name based on a path and service name.
     *
     * \param path The file system path.
     * \param name The name of the service at the path.
     */
    PipeName(fs::path path, std::string name);

    /**
     * Create a new path based on another path.
     *
     * \param p The path to copy from.
     */
    PipeName(const PipeName& p);

    /**
     * Create a new path based on another path.
     *
     * \param p The path to move from.
     */
    PipeName(PipeName&& p);

    /**
     * Append an additional name to the current pipe.
     *
     * \param name The name to append.
     * \return The new pipe name.
     */
    PipeName Append(const std::string& name) const;

    /**
     * Gets the pipe name as a string.
     *
     * \return The pipe name as a string.
     */
    const std::string& Get() const;

    /**
     * Copy another pipe name to this pipe name.
     *
     * \param p The other pipe name.
     */
    void operator=(const PipeName& p);

    /**
     * Move another pipe name to this pipe name.
     *
     * \param p The other pipe name.
     * \return This pipe name.
     */
    PipeName& operator=(PipeName&& p);
};
}  // namespace winss

#endif  // LIB_WINSS_PIPE_NAME_HPP_
