/*
 * granite engine 1.0 | 2006-2016 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
 * file: hotkey
 * created: 22-02-2016
 *
 * description: global hotkeys
 *
 * changelog:
 * - 22-02-2016: file created
 */

#pragma once
#include "includes.hpp"
#include "keycodes.hpp"

namespace granite { namespace system {
namespace hotkey {

bool init();
int add(keyId key, modId mods, std::function<void()> fx);
bool remove(int id);
void process();
void shutdown();

}
}}
