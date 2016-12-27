#include "log_settings_parser.hpp"
#include <easylogging/easylogging++.hpp>
#include <winss/filesystem_interface.hpp>
#include <filesystem>
#include <vector>
#include <string>
#include "log_settings.hpp"

namespace fs = std::experimental::filesystem;

winss::LogSettings winss::LogSettingsParser::Parse(
    const std::vector<std::string>& directives) const {
    winss::LogSettings settings;

    for (const std::string& directive : directives) {
        if (directive.length() > 0) {
            const char id = directive.at(0);
            std::string value;

            if (directive.length() > 1) {
                value = directive.substr(1);
            }

            switch (id) {
            case 'n':
                try {
                    unsigned int number = std::stoul(value, nullptr, 10);

                    VLOG(3) << "Log number set to " << number;
                    settings.number = number;
                } catch (const std::exception&) {
                    LOG(WARNING) << "The number '" << value << "' is invalid";
                }
                break;
            case 's':
                try {
                    unsigned int file_size = std::stoul(value, nullptr, 10);

                    if (file_size < 4096) {
                        file_size = 4096;
                    }

                    VLOG(3) << "Log file size set to " << file_size;
                    settings.file_size = file_size;
                } catch (const std::exception&) {
                    LOG(WARNING) << "File size '" << value << "' is invalid";
                }
                break;
            case '\\':
                settings.log_dir = value;
                break;
            default:
                LOG(WARNING) << "Unrecognized script directive ";
                break;
            }
        }
    }

    settings.log_dir = FILESYSTEM.Absolute(settings.log_dir);

    return settings;
}
