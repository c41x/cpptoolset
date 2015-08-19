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

template <typename T, int padding>
class simd_vector {
	T *_data;
	size_t _size;
	size_t _psize;
public:
	simd_vector() : _data(nullptr), _size(0), _psize(0) {}
	~simd_vector() {
		_mm_free(_data);
		_data = nullptr;
		_size = _psize = 0;
	}

	void resize(size_t s) {
		if (s > _size) {
			_psize = s + padding;
			_size = s;
			_data = (T*)_mm_malloc(_psize, 16);
		}
	}

	T *data() { return _data; }
	const T *data() const { return _data; }

	void assign(const T *const begin, const T *const end) {
		resize(std::distance(begin, end));
		memcpy(_data, begin, _size);
	}

	size_t size() const { return _size; }
};

}}
