#include "fs.h"
#include "string.h"

namespace granite { namespace base { namespace fs {

namespace {
std::vector<string> _files;
std::FILE *_f;
}

bool open(const string &vfs) {
	// TODO: open vfs
	return true;
}

void close() {

}

const std::vector<string> &getFileList() {
	return _files;
}

stream load(const string &path) {
	_f = std::fopen(path.c_str(), "r");
	if (_f == NULL) {
		gassert(false, strs("could not open file: ", path));
		return stream();
	}

	std::fseek(_f, 0, SEEK_END);
	size_t size = std::ftell(_f);
	std::rewind(_f);

	stream s;
	s.resize(size);
	size_t readCount = std::fread(s.data(), size, 1, _f);
	gassert(readCount == 1, strs("read file failed: ", path));

	std::fclose(_f);
	return s;
}

bool store(const string &path, stream &s, bool compress) {
	_f = std::fopen(path.c_str(), "w");
	if (_f == NULL) {
		gassert(false, strs("could not open file: ", path));
		return false;
	}

	size_t bytesWrite = std::fwrite(path.data(), path.size(), 1, _f);
	gassert(bytesWrite == 1, strs("write file failed: ", path));

	std::fclose(_f);
	return bytesWrite == 1;
}

bool remove(const string &path) {
	int err = std::remove(path.c_str());
	gassert(err == 0, strs("error deleting file: ", path));
	return err == 0;
}

}}}
