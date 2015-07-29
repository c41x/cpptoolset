#include "fs.h"
#include "string.h"
#include "gstdlib.h"
#include "lz4.h"

#include <regex>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef GE_COMPILER_VISUAL
#include <shlobj.h>
#include <direct.h>
#include <io.h>
#elif defined(GE_COMPILER_GCC) && defined(GE_PLATFORM_LINUX)
#include <unistd.h>
#include <pwd.h>
#include <dirent.h>
#elif defined(GE_COMPILER_GCC)
#include <unistd.h>
#include <dirent.h>
#endif

namespace granite { namespace base { namespace fs {

namespace {
string _dirProgramData, _dirUser, _dirWorkingDir;
bool _preferVFS = false;
bool _allowGlobal = true;
std::vector<string> _doNotCompress = {"png", "jpg", "ogg", "mp3", "zip", "7z", "rar", "gfs", "flac"};

const char *directoryTypeToStr(directoryType type) {
	if (type == userData)
		return "user data directory";
	else if (type == programData)
		return "program data directory";
	else return "working directory";
}

// checks if given path is global
bool isGlobal(const string &s) {
	#ifdef GE_PLATFORM_WINDOWS
	return s.size() > 1 && s[1] == ':';
	#else
	return s.size() > 0 && s[0] == '/';
	#endif
}

// expand full path to directory
string getPath(directoryType type) {
	if (type == userData) return _dirUser;
	else if (type == programData) return _dirProgramData;
	return _dirWorkingDir;
}

#ifdef GE_PLATFORM_WINDOWS
string _normalizePath(const string &s) {
	string ret = s;
	findAndReplace(ret, '/', '\\');
	return ret;
}
#endif

// expand path
string fullPath(directoryType type, const string &p = "") {
	#ifdef GE_PLATFORM_WINDOWS
	return getPath(type) + GE_DIR_SEPARATOR + _normalizePath(p);
	#else
	return getPath(type) + GE_DIR_SEPARATOR + p;
	#endif
}

// checks if file/directory exists
bool _exists(const string &path) {
	struct stat buffer;
	return stat(path.c_str(), &buffer) == 0;
}

// checks if file exists
bool _exists_file(const string &file) {
	struct stat buffer;
	#ifdef GE_PLATFORM_WINDOWS
	return stat(file.c_str(), &buffer) == 0 && (buffer.st_mode & S_IFREG) != 0;
	#else
	return stat(file.c_str(), &buffer) == 0 && S_ISREG(buffer.st_mode) != 0;
	#endif
}

// resize given file
void _resize(std::FILE *f, size_t newSize) {
	#ifdef GE_COMPILER_VISUAL
	_chsize_s(_fileno(f), newSize);
	#else
	ftruncate(fileno(f), newSize);
	#endif
}

// create directory tree, path and pathBase must be normalized
bool _mkdirtree(const string &pathBase, const string &path) {
	string p = pathBase + GE_DIR_SEPARATOR;
	for (auto &e : divideString(path, GE_DIR_SEPARATOR)) {
		p += e.str();
		if (!_exists(p)) {
			#ifdef GE_COMPILER_VISUAL
			if (0 != _mkdir(p.c_str()))
				return false;
			#elif GE_PLATFORM_LINUX
			if (0 != mkdir(p.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
				return false;
			#else
			if (0 != mkdir(p.c_str()))
				return false;
			#endif
		}
		p += GE_DIR_SEPARATOR;
	}
	return true;
}

// gives list of files that fullfill predicate condition (only current folder)
fileList _filterFileList(const string &basePath,
						 const string &path,
						 std::function<bool(fileInfo &)> pred = std::function<bool(fileInfo &)>()) {
	fileList r;

	#ifdef GE_PLATFORM_WINDOWS
	auto toUnixTime = [](FILETIME ft) -> uint64 {
		ULARGE_INTEGER li;
		li.LowPart = ft.dwLowDateTime;
		li.HighPart = ft.dwHighDateTime;
		return li.QuadPart / 10000000 - 11644473600LL;
	};
	WIN32_FIND_DATA fd;
	HANDLE hf;
	if ((hf = FindFirstFile((basePath + path + (path == "" ? "*" : "\\*")).c_str(), &fd)) == INVALID_HANDLE_VALUE)
		return r;
	do {
		if (fd.cFileName[0] == '.')
			continue;
		fileInfo fi = { path,
						fd.cFileName,
						(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0,
						toUnixTime(fd.ftCreationTime),
						toUnixTime(fd.ftLastWriteTime) };
		findAndReplace(fi.path, "\\", "/");
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
						(st.st_mode & S_IFDIR) != 0,
						(uint64)st.st_mtime,
						(uint64)st.st_ctime };
		if (!pred || pred(fi))
			r.push_back(fi);
	}
    closedir(dir);
	#else
	#error "not implemented"
	#endif

	return r;
}

// same as above but recursively down in fs
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

//- vfs
/* gfs file structure
// header
char id[4]; // "GFS2"
uint64 headerOffset;

// data
< for each file:
uin8 rawData[size];
>

// index
< for each file:
uint32 filesCount;
uint32 nameLength;
char name[];
uint64 position;
uint64 size;
uint8 flags;
uint64 modTime;
uint64 createTime;
>
*/

// file entry
struct vfs_file {
	string id;
	uint64 position;
	uint64 size;
	uint8 flags; // 1 - compressed, 2 - directory
	uint64 createTime;
	uint64 modTime;
};

// index for (real) file
struct vfs {
	std::vector<vfs_file> files;
	uint64 indexOffset;
	std::FILE *f;
	bool dirty;
	std::vector<std::tuple<uint64, uint64>> removeQueue; // <position, size>
};

// index map (full path as key)
std::map<string, vfs> _vfs;

// get path from archive id
string _vfs_extract_path(const string &id) {
	size_t pos = id.find_last_of('/');
	if (pos != id.npos)
		return id.substr(0, pos);
	return "";
}

// get name from archive id
string _vfs_extract_name(const string &id) {
	size_t pos = id.find_last_of('/');
	if (pos != id.npos)
		return id.substr(pos + 1);
	return id;
}

// writes index to file (write pointer must be set before call)
void _vfs_write_index(vfs &v) {
	uint32 filesCount = (uint32)v.files.size();

	// write index to stream and write
	stream s;
	s.reserve(filesCount * (4 + 10 + 8 + 8 + 1 + 8 + 8)); // estimate size
	for (const auto &f : v.files) {
		s.write(f.id);
		s.write(f.position);
		s.write(f.size);
		s.write(f.flags);
		s.write(f.createTime);
		s.write(f.modTime);
	}
	size_t writeCount = std::fwrite(&filesCount, sizeof(uint32), 1, v.f);
	writeCount += std::fwrite(s.data(), s.size(), 1, v.f);
	gassertl(writeCount == 2, strs("could not write file index, errno: ", errno));

	// trim file here
	_resize(v.f, std::ftell(v.f));
}

// (re) opens/creates vfs, initializes index
void _vfs_open(const string path) {
	if (!_exists_file(path)) {
		// create directory tree first (if needed)
		#ifdef GE_PLATFORM_WINDOWS
		string filePath = extractFilePath(_normalizePath(path));
		_mkdirtree(filePath.substr(0, 2), filePath.substr(3));
		#else
		_mkdirtree("", extractFilePath(path));
		#endif

		// create and initialize new vfs
		uint32 filesCount = 0;
		vfs v;
		v.f = std::fopen(path.c_str(), "wb+");
		if (v.f == NULL) {
			gassertl(false, strs("vfs create: could not open file: ", path, "for write, errno: ", errno));
			return;
		}
		v.indexOffset = 4 + sizeof(uint64);
		v.dirty = false;
		size_t chunksWritten = 0;
		chunksWritten += std::fwrite("GFS2", 4, 1, v.f);
		chunksWritten += std::fwrite(&v.indexOffset, sizeof(uint64), 1, v.f);
		chunksWritten += std::fwrite(&filesCount, sizeof(uint32), 1, v.f);
		gassertl(chunksWritten == 3, strs("vfs create: could not write to file: ", path, " errno: ", errno));
		_vfs[path] = v;
	}
	else {
		auto vt = _vfs.find(path);
		if (vt == _vfs.end()) {
			// create index
			uint32 filesCount = 0;
			vfs v;
			stream s;
			size_t size;
			v.f = std::fopen(path.c_str(), "rb+");
			if (v.f == NULL) {
				gassert(false, strs("vfs open: could not open file: ", path, " errno: ", errno));
				return;
			}
			v.dirty = false;

			// read and validate header
			char id[4];
			if (1 != std::fread(id, 4, 1, v.f)) goto signalError;
			if (id[0] != 'G' || id[1] != 'F' || id[2] != 'S' || id[3] != '2') {
				std::fclose(v.f);
				gassert(false, strs("vfs open: not an vfs archive: ", path));
				return;
			}
			if (1 != std::fread(&v.indexOffset, sizeof(uint64), 1, v.f)) goto signalError;

			// read file index in one chunk
			if (0 != std::fseek(v.f, 0, SEEK_END)) goto signalError;
			size = std::ftell(v.f) - v.indexOffset;
			if (0 != std::fseek(v.f, (long)v.indexOffset, SEEK_SET)) goto signalError;
			s.resize(size);
			if (1 != std::fread(s.data(), size, 1, v.f)) goto signalError;

			// read index
			s.read(filesCount);
			while (filesCount-- > 0) {
				vfs_file fi;
				s.read(fi.id);
				s.read(fi.position);
				s.read(fi.size);
				s.read(fi.flags);
				s.read(fi.createTime);
				s.read(fi.modTime);
				v.files.push_back(fi);
			}

			// store index in map
			_vfs[path] = v;
			return;

			// print error to log
			signalError:
			std::fclose(v.f);
			gassertl(false, strs("vfs open: error reading file: ", path, " errno: ", errno));
			return;
		}
		else {
			// already in index - reopen if not opened
			if (vt->second.f == NULL)
				vt->second.f = fopen(path.c_str(), "rb+");
		}
	}
}

// calculates fragmented data size
uint64 _vfs_fragmented_size(vfs &v) {
	return std::accumulate(v.removeQueue.begin(), v.removeQueue.end(), uint64(0),
						   [](const auto &acc, const auto &e) {
							   return acc + std::get<1>(e);
						   });
}

// defragment file (collapse holes left by files removal)
void _vfs_defragment(vfs &v) {
	if (v.removeQueue.size() > 1) {
		// sort
		std::sort(v.removeQueue.begin(), v.removeQueue.end(),
				  [](const auto &a, const auto &b) {
					  return std::get<0>(a) < std::get<0>(b);
				  });

		// remove gaps
		for (size_t i = 0; i < v.removeQueue.size() - 1; ) {
			if (std::get<0>(v.removeQueue[i]) + std::get<1>(v.removeQueue[i]) == std::get<0>(v.removeQueue[i + 1])) {
				std::get<1>(v.removeQueue[i]) += std::get<1>(v.removeQueue[i + 1]);
				v.removeQueue.erase(v.removeQueue.begin() + i + 1);
			}
			else ++i;
		}
	}

	// move file parts
	stream buff(1024 * 1024);
	uint64 totalRemoved = 0;
	size_t size = v.removeQueue.size();
	for (size_t i = 0; i < size; ++i) {
		uint64 to = std::get<0>(v.removeQueue[i]);
		uint64 from = to + std::get<1>(v.removeQueue[i]);
		uint64 count = i + 1 == size ? -1 : (std::get<0>(v.removeQueue[i + 1]) - from);
		uint64 readed = 1;

		while (readed != 0) {
			std::fseek(v.f, from, SEEK_SET);
			readed = std::fread(buff.data(), 1, std::min(buff.size(), (size_t)count), v.f);
			std::fseek(v.f, to, SEEK_SET);
			std::fwrite(buff.data(), readed, 1, v.f);
			to += readed;
			from += readed;
			count -= readed;
		}

		uint64 fPos = std::get<0>(v.removeQueue[i]);
		uint64 fSize = std::get<1>(v.removeQueue[i]);
		totalRemoved += fSize;

		// update file index
		for (auto &fi : v.files) {
			if (fi.position > fPos)
				fi.position -= fSize;
		}
	}

	// clear queue
	v.removeQueue.clear();

	// update index offset
	v.indexOffset -= totalRemoved;

	// truncate file
	std::fseek(v.f, 0, SEEK_END);
	size_t currentSize = std::ftell(v.f);
	_resize(v.f, currentSize - totalRemoved);
}

// close file handle
void _vfs_close(vfs &v) {
	_vfs_defragment(v);
	if (v.dirty) {
		std::fseek(v.f, v.indexOffset, SEEK_SET);
		_vfs_write_index(v);

		// update header
		std::fseek(v.f, 4, SEEK_SET);
		if (1 != std::fwrite(&v.indexOffset, sizeof(uint32), 1, v.f)) {
			gassert(false, strs("could not close vfs: write failed, errno: ", errno));
		}
		v.dirty = false;
	}

	std::fclose(v.f);
	v.f = NULL;
}

// search for file id in index
std::vector<vfs_file>::iterator _vfs_find_file(vfs &v, const string &id) {
	return std::find_if(v.files.begin(), v.files.end(),
						[&id](vfs_file &f) {
							return f.id == id;
						});
}

// checks if file exists in archive index
bool _vfs_exists(vfs &v, const string &id) {
	return v.files.end() != _vfs_find_file(v, id);
}

// read file from archive to stream, returns empty stream if failed
void _vfs_read(vfs &v, const string &id, stream &s) {
	auto f = _vfs_find_file(v, id);
	gassert(f != v.files.end(), strs("vfs file: ", id, " not found"));
	if (f != v.files.end()) {
		std::fseek(v.f, f->position, SEEK_SET);

		if (f->flags & 1) {
			// decompress
			int64 realSize = 0;
			std::fread(&realSize, sizeof(int64), 1, v.f);
			stream sc(f->size - sizeof(int64));
			std::fread(sc.data(), sc.size(), 1, v.f);
			size_t offset = s.size();
			s.resize(offset + realSize);
			LZ4_decompress_safe((const char*)sc.data(), (char*)s.data() + offset, sc.size(), realSize);
		}
		else {
			// read
			size_t offset = s.size();
			s.resize(offset + f->size);
			std::fread(s.data() + offset, f->size, 1, v.f);
		}
	}
	else logError(strs("vfs read: file: ", id, " not found"));
}

// remove file from archive and index
bool _vfs_remove(vfs &v, const string &id) {
	auto f = _vfs_find_file(v, id);
	if (f != v.files.end()) {
		// add file to remove queue
		v.removeQueue.push_back(std::make_tuple(f->position, f->size));

		// remove from index
		v.files.erase(f);

		// if fragmented size exceeds given limit -> do deframentation
		if (_vfs_fragmented_size(v) > 1024 * 1024 * 100) // 100MB
			_vfs_defragment(v);

		// mark index as dirty
		v.dirty = true;
		return true;
	}
	else {
		// report error
		logError(strs("could not delete: ", id, " from vfs - file not present in index"));
		return false;
	}
}

// add/rename file to archive and index
bool _vfs_add(vfs &v, const string &id, const stream &s, bool compress = true) {
	auto f = _vfs_find_file(v, id);

	// replace file (remove old file first)
	uint64 createTime = std::time(0);
	if (f != v.files.end()) {
		_vfs_remove(v, id);
		createTime = f->createTime; // preserve create time
	}

	// disable compression for some file types
	if (compress) {
		if (std::count(_doNotCompress.begin(), _doNotCompress.end(), extractExt(id)) > 0)
			compress = false;
	}

	// create file
	std::fseek(v.f, v.indexOffset, SEEK_SET);
	if (compress) {
		stream sc(LZ4_compressBound(s.size()));
		uint64 compressedSize = LZ4_compress_default((const char*)s.data(), (char*)sc.data(), s.size(), sc.size());
		int64 realSize = s.size();
		size_t chunksWritten = 0;
		chunksWritten = std::fwrite(&realSize, sizeof(int64), 1, v.f);
		chunksWritten += std::fwrite(sc.data(), compressedSize, 1, v.f);
		if (chunksWritten != 2) goto writeError;
		v.files.push_back({id, v.indexOffset, compressedSize + sizeof(int64), 1, createTime, (uint64)std::time(0)});
	}
	else {
		if (1 != std::fwrite(s.data(), s.size(), 1, v.f)) goto writeError;
		v.files.push_back({id, v.indexOffset, s.size(), 0, createTime, (uint64)std::time(0)});
	}
	v.indexOffset = std::ftell(v.f);
	v.dirty = true;
	return true;

	writeError:
	gassertl(false, strs("error: could not write file to vfs: ", id, " errno: ", errno));
	return false;
}

// returns full path to file, vfs id, is vfs, is valid
std::tuple<string, string, bool, bool> _resolveLocation(const string &ipath, directoryType type, bool mustExist) {
	std::tuple<string, string, bool, bool> r;

	// resolve global file
	if (_allowGlobal && isGlobal(ipath)) {
		#ifdef GE_PLATFORM_WINDOWS
		if ((mustExist && _exists_file(_normalizePath(ipath))) || !mustExist)
			return std::make_tuple(_normalizePath(ipath), "", false, true);
		#else
		if ((mustExist && _exists_file(ipath)) || !mustExist)
			return std::make_tuple(ipath, "", false, true);
		#endif
	}

	auto resolveNormalFile = [&r, &type, &ipath, &mustExist]() {
		if (!mustExist || _exists_file(fullPath(type, ipath))) {
			r = std::make_tuple(fullPath(type, ipath), "", false, true);
			return true;
		}
		return false;
	};

	auto resolveVFS = [&r, &type, &ipath]() {
		string base = getPath(type);
		string path = base;
		std::vector<stringRange> explodePath = divideString(ipath, '/');
		for (auto &s : explodePath) {
			base += strs(GE_DIR_SEPARATOR, s);
			path = base + ".gfs";
			if (_vfs.find(path) != _vfs.end()) {
				r = std::make_tuple(path, stringRange(std::min(ipath.end(), s.end + 1), ipath.end()), true, true);
				return true;
			}
		}
		return false;
	};

	if (_preferVFS) {
		if (resolveVFS() || resolveNormalFile())
			return r;
	}
	else {
		if (resolveNormalFile() || resolveVFS())
			return r;
	}

	return std::make_tuple("", "", false, false);
}

// file op
template <typename T_OP>
void _vfs_files_op(const string &filepath, const string &id, T_OP op) {
	fileList r;
	auto v = _vfs.find(filepath);
	gassert(v != _vfs.end(), strs("vfs index not found: ", filepath));
	if (v != _vfs.end()) {
		for (const auto &f : v->second.files) {
			op(f);
		}
	}
}
}

//- impl
// expands path
string fileInfo::fullPath() const {
	return path == "" ? name : (path + '/' + name);
}

bool fileInfo::operator==(const fileInfo &f) const {
	return f.path == path && f.name == name && f.dir == dir;
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
	gassertl(false, "could not find executable directory");
	return "";
}

string getUserDirectory() {
	#ifdef GE_COMPILER_VISUAL
	char path[MAX_PATH];
	if (SUCCEEDED(SHGetSpecialFolderPath(NULL, path, CSIDL_PROFILE, false)))
		return string(path);
	#elif defined(GE_PLATFORM_LINUX)
	struct passwd *pw = getpwuid(getuid());
	return pw->pw_dir;
	#else
	#endif
	gassertl(false, "could not find user directory");
	return "";
}

// initialize fs
bool open(const string &path, directoryType type) {
	#ifdef GE_PLATFORM_WINDOWS
	string normalizedPath = _normalizePath(path);
	#else
	string normalizedPath = path;
	#endif
	if (_exists(normalizedPath)) {
		if (type == userData)
			_dirUser = normalizedPath;
		else if (type == programData)
			_dirProgramData = normalizedPath;
		else if (type == workingDirectory)
			_dirWorkingDir = normalizedPath;
		logInfo(strs("initialized ", directoryTypeToStr(type), ": ", normalizedPath));
		return true;
	}
	gassertl(false, strs("specified path does not exists: ", normalizedPath));
	return false;
}

// resolving file rule searches for archives vs. regular files first
void preferArchives(bool preferVFS) {
	_preferVFS = preferVFS;
}

// setup extensions
void doNotCompress(std::vector<string> extensions) {
	_doNotCompress = extensions;
}

void allowGlobalPaths(bool doAllow) {
	_allowGlobal = doAllow;
}

// flush all archives (writes file index for all archives)
void flush() {
	for (auto &v : _vfs) {
		_vfs_close(v.second);
		_vfs_open(v.first);
	}
}

// create archive file
bool createArchive(const string &path, directoryType type) {
	string fpath = fullPath(type, path);
	if (_exists_file(fpath))
		return false;
	_vfs_open(fpath);
	return true;
}

// initialize archive (archive must be initialized first, before use)
void initArchive(const string &path, directoryType type) {
	_vfs_open(fullPath(type, path));
}

// scan for vfs files and initialize all
void initAllArchives(directoryType type) {
	fileList fl = matchFiles(".*\\.gfs", "", type);
	for (const auto &f : fl)
		_vfs_open(fullPath(type, f.fullPath()));
}

// close file system
void close() {
	for (auto &v : _vfs)
		_vfs_close(v.second);
	_vfs.clear();
}

// list files in given directory
fileList listFiles(const string &path, directoryType type) {
	bool vfs, valid;
	string filepath, id;
	std::tie(filepath, id, vfs, valid) = _resolveLocation(path, type, true);

	if (vfs) {
		fileList r;
		_vfs_files_op(filepath, id,
					  [&r, &id](auto &f) {
						  string filePath = _vfs_extract_path(f.id);
						  if (id == filePath)
							  r.push_back({filePath, _vfs_extract_name(f.id), false, f.createTime, f.modTime});
						  else {
							  fileInfo fi = {_vfs_extract_path(filePath), _vfs_extract_name(filePath), true, f.createTime, f.modTime};
							  if (id == fi.path && std::count(r.begin(), r.end(), fi) == 0)
								  r.push_back(fi);
						  }
					  });
		return r;
	}

	return _filterFileList(fullPath(type), path);
}

fileList listFilesFlat(const string &path, directoryType type) {
	bool vfs, valid;
	string filepath, id;
	std::tie(filepath, id, vfs, valid) = _resolveLocation(path, type, true);

	if (vfs) {
		fileList r;
		_vfs_files_op(filepath, id,
					  [&r, &id](auto &f) {
						  fileInfo fi = {_vfs_extract_path(f.id), _vfs_extract_name(f.id), false, f.createTime, f.modTime};
						  if (!f.id.compare(0, id.size(), id) && std::count(r.begin(), r.end(), fi) == 0)
							  r.push_back(fi);
					  });
		return r;
	}

	return _filterFileListR(fullPath(type), path);
}

// find files resursively
fileList findFiles(const string &name, const string &path, directoryType type) {
	bool vfs, valid;
	string filepath, id;
	std::tie(filepath, id, vfs, valid) = _resolveLocation(path, type, true);

	if (vfs) {
		fileList r;
		_vfs_files_op(filepath, id,
					  [&r, &name](auto &f) {
						  string fName = _vfs_extract_name(f.id);
						  if (name == fName)
							  r.push_back({_vfs_extract_path(f.id), fName, false, f.createTime, f.modTime});
					  });
		return r;
	}

	return _filterFileListR(fullPath(type), path,
							[&name](fileInfo &fi) { return name == fi.name; });
}

// match files by regex recursively
fileList matchFiles(const string &regex, const string &path, directoryType type) {
	bool vfs, valid;
	string filepath, id;
	std::tie(filepath, id, vfs, valid) = _resolveLocation(path, type, true);

	if (vfs) {
		fileList r;
		_vfs_files_op(filepath, id,
					  [&r, &regex](auto &f) {
						  string fName = _vfs_extract_name(f.id);
						  if (std::regex_match(fName, std::regex(regex)))
							  r.push_back({_vfs_extract_path(f.id), fName, false, f.createTime, f.modTime});
					  });
		return r;
	}

	return _filterFileListR(fullPath(type), path,
							[&regex](fileInfo &fi) { return std::regex_match(fi.name, std::regex(regex)); });
}

// load file/archive file to stream
stream load(const string &path, directoryType type) {
	bool vfs, valid;
	string filepath, id;
	std::tie(filepath, id, vfs, valid) = _resolveLocation(path, type, true);

	// check if path is correctly resolved
	if (!valid) {
		gassert(false, strs("could not resolve file location: ", path));
		return stream();
	}

	// load / decompress vfs
	if (vfs) {
		auto v = _vfs.find(filepath);
		gassert(v != _vfs.end(), strs("vfs index not found: ", path));
		if (v != _vfs.end()) {
			stream s;
			_vfs_read(v->second, id, s);
			return s;
		}
		return stream();
	}

	// it's regular file - just load
	std::FILE *f = std::fopen(filepath.c_str(), "rb");
	if (f == NULL) {
		gassertl(false, strs("could not open file: ", path, " errno:", errno));
		return stream();
	}

	std::fseek(f, 0, SEEK_END);
	size_t size = std::ftell(f);
	std::rewind(f);

	stream s;
	s.resize(size);
	size_t readCount = std::fread(s.data(), size, 1, f);
	gassertl(readCount == 1 || size == 0, strs("read file failed: ", path, " errno: ", errno));

	std::fclose(f);
	return s;
}

// save stream to file/archive file
bool store(const string &path, stream &s, directoryType type, bool compress) {
	bool vfs, valid;
	string filepath, id;
	std::tie(filepath, id, vfs, valid) = _resolveLocation(path, type, false);

	if (vfs) {
		auto v = _vfs.find(filepath);
		if (v != _vfs.end())
			return _vfs_add(v->second, id, s, compress);
		gassertl(false, strs("could not write: ", path, " to vfs, probably archive not initialized"));
		return false;
	}

	std::FILE *f = std::fopen(fullPath(type, path).c_str(), "wb+");
	if (f == NULL) {
		#ifdef GE_PLATFORM_WINDOWS
		_mkdirtree(getPath(type), extractFilePath(_normalizePath(path)));
		#else
		_mkdirtree(getPath(type), extractFilePath(path));
		#endif
		f = std::fopen(fullPath(type, path).c_str(), "wb+");
		if (f == NULL) {
			gassertl(false, strs("could not open file: ", path, " errno:", errno));
			return false;
		}
	}

	size_t bytesWrite = std::fwrite(s.data(), s.size(), 1, f);
	if (s.size() != 0 && bytesWrite != 1) {
		gassertl(false, strs("write file failed: ", path, " errno: ", errno));
	}

	std::fclose(f);
	return bytesWrite == 1;
}

// move version
bool store(const string &path, stream &&s, directoryType type, bool compress) {
	return store(path, std::forward<stream&>(s), type, compress);
}

// remove file / archive file
bool remove(const string &path, directoryType type) {
	bool vfs, valid;
	string filepath, id;
	std::tie(filepath, id, vfs, valid) = _resolveLocation(path, type, true);

	if (!valid) {
		gassertl(false, strs("could not resolve file location: ", path));
		return false;
	}

	if (vfs) {
		auto v = _vfs.find(filepath);
		gassert(v != _vfs.end(), strs("vfs index not found: ", path));
		if (v != _vfs.end())
			return _vfs_remove(v->second, id);
		return false;
	}

	int err = std::remove(filepath.c_str());
	if (err != 0) {
		gassertl(false, strs("error deleting file: ", filepath, " errno: ", errno));
	}

	// remove archive index also
	auto v = _vfs.find(filepath);
	if (v != _vfs.end())
		_vfs.erase(v);
	return err == 0;
}

// checks if archive file/file exists
bool exists(const string &name, directoryType type) {
	bool vfs, valid;
	string filepath, id;
	std::tie(filepath, id, vfs, valid) = _resolveLocation(name, type, true);

	if (!valid)
		return false;

	if (vfs) {
		auto v = _vfs.find(filepath);
		gassert(v != _vfs.end(), strs("vfs index not found: ", name));
		if (v != _vfs.end())
			return _vfs_exists(v->second, id);
		return false;
	}

	return true;
}

}}}
