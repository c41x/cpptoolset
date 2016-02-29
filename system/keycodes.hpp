/*
 * granite engine 1.0 | 2006-2016 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
 * file: keycodes
 * created: 22-02-2016
 *
 * description: portable key codes (X11 / WinAPI)
 *
 * changelog:
 * - 22-02-2016: file created
 */

#pragma once
#include "includes.hpp"

#if defined(GE_PLATFORM_LINUX)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

namespace granite { namespace system {

#if defined(GE_PLATFORM_WINDOWS)
typedef UINT keyId;
typedef UINT modId;
#elif defined(GE_PLATFORM_LINUX)
typedef unsigned int keyId;
typedef unsigned int modId;
#endif

modId getModifier(const granite::base::string &mod);
keyId getKey(const granite::base::string &key);

}}

//~
