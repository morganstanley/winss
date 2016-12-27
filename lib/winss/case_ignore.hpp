#ifndef LIB_WINSS_CASE_IGNORE_HPP_
#define LIB_WINSS_CASE_IGNORE_HPP_

#include <string>
#include <functional>

namespace winss {
typedef unsigned char u_char;
struct case_ignore : std::binary_function<std::string, std::string, bool> {
    struct nocase_compare : public std::binary_function<u_char, u_char, bool> {
        bool operator() (const u_char& c1, const u_char& c2) const {
            return ::tolower(c1) < ::tolower(c2);
        }
    };

    bool operator() (const std::string& s1, const std::string& s2) const {
        return std::lexicographical_compare
        (s1.begin(), s1.end(), s2.begin(), s2.end(), nocase_compare());
    }
};
}  // namespace winss

#endif  // LIB_WINSS_CASE_IGNORE_HPP_
