#include "fs.h"
#include "string.h"
#include "gstdlib.h"

#include <regex>
#include <sys/stat.h>
#ifdef GE_PLATFORM_WINDOWS
#include <shlobj.h>
#elif defined(GE_PLATFORM_LINUX)
#include <unistd.h>
#include <pwd.h>
#include <dirent.h>
#endif

namespace granite { namespace base { namespace fs {

namespace {
string _dirProgramData, _dirUser, _dirWorkingDir;

string fullPath(directoryType type, const string &p = "") {
	if (type == userData) return _dirUser + GE_DIR_SEPARATOR + p;
	else if (type == programData) return _dirProgramData + GE_DIR_SEPARATOR + p;
	return _dirWorkingDir + GE_DIR_SEPARATOR + p;
}

bool _exists(const string &path) {
	struct stat buffer;
	return stat(path.c_str(), &buffer) == 0;
}

fileList _filterFileList(const string &basePath,
						 const string &path,
						 std::function<bool(fileInfo &)> pred = std::function<bool(fileInfo &)>()) {
	fileList r;

	#ifdef GE_PLATFORM_WINDOWS
	WIN32_FIND_DATA fd;
	HANDLE hf;
	if ((hf = FindFirstFile((basePath + path + (path == "" ? "*" : "\\*")).c_str(), &fd)) == INVALID_HANDLE_VALUE)
		return r;
	do {
		if (fd.cFileName[0] == '.')
			continue;
		fileInfo fi = { path,
						fd.cFileName,
						(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 };
		if (!pred || pred(fi))
			r.push_back(fi);
	}
	while(FindNextFile(hf, &fd));
	#elif defined(GE_PLATFORM_LINUX)
    DIR *dir;
    class dirent *ent;
    class stat st;
    dir = opendir((basePath + path).c_str());
    while ((ent = readdir(dir)) != NULL) {
		if (ent->d_name[0] == '.')
			continue;
		if (stat((basePath + path + "/" + ent->d_name).c_str(), &st) == -1)
            continue;
		fileInfo fi = { path,
						ent->d_name,
						(st.st_mode & S_IFDIR) != 0 };
		if (!pred || pred(fi))
			r.push_back(fi);
	}
    closedir(dir);
	#else
	#error "not implemented"
	#endif

	return r;
}

fileList _filterFileListR(const string &basePath,
						  const string &path,
						  std::function<bool(fileInfo &)> pred = std::function<bool(fileInfo &)>()) {
	fileList r;
	std::deque<string> dirToCheck = { path };

	do {
		string path = dirToCheck.front();
		for (const auto &f : _filterFileList(basePath, path, [](fileInfo &fi) { return fi.dir; }))
			dirToCheck.push_back(path == "" ? f.name : (path + GE_DIR_SEPARATOR + f.name));
		append(r, _filterFileList(basePath, path, pred));
		dirToCheck.pop_front();
	}
	while (!dirToCheck.empty());
	return r;
}
}

string getExecutableDirectory() {
	#ifdef GE_PLATFORM_WINDOWS
	char res[MAX_PATH];
	return extractFilePath(string(res, GetModuleFileName(NULL, res, MAX_PATH)));
	#elif defined(GE_PLATFORM_LINUX)
	char res[FILENAME_MAX];
	ssize_t count = readlink("/proc/self/exe", res, FILENAME_MAX);
	return extractFilePath(string(res, count > 0 ? count : 0));
	#else
	#error "Not implemented"
	#endif
	gassert(false, "could not find executable directory");
	return "";
}

string getUserDirectory() {
	#ifdef GE_PLATFORM_WINDOWS
	char path[MAX_PATH];
	if (SUCCEEDED(SHGetSpecialFolderPath(NULL, path, CSIDL_PROFILE, false)))
		return string(path);
	#elif defined(GE_PLATFORM_LINUX)
	struct passwd *pw = getpwuid(getuid());
	return pw->pw_dir;
	#else
	#endif
	gassert(false, "could not find user directory");
	return "";
}

bool open(const string &path, directoryType type) {
	if (_exists(path)) {
		if (type == userData)
			_dirUser = path;
		else if (type == programData)
			_dirProgramData = path;
		else if (type == workingDirectory)
			_dirWorkingDir = path;
		// TODO: search for vfs files and cache them
		return true;
	}
	gassert(false, strs("specified path does not exists: ", path));
	return false;
}

void close() {
	// TODO: delete cache
}

fileList listFiles(const string &path, directoryType type) {
	return _filterFileList(fullPath(type), path);
}

fileList findFiles(const string &name, const string &path, directoryType type) {
	return _filterFileListR(fullPath(type), path,
							[&name](fileInfo &fi) { return name == fi.name; });
}

fileList matchFiles(const string &regex, const string &path, directoryType type) {
	return _filterFileListR(fullPath(type), path,
							[&regex](fileInfo &fi) { return std::regex_match(fi.name, std::regex(regex)); });
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
stream load(const string &path, directoryType type) {
	std::FILE *f = std::fopen(fullPath(type, path).c_str(), "r");
	if (f == NULL) {
		gassert(false, strs("could not open file: ", path));
		return stream();
	}

	std::fseek(f, 0, SEEK_END);
	size_t size = std::ftell(f);
	std::rewind(f);

	stream s;
	s.resize(size);
	size_t readCount = std::fread(s.data(), size, 1, f);
	gassert(readCount == 1, strs("read file failed: ", path));

	std::fclose(f);
	return s;
}
#pragma GCC diagnostic pop

bool store(const string &path, stream &s, directoryType type, bool compress) {
	std::FILE *f = std::fopen(fullPath(type, path).c_str(), "w");
	if (f == NULL) {
		gassert(false, strs("could not open file: ", path));
		return false;
	}

	size_t bytesWrite = std::fwrite(path.data(), path.size(), 1, f);
	gassert(bytesWrite == 1, strs("write file failed: ", path));

	std::fclose(f);
	return bytesWrite == 1;
}

bool remove(const string &path, directoryType type) {
	int err = std::remove(fullPath(type, path).c_str());
	gassert(err == 0, strs("error deleting file: ", path));
	return err == 0;
}

bool exists(const string &name, directoryType type) {
	return _exists(fullPath(type, name));
}

}}}
