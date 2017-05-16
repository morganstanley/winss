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

#ifndef LIB_WINSS_SUPERVISE_STATE_FILE_HPP_
#define LIB_WINSS_SUPERVISE_STATE_FILE_HPP_

#include <filesystem>
#include <string>
#include "supervise.hpp"

namespace fs = std::experimental::filesystem;

namespace winss {
/**
 * Serializes the state file but can also read it as a human-readable
 * message.
 */
class SuperviseStateFile : public winss::SuperviseListener {
 private:
    fs::path state_file;  /**< The state file location. */

 public:
    static const char kStateFile[];  /**< The state file name. */

     /**
     * Supervise state file constructor.
     *
     * \param service_dir The service directory.
     */
    explicit SuperviseStateFile(fs::path service_dir);
    SuperviseStateFile(const SuperviseStateFile&) = delete;  /**< No copy. */
    SuperviseStateFile(SuperviseStateFile&&) = delete;  /**< No move. */

    /**
     * Gets the path of the state file.
     *
     * \return The state file path.
     */
    virtual const fs::path& GetPath() const;

     /**
     * Supervisor listener handler.
     *
     * \param[in] notification The event which occurred.
     * \param[in] state The current state of the supervisor.
     * \return Always true.
     */
    virtual bool Notify(winss::SuperviseNotification notification,
        const winss::SuperviseState& state);

     /**
     * Read the state of the supervisor from the file.
     *
     * \param[out] state The state of the supervisor.
     * \return True if the state read was successful otherwise false.
     */
    virtual bool Read(winss::SuperviseState* state) const;

    /**
     * Format the state as a human-readable string.
     *
     * \param[in] state The state of the supervisor.
     * \param[in] is_up Hints if the supervisor is currently up.
     * \return Human-readable string.
     */
    virtual std::string Format(const winss::SuperviseState& state,
        bool is_up) const;

    void operator=(const SuperviseStateFile&) = delete;  /**< No copy. */
    /** No move. */
    SuperviseStateFile& operator=(SuperviseStateFile&&) = delete;
};
}  // namespace winss

#endif  // LIB_WINSS_SUPERVISE_STATE_FILE_HPP_
