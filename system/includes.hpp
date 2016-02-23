/*
 * granite engine 1.0 | 2006-2016 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
 * file: includes.h
 * created: 22-10-2012
 *
 * description: common *external* includes
 *
 * changelog:
 * - 22-10-2012: add
 *
 */

#pragma once
#include "../base/base.hpp"

#ifndef GE_DONT_INCLUDE_GLFW
#include <GLFW/glfw3.h>
#endif

// base namespace is visible in system
namespace granite { namespace system {
using namespace ::granite::base;
}}

//~
