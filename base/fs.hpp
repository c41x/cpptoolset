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
#include "includes.hpp"
#include "stream.hpp"

namespace granite { namespace base {

namespace fs {

enum directoryType {
    programData,
    userData,
    workingDirectory
};

struct fileInfo {
    string path;
    string name;
    bool dir;
    uint64 createTime, modTime;
    string fullPath() const;
    bool operator==(const fileInfo &f) const;
};

typedef std::vector<fileInfo> fileList;

string getExecutableDirectory();
string getUserDirectory();

bool open(const string &path, directoryType type = workingDirectory);
void createFolderTree(const string &path);
void preferArchives(bool doPrefer);
void doNotCompress(std::vector<string> extensions);
void allowGlobalPaths(bool doAllow);
void initArchive(const string &path, directoryType type = workingDirectory);
void initAllArchives(directoryType type = workingDirectory);
void flush();
bool createArchive(const string &path, directoryType = workingDirectory);
void close();
fileList listFiles(const string &path = "", directoryType type = workingDirectory);
fileList listFilesFlat(const string &path = "", directoryType type = workingDirectory);
fileList findFiles(const string &name, const string &path = "", directoryType type = workingDirectory);
fileList matchFiles(const string &regex, const string &path = "", directoryType type = workingDirectory);
stream load(const string &path, directoryType type = workingDirectory);
bool store(const string &path, const_stream s, directoryType type = workingDirectory, bool compress = true);
bool remove(const string &path, directoryType type = workingDirectory);
bool exists(const string &name, directoryType type = workingDirectory);

//- directory watcher
enum fileMonitorChange {
    fileMonitorAdd,
    fileMonitorRemove,
    fileMonitorModify
};

typedef std::vector<std::tuple<fileMonitorChange, string>> fileMonitorChanges;

uint32 addWatch(const string &dir, bool recursively = false, directoryType type = workingDirectory);
fileMonitorChanges pollWatch(uint32 id);
void removeWatch(uint32 id);

}

}}
// TODO: null delimited char load (see rosemary vulkan shader loading (double allocation only to have null terminator))
