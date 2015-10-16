/*
 * granite engine 1.0 | 2006-2015 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
 * file: string detail
 * created: 26-03-2015
 *
 * description: detail implementation for string
 * note: should not be included directly (only for string.hpp)
 *
 * changelog:
 * - 26-03-2015: file created
 */

namespace detail {

//- detail / conversions from string -
template <typename T> T strToSigned(const stringRange &range) {
	string::const_iterator it(range.begin);
	T sign, ret = 0;
	if ((*range.begin) == '-') {
		it++;
		sign = -1;
	}
	else sign = 1;
	for(; it != range.end; ++it) {
		gassert(isDigit(*it), "parsing string to integer - non numeric character");
		ret = ret * 10 + getAlphaToDigit(*it);
	}
	return ret * sign;
}

template <typename T> T strToUnsigned(const stringRange &range) {
	T ret = 0;
	for (string::const_iterator it(range.begin); it != range.end; ++it) {
		gassert(isDigit(*it), "parsing string to integer - non numeric character");
		ret = ret * 10 + getAlphaToDigit(*it);
	}
	return ret;
}

template <typename T> T strToReal(const stringRange &range) {
	string::const_iterator it(range.begin);
	T sign;
	if ((*range.begin) == '-') {
		++it;
		sign = -static_cast<T>(1.0);
	}
	else sign = static_cast<T>(1.0);
	T ret = static_cast<T>(0.0);
	for (; it!=range.end; it++) {
		if (*it == '.'){
			++it;
			break;
		}
		ret = ret * static_cast<T>(10.0) + static_cast<T>(getAlphaToDigit(*it));
		gassert(isDigit(*it), "parsing string to real - non numeric character");
	}
	T mant = static_cast<T>(0.0), fact=static_cast<T>(1.0);
	for (; it!=range.end; it++){
		fact *= static_cast<T>(0.1);
		mant += static_cast<T>(getAlphaToDigit(*it)) * fact;
		gassert(isDigit(*it), "parsing string to real - non numeric character");
	}
	return (mant + ret) * sign;
}

inline bool strToBool(const stringRange &range) {
	string::const_iterator it(range.begin);
	if (it != range.end && (*it) != 't') return false; ++it;
	if (it != range.end && (*it) != 'r') return false; ++it;
	if (it != range.end && (*it) != 'u') return false; ++it;
	if (it != range.end && (*it) != 'e') return false; ++it;
	if (it == range.end) return true;
	return false;
}

//- detail / conversions to string -
template <typename T> stringRange signedToStr(const T &i, string &os) {
	string::iterator p = os.begin() + os.size() - 1;
	if (i == 0) {
		gassert(p >= os.begin(), "signed to string - index out of buffer");
		*p = '0';
		return stringRange(p, os.end());
	}
	T v = i < 0 ? -i : i;
	while (v) {
		gassert(p + 1 >= os.begin(), "unsigned to string - index out of buffer");
		*p = getDigitToAlpha(v % 10);
		v /= 10;
		p--;
	}
	if (i < 0) {
		*p = '-';
		p--;
	}
	gassert(p + 1 >= os.begin(), "unsigned to string - index out of buffer");
	return stringRange(p + 1, os.end());
}

template <typename T> stringRange unsignedToStr(const T &u, string &os) {
	string::iterator p = os.begin() + os.size() - 1;
	if (u == 0) {
		gassert(p >= os.begin(), "unsigned to string - index out of buffer");
		*p = '0';
		return stringRange(p, os.end());
	}
	T v = u;
	while (v) {
		gassert(p + 1 >= os.begin(), "unsigned to string - index out of buffer");
		*p = getDigitToAlpha(v % 10);
		v /= 10;
		p--;
	}
	gassert(p + 1 >= os.begin(), "unsigned to string - index out of buffer");
	return stringRange(p + 1, os.end());
}

template <typename T> stringRange realToStr(T rr, string &os, int precision) {
	gassert(size_t(precision + 2) <= os.size(), "real to string - buffer too small");
	string::iterator p = os.end() - 1 - precision;
	T r = rr < static_cast<T>(0.0) ? -rr : rr;
	uint32 i = uint32(r);

	// dot
	*p = '.';
	++p;

	// mantissa .###
	r -= T(i);
	int numsLeft = precision;
	while (numsLeft--) {
		char dig = char(r *= static_cast<T>(10.0));
		*p = getDigitToAlpha(dig);
		r -= T(dig);
		++p;
	}

	// base ###.
	p = os.end() - precision - 2;
	if (i == 0) {
		gassert(p >= os.begin(), "real to string - index out of buffer");
		*p = '0';
		--p;
	}
	while (i) {
		gassert(p >= os.begin(), "real to string - index out of buffer");
		*p = getDigitToAlpha(i % 10);
		i /= 10;
		--p;
	}

	// sign
	if (rr < static_cast<T>(0.0)) {
		gassert(p >= os.begin(), "real to string - index out of buffer");
		*p = '-';
		--p;
	}

	gassert(p + 1 <= os.end(), "real to string - index out of buffer");
	return stringRange(p + 1, os.end());
}

inline string boolToStr(const bool &b) {
	return string(b ? "true" : "false");
}

inline stringRange boolToStr(const bool &b, string &os) {
	static const string tt = "true";
	static const string ff = "false";
	return stringRange(b ? tt : ff);
}

//- detail / tests -
inline bool isInteger(const stringRange &range) {
	string::const_iterator it(range.begin);
	if ((*range.begin) == '-') {
		it++;
		if (it == range.end)
			return false;
	}
	for (; it != range.end; ++it) {
		if (!isDigit(*it))
			return false;
	}
	return true;
}

inline bool isFloat(const stringRange &range) {
	string::const_iterator it(range.begin);
	if ((*range.begin) == '-') {
		++it;
		if (it == range.end)
			return false;
	}
	bool dotFound = false;
	for (; it != range.end; it++) {
		if (*it == '.' && !dotFound){
			++it;
			dotFound = true;
		}
		if (!isDigit(*it))
			return false;
	}
	return true;
}

} // namespace detail

//- stringRange
bool stringRange::operator==(const stringRange &s) const {
	size_t c1 = s.count();
	if (c1 != count())
		return false;
	for (size_t i = 0; i < c1; ++i)
		if (*(s.begin + i) != *(begin + i))
			return false;
	return true;
}

bool stringRange::operator==(const string &s) const {
	size_t c1 = s.size();
	if (c1 != count())
		return false;
	for (size_t i = 0; i < c1; ++i)
		if (s[i] != *(begin + i))
			return false;
	return true;
}

//- size estimator -
// required space for string estimator for basic types + variadic version for multiple args
inline size_t estimateSize() { return 0; } // just to terminate variadic
template<> inline size_t estimateSize(const char &) { return 1; }
template<> inline size_t estimateSize(const uint8 &) { return 3; }
template<> inline size_t estimateSize(const int16 &) { return 6; }
template<> inline size_t estimateSize(const uint16 &) { return 5; }
template<> inline size_t estimateSize(const int32 &) { return 11; }
template<> inline size_t estimateSize(const uint32 &) { return 10; }
template<> inline size_t estimateSize(const int64 &) { return 21; }
template<> inline size_t estimateSize(const uint64 &) { return 20; }
template<> inline size_t estimateSize(const float &) { return 2 + floatPrecision + 10; }
template<> inline size_t estimateSize(const double &) { return 2 + floatPrecision + 10; }
template<> inline size_t estimateSize(const bool & v) { return v ? 4 : 5; }
template<> inline size_t estimateSize(const string &v) { return v.size(); }
template<> inline size_t estimateSize(const long unsigned int &) { return 20; }
template<> inline size_t estimateSize(const long int &) { return 21; }
template<> inline size_t estimateSize(const stringRange &s) { return s.count(); }
inline size_t estimateSize(const char * const v) { return strlen(v); }

template <typename T, typename... Args> size_t estimateSize(const T &v, const Args&... args) {
	return estimateSize(args...) + estimateSize(v);
}

//- tests -
// testing if string can be converted to T
template<> inline bool strIs<float>(const stringRange &range) { return detail::isFloat(range); }
template<> inline bool strIs<double>(const stringRange &range) { return detail::isFloat(range); }
template<> inline bool strIs<int>(const stringRange &range) { return detail::isInteger(range); }
template<> inline bool strIs<uint>(const stringRange &range) { return detail::isInteger(range); }
template<> inline bool strIs<char>(const stringRange &range) { return range.count() == 1; }
template<> inline bool strIs<uint8>(const stringRange &range) { return detail::isInteger(range); }
template<> inline bool strIs<int16>(const stringRange &range) { return detail::isInteger(range); }
template<> inline bool strIs<uint16>(const stringRange &range) { return detail::isInteger(range); }
template<> inline bool strIs<int64>(const stringRange &range) { return detail::isInteger(range); }
template<> inline bool strIs<uint64>(const stringRange &range) { return detail::isInteger(range); }
template<> inline bool strIs<long unsigned int>(const stringRange &range) { return detail::isInteger(range); }
template<> inline bool strIs<long int>(const stringRange &range) { return detail::isInteger(range); }
template<> inline bool strIs<bool>(const stringRange &range) { return range.str() == "true" || range.str() == "false"; }

//- from string -
template<> inline char fromStr<char>(const stringRange &range) { return *range.begin; }
template<> inline uint8 fromStr<uint8>(const stringRange &range) { return detail::strToUnsigned<uint8>(range); }
template<> inline int16 fromStr<int16>(const stringRange &range) { return detail::strToSigned<int16>(range); }
template<> inline uint16 fromStr<uint16>(const stringRange &range) { return detail::strToUnsigned<uint16>(range); }
template<> inline int32 fromStr<int32>(const stringRange &range) { return detail::strToSigned<int32>(range); }
template<> inline uint32 fromStr<uint32>(const stringRange &range) { return detail::strToUnsigned<uint32>(range); }
template<> inline int64 fromStr<int64>(const stringRange &range) { return detail::strToSigned<int64>(range); }
template<> inline uint64 fromStr<uint64>(const stringRange &range) { return detail::strToUnsigned<uint64>(range); }
template<> inline long int fromStr<long int>(const stringRange &range) { return detail::strToSigned<long int>(range); }
template<> inline long unsigned int fromStr<long unsigned int>(const stringRange &range) { return detail::strToUnsigned<long unsigned int>(range); }
template<> inline float fromStr<float>(const stringRange &range) { return detail::strToReal<float>(range); }
template<> inline double fromStr<double>(const stringRange &range) { return detail::strToReal<double>(range); }
template<> inline bool fromStr<bool>(const stringRange &range) { return detail::strToBool(range); }

//- to string -
template<> inline stringRange toStr(const char &i, string &os) { os[0] = i; return stringRange(os.begin(), os.begin() + 1); }
template<> inline stringRange toStr(const uint8 &i, string &os) { return detail::unsignedToStr<uint8>(i, os); }
template<> inline stringRange toStr(const int16 &i, string &os) { return detail::signedToStr<int16>(i, os); }
template<> inline stringRange toStr(const uint16 &i, string &os) { return detail::unsignedToStr<uint16>(i, os); }
template<> inline stringRange toStr(const int32 &i, string &os) { return detail::signedToStr<int32>(i, os); }
template<> inline stringRange toStr(const uint32 &i, string &os) { return detail::unsignedToStr<uint32>(i, os); }
template<> inline stringRange toStr(const int64 &i, string &os) { return detail::signedToStr<int64>(i, os); }
template<> inline stringRange toStr(const uint64 &i, string &os) { return detail::unsignedToStr<uint64>(i, os); }
template<> inline stringRange toStr(const long int &i, string &os) { return detail::signedToStr<long int>(i, os); }
template<> inline stringRange toStr(const long unsigned int &i, string &os) { return detail::unsignedToStr<long unsigned int>(i, os); }
template<> inline stringRange toStr(const float &i, string &os) { return detail::realToStr<float>(i, os, floatPrecision); }
template<> inline stringRange toStr(const double &i, string &os) { return detail::realToStr<double>(i, os, floatPrecision); }
template<> inline stringRange toStr(const bool &i, string &os) { return detail::boolToStr(i, os); }
template<> inline stringRange toStr(const stringRange &s, string &os) { return s; }
inline stringRange toStr(const string &ss, string &os) { return stringRange(ss); }
inline stringRange toStr(const char *cs, string &os) { size_t len = strlen(cs); os.resize(len); std::copy(cs, cs + len, os.begin()); return stringRange(os.begin(), os.begin() + len); }

// custom specializations for strings
inline string toStr(const string &s) { return s; }
inline string toStr(const char *s) { return string(s); }

//- detail / strs, strf -
namespace detail {
inline void strf_(string &buffer, string &out, string::const_iterator format, string::const_iterator formatEnd) {
	while (format != formatEnd) {
		if (*format == '%') {
			gassert(*(format + 1) == '%', "formatted string: missing function arguments");
			if(*(format + 1) == '%')
				++format;
		}
		out.append(1, *format);
		++format;
	}
}

template <typename T, typename... Args> void strf_(string &buffer, string &out, string::const_iterator format,
												   string::const_iterator formatEnd, const T &v, const Args&... args) {
	while (format != formatEnd) {
		if (*format == '%') {
			if (*(format + 1) == '%') // replace %% -> %
				++format;
			else {
				buffer.resize(std::max(buffer.size(), estimateSize(v)));
				stringRange r = toStr(v, buffer);
				out.append(r.begin, r.end);
				strf_(buffer, out, format + 1, formatEnd, args...);
				return;
			}
		}
		out.append(1, *format);
		++format;
	}
	gassert(false, "formatted string: extra arguments passed to function");
}

template <typename T> void strs_(string &buffer, string &out, const T &v) {
	buffer.resize(std::max(buffer.size(), estimateSize(v)));
	stringRange r = toStr(v, buffer);
	out.append(r.begin, r.end);
}
template <typename T, typename... Args> void strs_(string &buffer, string &out, const T &val, const Args&... args) {
	strs_(buffer, out, val);
	strs_(buffer, out, args...);
}
}

//- strs, strf -
template <typename... Args> string strf(const string &format, const Args&... args) {
	size_t s = format.size();
	string ret, buffer;
	ret.reserve(s + estimateSize(args...));
	detail::strf_(buffer, ret, format.begin(), format.end(), args...);
	return ret;
}

template <typename... Args> string strs(const Args&... args) {
	string buff, ret;
	ret.reserve(estimateSize(args...));
	detail::strs_(buff, ret, args...);
	return ret;
}

// TODO: thread safety (buffer in strs and strf)
//~
