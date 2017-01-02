#include "filesystem_interface.hpp"
#include <filesystem>
#include <string>
#include <fstream>
#include <vector>
#include <memory>
#include "easylogging/easylogging++.hpp"

namespace fs = std::experimental::filesystem;

std::shared_ptr<winss::FilesystemInterface>
winss::FilesystemInterface::instance =
    std::make_shared<winss::FilesystemInterface>();

std::string winss::FilesystemInterface::Read(const fs::path& path) const {
    try {
        VLOG(5) << "Reading file " << path;

        std::ifstream infile{ path };
        std::string cmd{
            std::istreambuf_iterator<char>(infile),
            std::istreambuf_iterator<char>()
        };
        infile.close();

        return cmd;
    } catch (const std::exception& e) {
        VLOG(1) << "Failed to read file " << path << ": " << e.what();
    }

    return "";
}

bool winss::FilesystemInterface::Write(const fs::path& path,
    const std::string& content) const {
    fs::path temp_path = path;
    temp_path += ".new";

    try {
        VLOG(5) << "Writting file " << temp_path;

        std::ofstream outfile(temp_path, std::ios::out | std::ios::trunc);
        outfile << content << std::endl;
        outfile.close();
    } catch (const std::exception& e) {
        VLOG(1) << "Failed to write file " << temp_path << ": " << e.what();
        return false;
    }

    return Rename(temp_path, path);
}

bool winss::FilesystemInterface::ChangeDirectory(const fs::path& dir) const {
    try {
        fs::current_path(dir);
    } catch (const fs::filesystem_error& e) {
        VLOG(1) << "Could not change directory " << dir << ": " << e.what();
        return false;
    }

    return true;
}

bool winss::FilesystemInterface::DirectoryExists(const fs::path& dir) const {
    try {
        fs::file_status dir_status = fs::status(dir);
        return fs::exists(dir_status) && fs::is_directory(dir_status);
    } catch (const fs::filesystem_error& e) {
        VLOG(1) << "Could not check exists " << dir << ": " << e.what();
        return false;
    }
}

bool winss::FilesystemInterface::CreateDirectory(const fs::path& dir) const {
    try {
        if (!DirectoryExists(dir)) {
            if (!fs::create_directory(dir)) {
                VLOG(6) << "Could not create directory " << dir;
                return false;
            }
        }

        return true;
    } catch (const fs::filesystem_error& e) {
        VLOG(1) << "Could not create directory " << dir << ": " << e.what();
        return false;
    }
}

bool winss::FilesystemInterface::Rename(const fs::path& from,
    const fs::path& to) const {
    try {
        VLOG(5) << "Renaming file " << from << " to " << to;

        fs::rename(from, to);
        return true;
    } catch (const fs::filesystem_error& e) {
        VLOG(1)
            << "Could not rename "
            << from
            << " to "
            << to
            << ": "
            << e.what();
        return false;
    }
}

bool winss::FilesystemInterface::Remove(const fs::path& path) const {
    try {
        VLOG(4) << "Removing path " << path;
        fs::remove(path);
        return true;
    } catch (const fs::filesystem_error& e) {
        VLOG(1) << "Could not remove path " << path << ": " << e.what();
        return false;
    }
}

bool winss::FilesystemInterface::FileExists(const fs::path& path) const {
    try {
        fs::file_status dir_status = fs::status(path);
        return fs::exists(dir_status) && fs::is_regular_file(dir_status);
    } catch (const fs::filesystem_error& e) {
        // Incase there are permission problems
        // in which case the file does not exist
        VLOG(1) << "Could check path exists " << path << ": " << e.what();
        return false;
    }
}

fs::path winss::FilesystemInterface::Absolute(const fs::path& path) const {
    try {
        return fs::canonical(path);
    } catch (const std::exception& e) {
        VLOG(1) << "Could not get canonical path " << path << ": " << e.what();
        return path;
    }
}

std::vector<fs::path> winss::FilesystemInterface::GetDirectories(
    const fs::path& path) const {
    std::vector<fs::path> directories;

    try {
        for (auto entry : fs::directory_iterator(path)) {
            if (fs::is_directory(entry.status())) {
                directories.push_back(entry.path());
            } else {
                VLOG(6) << "Skipping non-directory " << entry.path();
            }
        }
    } catch (const fs::filesystem_error& e) {
        VLOG(1)
            << "Could not iterate directories in "
            << path
            << ": "
            << e.what();
    }

    return directories;
}

std::vector<fs::path> winss::FilesystemInterface::GetFiles(
    const fs::path& path) const {
    std::vector<fs::path> files;

    try {
        for (auto entry : fs::directory_iterator(path)) {
            if (!fs::is_directory(entry.status())) {
                files.push_back(entry.path());
            } else {
                VLOG(6) << "Skipping directory " << entry.path();
            }
        }
    } catch (const fs::filesystem_error& e) {
        VLOG(1)
            << "Could not iterate files in "
            << path
            << ": "
            << e.what();
    }

    return files;
}

const winss::FilesystemInterface& winss::FilesystemInterface::GetInstance() {
    if (!winss::FilesystemInterface::instance) {
        winss::FilesystemInterface::instance =
            std::make_shared<FilesystemInterface>();
    }

    return *winss::FilesystemInterface::instance;
}
