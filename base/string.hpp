/*
 * granite engine 1.0 | 2006-2015 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
 * file: string.h
 * created: 16-01-2013
 *
 * description: string utilities & conversions
 *
 * changelog:
 * - 16-01-2013: file created
 * - 20-01-2013: added char helper functions
 * - 26-01-2013:
 *   * first complete working version
 *   * added stringRange
 *   * conversion functions
 *   * string utilities functions
 * - 27-01-2013:
 *   * formatted string function strf and strs
 *   * benchmark optimalizations
 */

#pragma once

#include "includes.hpp"
#include "log.hpp"

namespace granite { namespace base {

// 6 by default
extern int8 floatPrecision;

// 2 iterators for substring positions in string
struct stringRange {
private:
	explicit stringRange(string &&s) = delete;
	explicit stringRange(const string &&s) = delete;
public:
	string::const_iterator begin;
	string::const_iterator end;
	stringRange(string::const_iterator ibegin, string::const_iterator iend) : begin(ibegin), end(iend) { gassert(ibegin <= iend, "invalid range pointers positions"); }
	explicit stringRange(const string &s) : begin(s.begin()), end(s.end()) {}
	stringRange &operator()(string::const_iterator ibegin, string::const_iterator iend) { gassert(ibegin <= iend, "invalid range pointers positions"); begin = ibegin; end = iend; return *this; }
	stringRange &operator()(const string &s) { begin = s.begin(); end = s.end(); return *this; }
	size_t ibegin(const string &s) const { return std::distance(s.begin(), begin); }
	size_t iend(const string &s) const { return std::distance(s.begin(), end); }
	size_t count() const { return std::distance(begin, end); }
	string str() const { return string(begin, end); }
	operator string() const { return string(begin, end); }
	inline bool operator==(const stringRange &s) const;
	inline bool operator==(const string &s) const;
	bool operator!=(const stringRange &s) const { return !(*this == s); }
	bool operator!=(const string &s) const { return !(*this == s); }
};

// inlines char fxs:
inline bool isDigit(char c) { return c >= '0' && c <= '9'; }
inline bool isAlpha(char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }
inline bool isAlphaNumeric(char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'); }
inline bool isUpper(char c) { return c >= 'A' && c <= 'Z'; }
inline bool isLower(char c) { return c >= 'a' && c <= 'z'; }
inline bool setUpper(char &ioC) { if(!isAlpha(ioC) || isUpper(ioC)) return false; ioC-='z'-'Z'; return true; }
inline bool setLower(char &ioC) { if(!isAlpha(ioC) || isLower(ioC)) return false; ioC+='z'-'Z'; return true; }
inline char getUpper(char c) { return c - 'z' - 'Z'; }
inline char getLower(char c) { return c + 'z' - 'Z'; }
inline bool isWhiteSpace(char c) { return (c == ' ' || c == '\n' || c == '\t' || c == '\v' || c == '\r'); }
inline bool isLineBreak(char c) { return c == '\n' || c == '\r'; }
inline bool isHex(char c) { return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'); }
inline bool isOct(char c) { return c >= '0' && c <= '7'; }
inline bool isBin(char c) { return (c == '0' || c == '1'); }
inline char getAlphaToDigit(char c) { return (c - '0'); }
inline char getDigitToAlpha(char ii) { return (ii + '0'); }

// string utilities
void lowerCase(string &s);
void upperCase(string &s);
void trimWhitespaces(string &s);
void findAndDelete(string &s, const string &what);
void findAndReplace(string &s, const string &what, const string &replacement);
void findAndReplace(string &s, char what, char replacement);
void findAndCutAfter(string &s, const string &what);
void findAndCutBefore(string &s, const string &what);
void deleteWhitespaces(string &s);
bool containsSubstr(const string &s, const string &search);
std::vector<stringRange> divideString(const string &s, char divChar);
float matchString(const string &a, const string &b);
stringRange initToken(stringRange s);
stringRange nextToken(stringRange s, stringRange &tok);
bool endToken(stringRange s, stringRange tok);

// paths/file names string utilities
string extractFileName(const string &s);
string extractFilePath(const string &s);
string extractExt(const string &s);
string changeExt(const string &s, const string &ext);
string cutLongPath(const string &s);

// required space for string estimator
template <typename T> size_t estimateSize(const T &v) { return T::SFINAE_template_not_specialized_estimateSize; }

// testing functions - returns if given string can be converted to type T
template <typename T> bool strIs(const stringRange &) { return T::SFINAE_template_not_specialized_strIs; }
template <typename T> bool strIs(const string &s) { return strIs<T>(stringRange(s)); }

// conversions: from string to T
template<typename T> T fromStr(const stringRange &range) { return T::SFINAE_template_not_specialized_fromStr; }
template<typename T> T fromStr(const string &s) { return fromStr<T>(stringRange(s)); }

// conversions: from T to string
template <typename T> stringRange toStr(const T &, string &os) { return T::SFINAE_template_not_specialized_toStr; }
template <typename T> string toStr(const T &t) { string s; s.resize(estimateSize(t)); return toStr(t, s).str(); }

// string building
template<typename... Args> string strs(const Args&... args);
template<typename... Args> string strf(const string &format, const Args&... args);

// implementations
#include "string.inc.hpp"

}}
