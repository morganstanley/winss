/*
 * Copyright 2016-2017 Morgan Stanley
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
