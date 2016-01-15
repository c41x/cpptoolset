/*
 * granite engine 1.0 | 2006-2015 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
 * file: simd_vector
 * created: 19-08-2015
 *
 * description: aligned vector with some functions for better SSE programming support
 *
 * changelog:
 * - 19-08-2015: file created
 */

#pragma once
#include "includes.hpp"

namespace granite { namespace base {

#define SSE_SET8(I0, I1, I2, I3, I4, I5, I6, I7, I8, I9, I10, I11, I12, I13, I14, I15) _mm_set_epi8(I15, I14, I13, I12, I11, I10, I9, I8, I7, I6, I5, I4, I3, I2, I1, I0)

template <typename T, int padding = 0> // padding (number of additional elements)
class simd_vector {
	T *_data;
	size_t _size;
	size_t _psize;
	size_t _allocSize;

	// sets padding to zero
	void zero_padding() {
		memset(_data + _size, 0, (_psize - _size) * sizeof(T));
	}

public:
	simd_vector() : _data(nullptr), _size(0), _psize(0), _allocSize(0) { }

	simd_vector(const simd_vector &r) {
		_data = (T*)_mm_malloc(r._psize, 16);
		_psize = r._psize;
		_size = r._size;
		_allocSize = _psize;
		memcpy(_data, r._data, _psize);
		zero_padding();
	}

	simd_vector(simd_vector &&m) : _data(m._data), _size(m._size), _psize(m._psize) {
		m._data = nullptr;
	}

	~simd_vector() {
		_mm_free(_data);
		_data = nullptr;
		_size = _psize = _allocSize = 0;
	}

	simd_vector &operator=(simd_vector v) {
		std::swap(_data, v._data);
		std::swap(_psize, v._psize);
		std::swap(_size, v._size);
		std::swap(_allocSize, v._allocSize);
		return *this;
	}

	void resize(size_t s) {
		if (s > _size) {
			size_t oldSize = _size;
			T *oldData = _data;
			_psize = _allocSize = s + padding;
			_size = s;
			_data = (T*)_mm_malloc(_psize * sizeof(T), 16);
			if (oldSize != 0) {
				memcpy(_data, oldData, oldSize * sizeof(T));
				_mm_free(oldData);
			}
			zero_padding();
		}
	}

	void resize(size_t s, const T &v) {
		size_t oldSize = _size;
		resize(s);
		for (size_t i = oldSize; i < _size; ++i)
			_data[i] = v;
	}

	T *erase(T *position) {
		memmove(position, position + 1, (end() - position) * sizeof(T));
		--_size;
		--_psize;
		return position;
	}

	T *erase(T *first, T *last) {
		size_t size = last - first;
		memmove(first, last, (end() - last) * sizeof(T));
		_size -= size;
		_psize -= size;
		return first;
	}

	void shrink_to_fit() {
		if (_psize != _allocSize) {
			T *newData = (T*)_mm_malloc(_psize * sizeof(T), 16);
			_allocSize = _psize;
			memcpy(newData, _data, _psize * sizeof(T));
			_mm_free(_data);
			_data = newData;
			zero_padding();
		}
	}

	T *data() { return _data; }
	const T *data() const { return _data; }

	void assign(const T *const begin, const T *const end) {
		resize(std::distance(begin, end));
		memcpy(_data, begin, _size * sizeof(T));
	}

	size_t size() const { return _size; }

	T &front() { return *_data; }
	const T &front() const { return *_data; }
	T &back() { return *(_data + _size - 1); }
	const T &back() const { return *(_data + _size - 1); }

	// STL integration
	T *begin() { return _data; }
	const T *begin() const { return _data; }
	const T *cbegin() const { return _data; }
	T *end() { return _data + _size; }
	const T *end() const { return _data + _size; }
	const T *cend() const { return _data + _size; }
};

// STL...
template<typename T> T *begin(simd_vector<T> &v) { return v.begin(); }
template<typename T> const T *begin(simd_vector<T> &v) { return v.begin(); }
template<typename T> T *end(simd_vector<T> &v) { return v.end(); }
template<typename T> const T *end(simd_vector<T> &v) { return v.end(); }

}}

// TODO: push_back?
// TODO: custom value for padding? now padding is memset to 0
// TODO: asserts
