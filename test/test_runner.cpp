#include "gtest/gtest.h"
#include "winss/winss.hpp"
#include "easylogging/easylogging++.hpp"

INITIALIZE_EASYLOGGINGPP

int main(int argc, char* argv[]) {
    START_EASYLOGGINGPP(argc, argv);
    el::Configurations defaultConf;
    defaultConf.setToDefault();
    defaultConf.setGlobally(el::ConfigurationType::ToFile, "false");
    defaultConf.setGlobally(el::ConfigurationType::ToStandardOutput, "true");
    el::Loggers::reconfigureAllLoggers(defaultConf);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
