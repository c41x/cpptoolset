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
#include "includes.hpp"
#include "log.hpp"
#include "string.hpp"

namespace granite { namespace base {

class stream
{
    std::vector<uint8> _mem;
    size_t _pos;
public:
    inline stream(size_t size = 0);
    inline ~stream();
    inline stream(stream &&s);
    inline stream(const stream &s);
    inline stream &operator=(stream &&s);
    inline stream &operator=(const stream &s);

    // TODO: other vector fx

    // vectors stuff
    inline size_t size() const;
    inline void resize(size_t cap);
    inline void resize(size_t cap, const uint8 &val);
    inline void reserve(size_t cap);
    inline void clear();

    inline uint8 *data();
    inline const uint8 *data() const;

    // rw pointer stuff
    inline size_t read(void *data, size_t size);
    inline size_t read_const(void *data, size_t size) const;
    inline void write(const void *data, size_t size);
    inline void setPosFromBegin(size_t bytes);
    inline void setPosFromEnd(size_t bytes);
    inline void setPosOffset(size_t bytes);
    inline size_t getPos() const;
    inline void expand(size_t additional_cap);

    template <typename T> inline size_t read(T &out);
    template <typename T> inline void write(const T &in);
    template <typename T> inline size_t read(std::vector<T> &out);
    template <typename T> inline void write(const std::vector<T> &in);
};

class const_stream {
    const void *_data;
    const size_t _size;
public:
    inline const_stream(stream &&s);
    inline const_stream(const stream &s);
    inline const_stream(const void *m, const size_t s);
    inline ~const_stream();
    inline size_t size() const;
    inline const uint8 *data() const;
};

#include "stream.inc.hpp"

}}
