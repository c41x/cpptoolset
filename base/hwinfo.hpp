/*
 * granite engine 1.0 | 2006-2015 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
 * file: hardware info
 * created: 06-04-2015
 *
 * description: hardware info, cpuid
 *
 * changelog:
 * - 04-11-2008: file created
 * - 06-04-2015: refactor, added to git repo
 */

#pragma once
#include "includes.hpp"

namespace granite { namespace base {

namespace cpu {

// CPU info
void refresh();
string toStr(); //!< nice formatted info
const char *getBrandName();
const char *getId();
int getCacheSize();
int getCacheLineSize();
int getL2Associativity();
bool supportMMX();
bool supportSSE();
bool supportSSE2();
bool support3DNow();
bool support3DNowEx();
bool supportMMXEx();
bool supportCMOV();
bool support64bit();

// cycles measurment utility
class cycleTimer {
    uint64 _last;

public:
    cycleTimer();
    ~cycleTimer();

    void reset();
    uint64 elapsed();
};
}

}}
