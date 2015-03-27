/*
 * granite engine 1.0 | 2006-2015 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
 * file: math_string.h
 * created: 25-03-2015
 *
 * description: conversions between math stuff and string
 *
 * changelog:
 * - 25-03-2015: file created
 */

#pragma once

#include "includes.h"
#include "string.h"
#include "math.h"

namespace granite { namespace base {

template<> inline size_t estimateSize(const vec4f &v) {
	return estimateSize<float>(v.x) * 4 + 3;
}

template<> inline bool strIs<vec4f>(const stringRange &s) {
	auto tok = initToken(s);
	int n = 4;
	while(n--) {
		if (endToken(s, tok))
			return false;
		if (!strIs<float>(nextToken(s, tok)))
			return false;
	}
	return endToken(s, tok);
}

template<> inline vec4f fromStr<vec4f>(const stringRange &s) {
	vec4f r;
	auto tok = initToken(s);
	r.x = fromStr<float>(nextToken(s, tok));
	r.y = fromStr<float>(nextToken(s, tok));
	r.z = fromStr<float>(nextToken(s, tok));
	r.w = fromStr<float>(nextToken(s, tok));
	return r;
}

inline string toStr(const vec4f &v) {
	return strs(v.x, " ", v.y, " ", v.z, " ", v.w);
}

}}
