/*
 * granite engine 1.0 | 2006-2013 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com 
 * file: common.h
 * created: 22-10-2012
 * 
 * description: some useful defines/macros/typedefs. All base files should include this file
 * 
 * changelog:
 * - 28-09-2008: initial add date in old engine version
 * - 22-10-2012: add
 * - 19-01-2013: many improvements:
 *   * stringify most of magic defines
 *   * toggle breakpoint added
 *   * full build info macro
 *   * fixed GE_DEBUG and __SFUNC__ macros on MinGW
 * - 26-01-2013: gassert macro outputs message to std::cerr
 */

#pragma once

// define some info about this software
#define GE_NAME "Granite"
#define GE_VERSION 1.0
#define GE_SVERSION "1.0"
#define GE_AUTHOR "Jakub Duracz"          
#define GE_CREATION_DATE "22-08-2006"

// quoting shit
#define _GE_Q(x) #x
#define _GE_QUOTE(x) _GE_Q(x)

// detect compiler
#if defined(_MSC_VER) // visual c++
	#define GE_COMPILER_VISUAL
	#define GE_COMPILER_VERSION _MSC_VER
	#define GE_SCOMPILER_VERSION _GE_QUOTE(_MSC_VER)
	#define GE_SCOMPILER "VisualStudio " _GE_QUOTE(GE_COMPILER_VERSION)
#elif defined(__BORLANDC__) // borland c++
	#define GE_COMPILER_BORLAND
	#define GE_COMPILER_VERSION __BCPLUSPLUS__
	#define GE_SCOMPILER_VERSION _GE_QUOTE(__BCPLUSPLUS__)
	#define GE_SCOMPILER "BorlandC++ " _GE_QUOTE(GE_COMPILER_VERSION)
#elif defined(__GNUC__) // gcc
	#define GE_COMPILER_GCC
	#define GE_COMPILER_VERSION (((__GNUC__)*100)+(__GNUC_MINOR__*10)+__GNUC_PATCHLEVEL__)
	#define GE_SCOMPILER_VERSION _GE_QUOTE(__GNUC__) "." _GE_QUOTE(__GNUC_MINOR__) "." _GE_QUOTE(__GNUC_PATCHLEVEL__)
	#define GE_SCOMPILER "GCC " GE_SCOMPILER_VERSION
#else
	#pragma error "Unknown compiler!"
#endif

// detect operating system
#if defined(__WIN32__)||defined(_WIN32)
	#define GE_PLATFORM_WINDOWS
	#define GE_PLATFORM "Windows"
#elif defined(__APPLE__)||defined(__APPLE_CC__)
	#define GE_PLATFORM_APPLE
	#define GE_PLATFORM "Apple"
#else
	#define GE_PLATFORM_LINUX
	#define GE_PLATFORM "Linux"
#endif

// detect architecture (64 bit or 32 bit)
#if defined(__x86_64__)||defined(d_M_X64)||defined(__powerpc64__)||defined( __alpha__)||defined(__ia64__)||defined(__s390__)||defined(__s390x__)
	#define GE_ARCHITECTURE_64
	#define GE_ARCHITECTURE "64bit"
#else
	#define GE_ARCHITECTURE_32
	#define GE_ARCHITECTURE "32bit"
#endif


// lets define that __SFUNC__ holds current processed function (by preprocessor)
// be aware that in GCC all *function* magic macros are variables like char[]="functionName()"
#ifdef GE_COMPILER_BORLAND
	#define __SFUNC__ __FUNC__
#elif defined(GE_COMPILER_GCC)
	#define __SFUNC__ __PRETTY_FUNCTION__
#else // visual
	#define __SFUNC__ __FUNCTION__
#endif

// stringify __LINE__
#define __SLINE__ _GE_QUOTE(__LINE__)

// directory separator
#if defined(GE_PLATFORM_WINDOWS)
	#define GE_DIR_SEPARATOR '\\'
#else
	#define GE_DIR_SEPARATOR '/'
#endif

// EOL
#if defined(GE_PLATFORM_WINDOWS)
	#define GE_EOL "\r\n"
#elif defined(GE_PLATFORM_LINUX)
	#define GE_EOL "\n"
#elif defined(GE_PLATFORM_APPLE)
	#define GE_EOL "\r"
#endif

// compiler mode (debug or release)
#if defined(NDEBUG)
	#define GE_RELEASE
#else
	#define GE_DEBUG
#endif

// full build info
#if defined(GE_DEBUG)
	#define GE_BUILD_INFO GE_NAME " " GE_SVERSION " " GE_SCOMPILER " Debug build " GE_PLATFORM " " GE_ARCHITECTURE
#else
	#define GE_BUILD_INFO GE_NAME " " GE_SVERSION " " GE_SCOMPILER " Release build " GE_PLATFORM " " GE_ARCHITECTURE
#endif

// align to 1 byte
#if defined(GE_COMPILER_VISUAL)
	#define GE_PACKED __declspec(align(1))
#else
	#define GE_PACKED __attribute__((packed))
#endif

// align data to given value in bytes
#if defined(GE_COMPILER_VISUAL)
	#define GE_ALIGN_BEGIN(BYTES) __declspec(align(BYTES))
	#define GE_ALIGN_END(BYTES)
#else
	#define GE_ALIGN_BEGIN(BYTES)
	#define GE_ALIGN_END(BYTES) __attribute__((aligned(BYTES)))
#endif

// line information (separated by commas)
#define GE_LINE_INFO __FILE__,__LINE__,__FUNC__

// basic typedefs
namespace granite{
#ifdef GE_COMPILER_VISUAL
	typedef __int8 int8;
	typedef __int16 int16;
	typedef __int32 int32;
	typedef __int64 int64;
	typedef unsigned __int8 uint8;
	typedef unsigned __int16 uint16;
	typedef unsigned __int32 uint32;
	typedef unsigned __int32 uint;
	typedef unsigned __int64 uint64;
#elif defined(GE_COMPILER_BORLAND)
	typedef char int8;
	typedef short int16;
	typedef int int32;
	typedef __int64 int64;
	typedef unsigned char uint8;
	typedef unsigned short uint16;
	typedef unsigned int uint32;
	typedef unsigned int uint;
	typedef unsigned __int64 uint64;
#elif defined(GE_COMPILER_GCC)
	typedef char int8;
	typedef short int16;
	typedef int int32;
	typedef long long int64;
	typedef unsigned char uint8;
	typedef unsigned short uint16;
	typedef unsigned int uint32;
	typedef unsigned int uint;
	typedef unsigned long long uint64;
#endif
}

// run once macro - usefull for fast prototyping (do not use in production code)
#define GE_ONCE(RUNCODE)                          \
    do{                                           \
        static bool x=true;                       \
        if(x){                                    \
            RUNCODE;                              \
            x=false;                              \
        }                                         \
    }while(false)

// lock unused warnings
#if defined(GE_RELEASE) && defined(GE_COMPILER_VISUAL)
	//C4530: C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
	#pragma warning(disable:4530)
#endif

// macro for toggling breakpoint
#if defined(GE_COMPILER_VISUAL)
	// this one uses winbase.h header (just include windows.h)
	#define toggleBreakpoint DebugBreak()
#elif defined(GE_COMPILER_GCC)
	#define toggleBreakpoint __builtin_trap()
#else //?
	#define toggleBreakpoint
#endif

// logging macros
#define GE_LOG_PREFIX_STRING string(__FILE__ "@" __SLINE__ ":")+string(__SFUNC__)+string("> ")
#define logError(message) granite::base::log::log(granite::base::log::logLevelError,GE_LOG_PREFIX_STRING+message)
#define logOK(message) granite::base::log::log(granite::base::log::logLevelOK,GE_LOG_PREFIX_STRING+message)
#define logInfo(message) granite::base::log::log(granite::base::log::logLevelInfo,GE_LOG_PREFIX_STRING+message)
#define logCritical(message) granite::base::log::log(granite::base::log::logLevelCritical,GE_LOG_PREFIX_STRING+message)

// assert
#ifdef GE_DEBUG
#define gassert(condition,message) if(!(condition)) {const string e=string("assertion failed(" #condition ") ")+GE_LOG_PREFIX_STRING+message; logError(e); std::cerr<<e<<std::endl; toggleBreakpoint;} else
#else
#define gassert(condition,message)
#endif

//~

