#include <winss/winss.hpp>
#include <optionparser/optionparser.hpp>
#include <easylogging/easylogging++.hpp>
#include <winss/ctrl_handler.hpp>
#include <winss/not_owning_ptr.hpp>
#include <winss/log/log_settings.hpp>
#include <winss/log/log_settings_parser.hpp>
#include <winss/log/log.hpp>
#include <iostream>
#include <vector>
#include <string>
#include "resource/resource.h"

INITIALIZE_EASYLOGGINGPP

struct Settings {
    int verbose_level = 0;
    std::vector<std::string> log_args;
};

enum OptionIndex { UNKNOWN, HELP, VERSION, VERBOSE, TIMEOUT };
const option::Descriptor usage[] = {
    {
        UNKNOWN, 0, "", "", option::Arg::None,
        "Usage: winss-log" SUFFIX ".exe [options] script\n\n"
        "Options:"
    },
    {
        HELP, 0, "h", "help", option::Arg::None,
        "  --help  \tPrint usage and exit."
    },
    {
        VERSION, 0, "", "version", option::Arg::None,
        "  --version  \tPrint the current version of winss and exit."
    },
    {
        VERBOSE, 0, "v", "verbose", option::Arg::Optional,
        "  -v[<level>], \t--verbose[=<level>]  \tSets the verbose level."
    },
    { 0, 0, 0, 0, 0, 0 }
};

Settings ParseArgs(int argc, char* argv[]) {
    /* Skip program name argv[0] if present */
    argc -= (argc > 0);
    argv += (argc > 0);

    option::Stats stats(usage, argc, argv);
    std::vector<option::Option> options(stats.options_max);
    std::vector<option::Option> buffer(stats.buffer_max);
    option::Parser parse(usage, argc, argv, &options[0], &buffer[0]);

    if (parse.error()) {
        std::exit(100);
    }

    if (options[HELP]) {
        option::printUsage(std::cout, usage);
        std::exit(100);
    }

    if (options[VERSION]) {
        std::cout
            << "winss "
            << VERSION_MAJOR << "."
            << VERSION_MINOR << "."
            << VERSION_REVISION << "."
            << VERSION_BUILD;
#ifdef GIT_COMMIT_SHORT
        std::cout << "-" << GIT_COMMIT_SHORT;
#endif
        std::cout << std::endl;
        std::exit(0);
    }

    Settings settings{};
    for (int i = 0; i < parse.optionsCount(); ++i) {
        option::Option& opt = buffer[i];

        switch (opt.index()) {
        case VERBOSE:
            if (opt.arg == nullptr) {
                settings.verbose_level = el::base::consts::kMaxVerboseLevel;
            } else {
                try {
                    settings.verbose_level = std::strtol(opt.arg, nullptr, 10);
                } catch (const std::exception&) {
                    std::cerr
                        << "Option "
                        << opt.name
                        << " requires a numeric argument";
                }
            }
            break;
        }
    }

    for (int i = 0; i < parse.nonOptionsCount(); ++i) {
        settings.log_args.emplace_back(parse.nonOption(i));
    }

    return settings;
}

void ConfigureLogger(const Settings& settings) {
    el::Configurations defaultConf;
    defaultConf.setToDefault();
    defaultConf.setGlobally(el::ConfigurationType::ToFile, "false");
    defaultConf.setGlobally(el::ConfigurationType::ToStandardOutput, "true");
    el::Loggers::reconfigureAllLoggers(defaultConf);

    if (settings.verbose_level > 0) {
        el::Loggers::setVerboseLevel(settings.verbose_level);
    }
}

int main(int argc, char* argv[]) {
    /* Attach and ignore. The program will exit when stdin closes. */
    winss::AttachCtrlHandler();

    Settings settings = ParseArgs(argc, argv);
    ConfigureLogger(settings);

    winss::LogSettingsParser parser;
    winss::LogSettings log_settings = parser.Parse(settings.log_args);

    winss::LogStreamReader reader;
    winss::LogStreamWriter writer;

    winss::Log log(winss::NotOwned(&reader), winss::NotOwned(&writer),
        log_settings);
    return log.Start();
}
