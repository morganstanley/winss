#ifndef LIB_WINSS_PIPE_NAME_HPP_
#define LIB_WINSS_PIPE_NAME_HPP_

#include <filesystem>
#include <string>

namespace fs = std::experimental::filesystem;

namespace winss {
class PipeName {
 protected:
    std::string name;

    PipeName() {}

 public:
    explicit PipeName(fs::path path);
    PipeName(fs::path path, std::string name);
    PipeName(const PipeName& p);
    PipeName(PipeName&& s);

    PipeName Append(const std::string& name) const;

    const std::string& Get() const;

    void operator=(const PipeName& p);
    PipeName& operator=(PipeName&& p);
};
}  // namespace winss

#endif  // LIB_WINSS_PIPE_NAME_HPP_
