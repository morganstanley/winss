#ifndef LIB_WINSS_WINSS_HPP_
#define LIB_WINSS_WINSS_HPP_

#define ELPP_NO_DEFAULT_LOG_FILE
#define ELPP_CUSTOM_COUT std::cerr
#ifndef SUFFIX
#define SUFFIX ""
#endif
#include <windows.h>
#include <stdlib.h>
#pragma comment(lib, "rpcrt4.lib")

#endif  // LIB_WINSS_WINSS_HPP_
