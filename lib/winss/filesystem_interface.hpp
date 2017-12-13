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

#ifndef LIB_WINSS_FILESYSTEM_INTERFACE_HPP_
#define LIB_WINSS_FILESYSTEM_INTERFACE_HPP_

#include <windows.h>
#include <filesystem>
#include <vector>
#include <memory>
#include <string>

#define FILESYSTEM winss::FilesystemInterface::GetInstance()

namespace fs = std::experimental::filesystem;

namespace winss {
/**
 * An interface for interacting with the file system.
 */
class FilesystemInterface {
 protected:
    /** A singleton instance. */
    static std::shared_ptr<FilesystemInterface> instance;

 public:
    /**
     * Creates the file system interface.
     */
    FilesystemInterface() {}
    FilesystemInterface(const FilesystemInterface&) = delete;  /**< No copy. */
    FilesystemInterface(FilesystemInterface&&) = delete;  /**< No move. */

    /**
     * Reads the contents of the file at the given path.
     *
     * \param[in] path The file path.
     * \return The contents of the file as a string. The string will be empty
     *         if the file does not exist.
     */
    virtual std::string Read(const fs::path& path) const;

     /**
     * Writes the given content to a file.
     *
     * \param[in] path The file path.
     * \param[in] content The content of the file.
     * \return True if the write was successful otherwise false.
     */
    virtual bool Write(const fs::path& path, const std::string& content) const;

    /**
     * Change the current directory to the one given.
     *
     * \param[in] dir The directory.
     * \return True if the change directory was successful otherwise false.
     */
    virtual bool ChangeDirectory(const fs::path& dir) const;

    /**
     * Checks if the directory exists.
     *
     * \param[in] dir The directory.
     * \return True if the directory exists otherwise false.
     */
    virtual bool DirectoryExists(const fs::path& dir) const;

    /**
     * Creates a directory.
     *
     * \param[in] dir The directory.
     * \return True if the directory now exists otherwise false.
     */
    virtual bool CreateDirectory(const fs::path& dir) const;

    /**
     * Renames a file/directory.
     *
     * \param[in] from Existing file/directory.
     * \param[in] to New file/directory.
     * \return True if the path rename was successful otherwise false.
     */
    virtual bool Rename(const fs::path& from, const fs::path& to) const;

    /**
     * Remove a file or empty directory.
     *
     * \param[in] path The file/directory to remove.
     * \return True if the path was removed otherwise false.
     */
    virtual bool Remove(const fs::path& path) const;

    /**
     * Checks if a file exists.
     *
     * \param[in] path The file to check.
     * \return True if the file exists otherwise false.
     */
    virtual bool FileExists(const fs::path& path) const;

    /**
     * Gets the absolute path.
     *
     * \param[in] path The path to convert.
     * \return The absolute path.
     */
    virtual fs::path Absolute(const fs::path& path) const;

    /**
    * Gets the canonical UNC path.
    *
    * \param[in] path The path to convert.
    * \return The canonical UNC path.
    */
    virtual fs::path CanonicalUncPath(const fs::path& path) const;

    /**
     * Gets a list of directories at the given path.
     *
     * \param[in] path The path to list directories.
     * \return A list of directories.
     */
    virtual std::vector<fs::path> GetDirectories(const fs::path& path) const;

    /**
     * Gets a list of files at the given path.
     *
     * \param[in] path The path to list files.
     * \return A list of files.
     */
    virtual std::vector<fs::path> GetFiles(const fs::path& path) const;

    /**
     * Gets the singleton instance.
     *
     * \return The singleton instance.
     */
    static const FilesystemInterface& GetInstance();

    /** No copy. */
    FilesystemInterface& operator=(const FilesystemInterface&) = delete;
    /** No move. */
    FilesystemInterface& operator=(FilesystemInterface&&) = delete;
};
}  // namespace winss

#endif  // LIB_WINSS_FILESYSTEM_INTERFACE_HPP_
