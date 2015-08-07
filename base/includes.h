/*
 * granite engine 1.0 | 2006-2015 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
 * file: includes.h
 * created: 22-10-2012
 *
 * description: common *external* includes for base modules
 *
 * changelog:
 * - 22-10-2012: add
 * - 20-01-2013: added windows/linux headers for hires timers
 * - 29-09-2013: changelog moved to GIT repository
 */

#pragma once

#include "common.h"

// C-STD/STL
#include <ctime>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <stack>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <string>
#include <iostream>
#include <functional>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cassert>
#include <memory>
#include <numeric>

#ifdef GE_PLATFORM_WINDOWS
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <sys/time.h>
#include <time.h>
#endif

// SIMD
#include <mmintrin.h> //MMX
#include <xmmintrin.h> //SSE
#include <emmintrin.h> //SSE 2
#include <pmmintrin.h> //SSE 3

// string as built-in type
namespace granite { namespace base {
using std::string;
}}

//~
