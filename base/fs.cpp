#include "fs.h"
#include "string.h"
#include <sys/stat.h>
#ifdef GE_PLATFORM_WINDOWS
#include <shlobj.h>
#endif

namespace granite { namespace base { namespace fs {

namespace {
string _dirProgramData, _dirUser, _dirWorkingDir;

string fullPath(directoryType type, const string &p) {
	if (type == directoryTypeUserData) return _dirUser + GE_DIR_SEPARATOR + p;
	else if (type == directoryTypeProgramData) return _dirProgramData + GE_DIR_SEPARATOR + p;
	return _dirWorkingDir + GE_DIR_SEPARATOR + p;
}

bool _exists(const string &path) {
	struct stat buffer;
	return stat(path.c_str(), &buffer) == 0;
}

}

string getExecutableDirectory() {
	#ifdef GE_PLATFORM_WINDOWS
	char res[MAX_PATH];
	return extractFilePath(string(res, GetModuleFileName(NULL, res, MAX_PATH)));
	#elif GE_PLATFORM_LINUX
	char res[PATH_MAX];
	ssize_t count = readlink("/proc/self/exe", res, PATH_MAX);
	return extractFilePath(string(res, count > 0 ? count : 0));
	#else
	#error "Not implemented"
	#endif
	gassert(false, "could not find executable directory");
}

string getUserDirectory() {
	#ifdef GE_PLATFORM_WINDOWS
	char path[MAX_PATH];
	if (SUCCEEDED(SHGetSpecialFolderPath(NULL, path, CSIDL_PROFILE, false)))
		return string(path);
	#elif GE_PLATFORM_LINUX
	#else
	#endif
	gassert(false, "could not find user directory");
}

bool open(const string &path, directoryType type) {
	if (_exists(path)) {
		if (type == directoryTypeUserData)
			_dirUser = path;
		else if (type == directoryTypeProgramData)
			_dirProgramData = path;
		else if (type == directoryTypeWorkingDirecotry)
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

fileList getFileList(const string &path, directoryType type) {
	string p = fullPath(type, path);
	fileList r;

	#ifdef GE_PLATFORM_WINDOWS
	WIN32_FIND_DATA fd;
	HANDLE hf;
	if ((hf = FindFirstFile((p + "*").c_str(), &fd)) == INVALID_HANDLE_VALUE)
		return r;
	do {
		if (fd.cFileName[0] == '.')
			continue;
		r.push_back(std::make_tuple(fd.cFileName,
									(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0));
	}
	while(FindNextFile(hf, &fd));
	#elif GE_PLATFORM_LINUX
    DIR *dir;
    class dirent *ent;
    class stat st;
    dir = opendir(directory);
    while ((ent = readdir(dir)) != NULL) {
		if (ent->d_name[0] == '.')
			continue;
		r.push_back(std::make_tuple(end->d_name,
									(st.st_mode & S_IFDIR) != 0));
	}
    closedir(dir);
	#else
	#error "not implemented"
	#endif

	return r;
}

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
