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
	stream(size_t size = 0);
	~stream();

	// TODO: vectors stuff

	// rw pointer stuff
	size_t read(void *data, size_t size);
	void write(const void *data, size_t size);
	void setPosFromBegin(size_t bytes);
	void setPosFromEnd(size_t bytes);
	void setPosOffset(size_t bytes);
	size_t getPos() const;
	std::vector<uint8> &&take();

	template <typename T> size_t read(T &out);
	template <typename T> void write(const T &in);
};

#include "stream.inc.h"

}}
