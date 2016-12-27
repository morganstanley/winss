#ifndef LIB_WINSS_SHA256_HPP_
#define LIB_WINSS_SHA256_HPP_

#include <string>

namespace winss {
class SHA256 {
 private:
    SHA256() {}

 public:
    static std::string CalculateDigest(const std::string& value);
};
}  // namespace winss

#endif  // LIB_WINSS_SHA256_HPP_
