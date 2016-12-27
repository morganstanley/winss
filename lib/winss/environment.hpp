#ifndef LIB_WINSS_ENVIRONMENT_HPP_
#define LIB_WINSS_ENVIRONMENT_HPP_

#include <filesystem>
#include <vector>

namespace fs = std::experimental::filesystem;

namespace winss {
class Environment {
 public:
    virtual std::vector<char> ReadEnv() = 0;
    virtual ~Environment() {}
};
class EnvironmentDir : public Environment {
 private:
    fs::path env_dir;

 public:
    explicit EnvironmentDir(fs::path env_dir);

    std::vector<char> ReadEnv();
};
}  // namespace winss

#endif  // LIB_WINSS_ENVIRONMENT_HPP_
