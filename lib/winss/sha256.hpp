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

#ifndef LIB_WINSS_SHA256_HPP_
#define LIB_WINSS_SHA256_HPP_

#include <string>

namespace winss {
/**
 * SHA256 hash function helper.
 */
class SHA256 {
 private:
    /**
     * Disable creating instances.
     */
    SHA256() {}

 public:
    /**
     * Calculate the SHA256 value of the given string.
     *
     * \param value The string to take the SHA256 hash of.
     * \return The hash as a string.
     */
    static std::string CalculateDigest(const std::string& value);
};
}  // namespace winss

#endif  // LIB_WINSS_SHA256_HPP_
