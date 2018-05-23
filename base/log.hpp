/*
 * granite engine 1.0 | 2006-2015 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
 * file: log.*
 * created: 13-01-2013
 *
 * description: logging utility with signal callback installed, intended to use in async task
 *
 * changelog:
 * - 30-08-2006: original log class creation date from first engine version
 * - 13-01-2013: file created
 * - 15-01-2013: add functions to recognize error type
 * - 18-01-2013: first working version
 * - 20-01-2013: added macros/assert
 * - 23-01-2013: macros moved to common.h
 */

#pragma once

#include "includes.hpp"

namespace granite { namespace base { namespace log {

enum logLevel{
    logLevelError,
    logLevelOK,
    logLevelInfo,
    logLevelCritical
};

bool init(const string &fileName);
void log(logLevel level, const string &message);
void process();
const string &getBuffer(size_t index);
logLevel getLogLevel(size_t index);
size_t getBufferSize();
void shutdown();

}}}
