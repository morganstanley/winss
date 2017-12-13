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

#ifndef LIB_WINSS_CASE_IGNORE_HPP_
#define LIB_WINSS_CASE_IGNORE_HPP_

#include <string>
#include <functional>

namespace winss {
/**
 * Unsigned char for readability.
 */
typedef unsigned char u_char;

/**
 * A binary function to ignore case when comparing strings.
 */
struct case_ignore : std::binary_function<std::string, std::string, bool> {
    /**
     * A binary function to ignore case when comparing characters.
     */
    struct nocase_compare : public std::binary_function<u_char, u_char, bool> {
        /**
         * Compares two characters ignoring the case.
         *
         * \param c1 The first character.
         * \param c2 The second character.
         * \return True if c1 < c2
         */
        bool operator() (const u_char& c1, const u_char& c2) const {
            return ::tolower(c1) < ::tolower(c2);
        }
    };

    /**
     * Compares two strings ignoring the case.
     *
     * \param s1 The first string.
     * \param s2 The second string.
     * \return True if s1 < s2 in lexicographical order.
     */
    bool operator() (const std::string& s1, const std::string& s2) const {
        return std::lexicographical_compare
        (s1.begin(), s1.end(), s2.begin(), s2.end(), nocase_compare());
    }
};
}  // namespace winss

#endif  // LIB_WINSS_CASE_IGNORE_HPP_
