#include <filesystem>
#include <iostream>
#include <vector>
#include "winss/winss.hpp"
#include "optionparser/optionparser.hpp"
#include "easylogging/easylogging++.hpp"
#include "winss/ctrl_handler.hpp"
#include "winss/windows_interface.hpp"
#include "winss/filesystem_interface.hpp"
#include "winss/not_owning_ptr.hpp"
#include "winss/wait_multiplexer.hpp"
#include "winss/supervise/supervise.hpp"
#include "winss/supervise/controller.hpp"
#include "winss/supervise/state_file.hpp"
#include "winss/pipe_server.hpp"
#include "winss/pipe_name.hpp"
#include "resource/resource.h"

INITIALIZE_EASYLOGGINGPP

namespace fs = std::experimental::filesystem;

struct Settings {
    fs::path service_dir;
    int verbose_level = 0;
};

enum OptionIndex { UNKNOWN, HELP, VERSION, VERBOSE };
const option::Descriptor usage[] = {
    {
        UNKNOWN, 0, "", "", option::Arg::None,
        "Usage: winss-supervise" SUFFIX ".exe [options] servicedir\n\n"
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

    if (options[HELP] || argc == 0) {
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

    if (parse.nonOptionsCount() < 1) {
        std::cerr << "servicedir is required!" << std::endl;
        std::exit(100);
    }

    Settings settings{};
    settings.service_dir = FILESYSTEM.Absolute(parse.nonOption(0));

    for (int i = 0; i < parse.optionsCount(); ++i) {
        option::Option& opt = buffer[i];

        if (opt.index() == VERBOSE) {
            if (opt.arg == nullptr) {
                settings.verbose_level = el::base::consts::kMaxVerboseLevel;
            } else {
                try {
                    settings.verbose_level = std::strtol(opt.arg, nullptr, 10);
                } catch(const std::exception&) {
                    std::cerr
                        << "Option "
                        << opt.name
                        << " requires a numeric argument";
                }
            }
        }
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
    winss::AttachCtrlHandler();

    Settings settings = ParseArgs(argc, argv);
    ConfigureLogger(settings);

    winss::WaitMultiplexer multiplexer;
    multiplexer.AddCloseEvent(winss::GetCloseEvent(), 0);

    winss::PipeName pipe_name(settings.service_dir,
        winss::Supervise::kMutexName);
    winss::OutboundPipeServer outbound({
        pipe_name.Append("event"),
        winss::NotOwned(&multiplexer)
    });
    winss::InboundPipeServer inbound({
        pipe_name.Append("control"),
        winss::NotOwned(&multiplexer)
    });

    winss::Supervise supervise(winss::NotOwned(&multiplexer),
        settings.service_dir);
    winss::SuperviseController controller(winss::NotOwned(&supervise),
        winss::NotOwned(&outbound), winss::NotOwned(&inbound));
    winss::SuperviseStateFile state_file(settings.service_dir);

    if (FILESYSTEM.CreateDirectory(state_file.GetPath().parent_path())) {
        supervise.AddListener(winss::NotOwned(&state_file));
    }

    return multiplexer.Start();
}
