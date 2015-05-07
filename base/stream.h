/*
 * granite engine 1.0 | 2006-2015 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
 * file: stream
 * created: 07-05-2015
 *
 * description: stream buffer
 *
 * changelog:
 * - 01-12-2008: create
 * - 07-05-2015: complete rewrite
 */

#pragma once
#include "includes.h"

namespace granite { namespace base {

// TODO: alignas
class stream
{
	std::vector<uint8> _mem;
	size_t _pos;
public:
	stream(size_t size = 0) {
		if (size > 0)
			_mem.resize(size);
		_pos = 0;
	}

	~stream() {}

	// TODO: vectors stuff

	// rw pointer stuff
	size_t read(void *data, size_t size) {
		size_t n = std::min(_mem.size() - _pos, size);
		memcpy(data, &_mem[_pos], n);
		_pos += n;
		return n;
	}

	void write(const void *data, size_t size) {
		size_t need = std::max(_mem.size(), _pos + size);
		_mem.resize(need);
		memcpy(&_mem[_pos], data, size);
		_pos += size;
	}

	void setPosFromBegin(size_t bytes) {
		_pos = bytes;
	}

	void setPosFromEnd(size_t bytes) {
		gassert(_mem.size() >= bytes, "index out of range");
		_pos = _mem.size() - bytes;
	}

	void setPosOffset(size_t bytes) {
		gassert(_pos + bytes < _mem.size(), "index out of range");
		_pos += bytes;
	}

	size_t getPos() const {
		return _pos;
	}

	std::vector<uint8> &&take() {
		return std::move(_mem);
	}

	template <typename T> size_t read(T &out) {
		return read(&out, sizeof(out));
	}

	template <typename T> void write(const T &in) {
		write(&in, sizeof(in));
	}
};

template <> size_t stream::read(string &s) {
	size_t len;
	size_t r = read<size_t>(len);
	s.resize(len);
	return read(&s[0], len) + r;
}

template <> void stream::write(const string &s) {
	write<size_t>(s.size());
	write(&s[0], s.size());
}

}}
