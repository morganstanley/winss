#include "pipe_name.hpp"
#include <filesystem>
#include <string>
#include "easylogging/easylogging++.hpp"
#include "filesystem_interface.hpp"
#include "sha256.hpp"

namespace fs = std::experimental::filesystem;

winss::PipeName::PipeName(fs::path path) {
    path = FILESYSTEM.Absolute(path);
    name = "\\\\.\\pipe\\" + winss::SHA256::CalculateDigest(path.string());
}

winss::PipeName::PipeName(fs::path path, std::string name) : PipeName(path) {
    if (!name.empty()) {
        this->name += "_" + name;
    }
}

winss::PipeName::PipeName(const winss::PipeName& p) : name(p.name) {}

winss::PipeName::PipeName(winss::PipeName&& s) : name(std::move(s.name)) {}

winss::PipeName winss::PipeName::Append(const std::string& name) const {
    winss::PipeName pipe_name;

    pipe_name.name = this->name;
    if (!name.empty()) {
        pipe_name.name += "_" + name;
    }

    return pipe_name;
}

const std::string& winss::PipeName::Get() const {
    VLOG(5) << "Using pipe name: " << name;
    return name;
}

void winss::PipeName::operator=(const winss::PipeName& p) {
    name = p.name;
}

winss::PipeName& winss::PipeName::operator=(winss::PipeName&& p) {
    name = std::move(p.name);
    return *this;
}
