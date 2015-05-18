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

struct fileInfo {
	string path;
	string name;
	bool dir;
};

typedef std::vector<fileInfo> fileList;

string getExecutableDirectory();
string getUserDirectory();

bool open(const string &path, directoryType type);
void close();
fileList listFiles(const string &path = "", directoryType type = directoryTypeWorkingDirecotry);
fileList findFiles(const string &name, const string &path = "", directoryType type = directoryTypeWorkingDirecotry);
fileList matchFiles(const string &regex, const string &path = "", directoryType type = directoryTypeWorkingDirecotry);
stream load(const string &path, directoryType type = directoryTypeWorkingDirecotry);
bool store(const string &path, stream &s, directoryType type = directoryTypeWorkingDirecotry, bool compress = false);
bool remove(const string &path, directoryType type = directoryTypeWorkingDirecotry);
bool exists(const string &name, directoryType type = directoryTypeWorkingDirecotry);

}

}}
