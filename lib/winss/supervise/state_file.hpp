#ifndef LIB_WINSS_SUPERVISE_STATE_FILE_HPP_
#define LIB_WINSS_SUPERVISE_STATE_FILE_HPP_

#include <filesystem>
#include <string>
#include "supervise.hpp"

namespace fs = std::experimental::filesystem;

namespace winss {
class SuperviseStateFile : public winss::SuperviseListener {
 private:
    fs::path state_file;

 public:
    static const char kStateFile[];

    explicit SuperviseStateFile(fs::path service_dir);
    SuperviseStateFile(const SuperviseStateFile&) = delete;
    SuperviseStateFile(SuperviseStateFile&&) = delete;

    virtual const fs::path& GetPath() const;
    virtual bool Notify(winss::SuperviseNotification notification,
        const winss::SuperviseState& state);
    virtual bool Read(winss::SuperviseState* state) const;
    virtual std::string Format(const winss::SuperviseState& state,
        bool is_up) const;

    void operator=(const SuperviseStateFile&) = delete;
    SuperviseStateFile& operator=(SuperviseStateFile&&) = delete;
};
}  // namespace winss

#endif  // LIB_WINSS_SUPERVISE_STATE_FILE_HPP_
