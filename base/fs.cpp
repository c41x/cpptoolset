#include "fs.h"
#include "string.h"
#include "gstdlib.h"
#include "lz4.h"

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
bool _preferVFS = false;

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

string fullPath(directoryType type, const string &p = "") {
	#ifdef GE_PLATFORM_WINDOWS
	return getPath(type) + GE_DIR_SEPARATOR + _normalizePath(p);
	#else
	return getPath(type) + GE_DIR_SEPARATOR + p;
	#endif
}

bool _exists(const string &path) {
	struct stat buffer;
	return stat(path.c_str(), &buffer) == 0;
}

bool _exists_file(const string &file) {
	struct stat buffer;
	return stat(file.c_str(), &buffer) == 0 && S_ISREG(buffer.st_mode) != 0;
}

void _resize(std::FILE *f, size_t newSize) {
	#ifdef GE_PLATFORM_WINDOWS
	_chsize(_fileno(f), newSize);
	#else
	ftruncate(fileno(f), newSize);
	#endif
}

bool _mkdirtree(const string &pathBase, const string &path) {
	string p = pathBase + GE_DIR_SEPARATOR;
	for (auto &e : divideString(path, GE_DIR_SEPARATOR)) {
		p += e.str();
		if (!_exists(p)) {
			#ifdef GE_PLATFORM_WINDOWS
			if (0 != _mkdir(p.c_str()))
				return false;
			#else
			if (0 != mkdir(p.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
				return false;
			#endif
		}
		p += GE_DIR_SEPARATOR;
	}
	return true;
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


//- buffer pool
/*
const _minPool = 2;
const _poolSize = 1024 * 1024 * 16; // 16MB
std::vector<stream> _spool_s;
std::vector<size_t> _spool_free;

stream &_spool_get() {
}

void _spool_free(stream &s) {
	_spool.free
}
*/
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
};

// index map
std::map<string, vfs> _vfs;

// writes index to file (write pointer must be set before call)
void _vfs_write_index(vfs &v) {
	uint32 filesCount = v.files.size();
	std::fwrite(&filesCount, sizeof(uint32), 1, v.f);

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
	std::fwrite(s.data(), s.size(), 1, v.f);

	// trim file here
	_resize(v.f, std::ftell(v.f));
}

// (re) opens/creates vfs, initializes index
void _vfs_open(const string path) {
	std::cout << "opening archive: " << path << std::endl;
	if (!_exists_file(path)) {
		// create directory tree first (if needed)
		_mkdirtree("", extractFilePath(path));

		// create and initialize new vfs
		uint32 filesCount = 0;
		auto &v = _vfs[path];
		v.f = std::fopen(path.c_str(), "wb+");
		v.indexOffset = 4 + sizeof(uint64);
		v.dirty = false;
		std::fwrite("GFS2", 4, 1, v.f);
		std::fwrite(&v.indexOffset, sizeof(uint64), 1, v.f);
		std::fwrite(&filesCount, sizeof(uint32), 1, v.f);
	}
	else {
		auto vt = _vfs.find(path);
		if (vt == _vfs.end()) {
			// create index
			auto &v = _vfs[path];
			uint32 filesCount = 0;
			v.f = fopen(path.c_str(), "rb+");
			v.dirty = false;

			// read and validate header
			char id[4];
			std::fread(id, 4, 1, v.f);
			if (id[0] != 'G' || id[1] != 'F' || id[2] != 'S' || id[3] != '2')
				std::cout << "NOT AN VFS ARCHIVE!" << std::endl;
			std::fread(&v.indexOffset, sizeof(uint64), 1, v.f);

			// read file index in one chunk
			std::fseek(v.f, 0, SEEK_END);
			size_t size = std::ftell(v.f) - v.indexOffset;
			std::fseek(v.f, v.indexOffset, SEEK_SET);
			stream s;
			s.resize(size);
			std::fread(s.data(), size, 1, v.f);

			// read index
			s.read(filesCount);
			std::cout << filesCount << std::endl;
			while (filesCount-- > 0) {
				vfs_file fi;
				s.read(fi.id);
				s.read(fi.position);
				s.read(fi.size);
				s.read(fi.flags);
				s.read(fi.createTime);
				s.read(fi.modTime);
				std::cout << fi.id << ", " << fi.position << ", " << fi.size << ", " <<
					fi.flags << ", " << fi.createTime << ", " << fi.modTime << std::endl;
				v.files.push_back(fi);
			}
		}
		else {
			// already in index - reopen if not opened
			if (vt->second.f == nullptr)
				vt->second.f = fopen(path.c_str(), "rb+");
		}
	}
}

void _vfs_close(vfs &v) {
	if (v.dirty) {
		std::fseek(v.f, v.indexOffset, SEEK_SET);
		_vfs_write_index(v);

		// update header
		std::fseek(v.f, 4, SEEK_SET);
		std::fwrite(&v.indexOffset, sizeof(uint32), 1, v.f);
		v.dirty = false;
	}

	fclose(v.f);
	v.f = nullptr;
}

std::vector<vfs_file>::iterator _vfs_find_file(vfs &v, const string &id) {
	return std::find_if(v.files.begin(), v.files.end(),
						[&id](vfs_file &f) {
							return f.id == id;
						});
}

bool _vfs_exists(vfs &v, const string &id) {
	return v.files.end() != _vfs_find_file(v, id);
}

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
}

bool _vfs_remove(vfs &v, const string &id) {
	auto f = _vfs_find_file(v, id);
	if (f != v.files.end()) {
		// erase data from file
		stream buf(1024); // 1024 bytes buffer
		size_t readed = 1;
		size_t to = f->position, from = f->position + f->size;
		uint64 fPos = f->position;
		uint64 fSize = f->size;
		gassert(from > to, "error removing file: file index corrupted");
		while (readed != 0) {
			std::fseek(v.f, from, SEEK_SET);
			readed = std::fread(buf.data(), 1, buf.size(), v.f);
			std::fseek(v.f, to, SEEK_SET);
			std::fwrite(buf.data(), readed, 1, v.f);
			to += readed;
			from += readed;
		}

		// truncate file
		_resize(v.f, to);

		// remove from index
		v.files.erase(f);

		// update file index
		for (auto &fi : v.files) {
			if (fi.position > fPos)
				fi.position -= fSize;
		}

		// update index offset
		v.indexOffset -= fSize;

		// mark index as dirty
		v.dirty = true;
		return true;
	}
	else {
		// report error
		log::log(log::logLevelError, strs("could not delete: ", id, " from vfs - file not present in index"));
		return false;
	}
}

bool _vfs_add(vfs &v, const string &id, const stream &s, bool compress = true) {
	auto f = _vfs_find_file(v, id);

	// replace file (remove old file first)
	uint64 createTime = std::time(0);
	if (f != v.files.end()) {
		_vfs_remove(v, id);
		createTime = f->createTime; // preserve create time
	}

	// create file
	std::fseek(v.f, v.indexOffset, SEEK_SET);
	if (compress) {
		stream sc(LZ4_compressBound(s.size()));
		uint64 compressedSize = LZ4_compress_default((const char*)s.data(), (char*)sc.data(), s.size(), sc.size());
		int64 realSize = s.size();
		std::fwrite(&realSize, sizeof(int64), 1, v.f);
		std::fwrite(sc.data(), compressedSize, 1, v.f);
		v.files.push_back({id, v.indexOffset, compressedSize + sizeof(int64), 1, createTime, (uint64)std::time(0)});
	}
	else {
		std::fwrite(s.data(), s.size(), 1, v.f);
		v.files.push_back({id, v.indexOffset, s.size(), 0, createTime, (uint64)std::time(0)});
	}
	v.indexOffset = std::ftell(v.f);
	v.dirty = true;
	return true;
}

// returns full path to file, vfs id, is vfs, is valid
std::tuple<string, string, bool, bool> _resolveLocation(const string &ipath, directoryType type, bool mustExist) {
	std::tuple<string, string, bool, bool> r;

	auto resolveNormalFile = [&r, &type, &ipath, &mustExist]() {
		std::cout << "> resolving file for: " << ipath << std::endl;
		if (!mustExist || _exists_file(fullPath(type, ipath))) {
			std::cout << "  resolved [file]: " << fullPath(type, ipath) << std::endl;
			r = std::make_tuple(fullPath(type, ipath), "", false, true);
			return true;
		}
		return false;
	};

	auto resolveVFS = [&r, &type, &ipath]() {
		std::cout << "> resolving vfs for: " << ipath << std::endl;
		string base = getPath(type);
		string path = base;
		std::vector<stringRange> explodePath = divideString(ipath, '/');
		for (auto &s : explodePath) {
			base += strs(GE_DIR_SEPARATOR, s);
			path = base + ".gfs";
			std::cout << "  resolving path: " << path << " chunk: " << s.str() << std::endl;
			if (_vfs.find(path) != _vfs.end()) {
				std::cout << "  resolved [vfs]: " << path << std::endl;
				r = std::make_tuple(path, stringRange(s.end + 1, ipath.end()), true, true);
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
}

//- impl
string fileInfo::fullPath() const {
	return path == "" ? name : (path + GE_DIR_SEPARATOR + name);
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
		return true;
	}
	gassert(false, strs("specified path does not exists: ", path));
	return false;
}

void preferArchives(bool preferVFS) {
	_preferVFS = preferVFS;
}

void flush() {
	for (auto &v : _vfs) {
		_vfs_close(v.second);
		_vfs_open(v.first);
	}
}

bool createArchive(const string &path, directoryType type) {
	string fpath = fullPath(type, path);
	if (_exists_file(fpath))
		return false;
	_vfs_open(fpath);
	return true;
}

void initArchive(const string &path, directoryType type) {
	_vfs_open(fullPath(type, path));
}

// scan for vfs files and initialize all
void initAllArchives(directoryType type) {
	fileList fl = matchFiles(".*\\.gfs", "", type);
	for (const auto &f : fl) {
		std::cout << fullPath(type, f.fullPath()) << std::endl;
		_vfs_open(fullPath(type, f.fullPath()));
	}
}

void close() {
	for (auto &v : _vfs)
		_vfs_close(v.second);
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
	bool vfs, valid;
	string filepath, id;
	std::tie(filepath, id, vfs, valid) = _resolveLocation(path, type, false);

	if (vfs) {
		auto v = _vfs.find(filepath);
		gassert(v != _vfs.end(), strs("vfs index not found: ", path));
		if (v != _vfs.end())
			return _vfs_add(v->second, id, s, compress);
		return false;
	}

	std::FILE *f = std::fopen(fullPath(type, path).c_str(), "wb+");
	if (f == NULL) {
		_mkdirtree(getPath(type), extractFilePath(path));
		f = std::fopen(fullPath(type, path).c_str(), "wb+");
		if (f == NULL) {
			gassert(false, strs("could not open file: ", path));
			return false;
		}
	}

	size_t bytesWrite = std::fwrite(s.data(), s.size(), 1, f);
	gassert(bytesWrite == 1, strs("write file failed: ", path));

	std::fclose(f);
	return bytesWrite == 1;
}

bool remove(const string &path, directoryType type) {
	bool vfs, valid;
	string filepath, id;
	std::tie(filepath, id, vfs, valid) = _resolveLocation(path, type, true);

	if (!valid) {
		gassert(false, strs("could not resolve file location: ", path));
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
	gassert(err == 0, strs("error deleting file: ", path));
	return err == 0;
}

bool exists(const string &name, directoryType type) {
	bool vfs, valid;
	string filepath, id;
	std::tie(filepath, id, vfs, valid) = _resolveLocation(name, type, true);

	if (!valid) {
		gassert(false, strs("could not resolve file location: ", name));
		return false;
	}

	if (vfs) {
		auto v = _vfs.find(filepath);
		gassert(v != _vfs.end(), strs("vfs index not found: ", name));
		if (v != _vfs.end())
			return _vfs_exists(v->second, id);
		return false;
	}

	return false;
}

}}}

// TODO: rewrite vfs file by replace, not erase & add
// TODO: pools (buffer pool for decompression)
// TODO: asserts and logs
// TODO: extensions to compress (currently static)
// TODO: move semantics for streams
// TODO: integrate find, match with vfs...
