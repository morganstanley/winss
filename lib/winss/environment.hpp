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

#ifndef LIB_WINSS_ENVIRONMENT_HPP_
#define LIB_WINSS_ENVIRONMENT_HPP_

#include <filesystem>
#include <vector>

namespace fs = std::experimental::filesystem;

namespace winss {
/**
 * Base environment.
 */
class Environment {
 public:
    /**
     * Reads the environment source into an environment block for CreateProcess.
     *
     * An environment block consists of a null-terminated block of
     * null-terminated strings. Each string is in the following form:
     *
     * name=value\0
     */
    virtual std::vector<char> ReadEnv() = 0;

    /**
     * Default destructor
     */
    virtual ~Environment() {}
};

/**
 * A directory where each file is an environment variable.
 */
class EnvironmentDir : public Environment {
 private:
    fs::path env_dir;  /**< The environment directory. */

 public:
    /**
     * Constructor with the environment directory.
     *
     * \param env_dir The environment directory.
     */
    explicit EnvironmentDir(fs::path env_dir);

    /**
     * Reads The env dir source into an environment block for CreateProcess
     *
     * An environment block consists of a null-terminated block of
     * null-terminated strings. Each string is in the following form:
     *
     * name=value\0
     */
    std::vector<char> ReadEnv();
};
}  // namespace winss

#endif  // LIB_WINSS_ENVIRONMENT_HPP_
