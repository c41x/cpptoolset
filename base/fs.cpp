#include "fs.h"
#include "string.h"

namespace granite { namespace base { namespace fs {

namespace {
std::vector<string> _files;
}

bool open(const string &vfs, directoryType type) {
	return true;
}

void close() {

}

const std::vector<string> &getFileList() {
	return _files;
}

stream load(const string &path) {
	std::FILE *f = std::fopen(path.c_str(), "r");
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

bool store(const string &path, stream &s, bool compress) {
	std::FILE *f = std::fopen(path.c_str(), "w");
	if (f == NULL) {
		gassert(false, strs("could not open file: ", path));
		return false;
	}

	size_t bytesWrite = std::fwrite(path.data(), path.size(), 1, f);
	gassert(bytesWrite == 1, strs("write file failed: ", path));

	std::fclose(f);
	return bytesWrite == 1;
}

bool remove(const string &path) {
	int err = std::remove(path.c_str());
	gassert(err == 0, strs("error deleting file: ", path));
	return err == 0;
}

}}}
