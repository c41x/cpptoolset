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

template<> inline vec4f fromStr<vec4f>(const stringRange &range) {
	auto fetchToken = [](auto begin, auto end) {
		for (auto i = begin; i != end; ++i) {
			if (isWhiteSpace(*i)) {
				if (i == begin)
					++begin;
				else return stringRange(begin, i);
			}
		}
		return stringRange(begin, end);
	};

	vec4f r;
	auto tok = fetchToken(range.begin, range.end);
	r.x = fromStr<float>(tok);
	tok = fetchToken(tok.end, range.end);
	r.y = fromStr<float>(tok);
	tok = fetchToken(tok.end, range.end);
	r.z = fromStr<float>(tok);
	tok = fetchToken(tok.end, range.end);
	r.w = fromStr<float>(tok);
	return r;
}

inline string toStr(const vec4f &v) {
	string r;
	r += toStr(v.x);
	r += " ";
	r += toStr(v.y);
	r += " ";
	r += toStr(v.z);
	r += " ";
	r += toStr(v.w);
	return r;
}

}}
