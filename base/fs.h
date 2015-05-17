/*
 * granite engine 1.0 | 2006-2015 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
 * file: ffs
 * created: 13-05-2015
 *
 * description: file system utils, VFS
 *
 * changelog:
 * - 01-12-2008: file create
 * - 13-05-2015: complete rewrite (deleted changelog)
 */

#pragma once
#include "includes.h"
#include "stream.h"

namespace granite { namespace base {

namespace fs {

enum directoryType {
	directoryTypeProgramData,
	directoryTypeUserData,
	directoryTypeWorkingDirecotry
};

bool open(const string &vfs, directoryType type);
void close();
const std::vector<string> &getFileList();
stream load(const string &path);
bool store(const string &path, stream &s, bool compress);
bool remove(const string &path);

}

}}
