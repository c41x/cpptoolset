/*
 * granite engine 1.0 | 2006-2015 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
 * file: math_string.h
 * created: 25-03-2015
 *
 * description: conversions between math stuff and string
 * TODO: asserts
 *
 * changelog:
 * - 25-03-2015: file created
 */

#pragma once

#include "includes.hpp"
#include "string.hpp"
#include "math.hpp"
#include "gstdlib.hpp"

namespace granite { namespace base {

template<> inline size_t estimateSize(const vec4f &) {
	return estimateSize(float()) * 4 + 3;
}

template<> inline size_t estimateSize(const vec &) {
	return estimateSize(float()) * 4 + 3;
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

template<> inline bool strIs<vec>(const stringRange &s) { return strIs<vec4f>(s); }

template<> inline vec4f fromStr<vec4f>(const stringRange &s) {
	vec4f r;
	auto tok = initToken(s);
	r.x = fromStr<float>(nextToken(s, tok));
	r.y = fromStr<float>(nextToken(s, tok));
	r.z = fromStr<float>(nextToken(s, tok));
	r.w = fromStr<float>(nextToken(s, tok));
	return r;
}

template<> inline vec fromStr<vec>(const stringRange &s) { return fromStr<vec4f>(s); }

template<> inline stringRange toStr<vec4f>(const vec4f &v, string &os) {
	auto mark = os.begin();

	stringRange t = toStr(v.x, os);
	mark = copy_safe(t.begin, t.end, mark);
	*mark++ = ' ';

	t = toStr(v.y, os);
	mark = copy_safe(t.begin, t.end, mark);
	*mark++ = ' ';

	t = toStr(v.z, os);
	mark = copy_safe(t.begin, t.end, mark);
	*mark++ = ' ';

	t = toStr(v.w, os);
	mark = copy_safe(t.begin, t.end, mark);

	return stringRange(os.begin(), mark);
}

template<> inline stringRange toStr<vec>(const vec &v, string &os) { return toStr(vec4f(v), os); }

}}
