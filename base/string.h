/*
 * granite engine 1.0 | 2006-2013 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com 
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

#include "includes.h"
#include "math.h"
#include "log.h"

namespace granite{namespace base{

// 2 iterators for substring positions in string
struct stringRange{
	string::const_iterator begin;
	string::const_iterator end;
	stringRange(string::const_iterator ibegin,string::const_iterator iend):begin(ibegin),end(iend){}
	explicit stringRange(const string &s):begin(s.begin()),end(s.end()){}
	stringRange &operator()(string::const_iterator ibegin,string::const_iterator iend){begin=ibegin;end=iend;return *this;}
	stringRange &operator()(const string &s){begin=s.begin();end=s.end(); return *this;}
	size_t ibegin(const string &s)const{return std::distance(s.begin(),begin);}
	size_t iend(const string &s)const{return std::distance(s.begin(),end);}
	size_t count()const{return std::distance(begin,end);}
	string str()const{return string(begin,end);}
	operator string()const{return string(begin,end);}
};

// inlines char fxs:
inline bool isDigit(char c){return c>='0'&&c<='9';}
inline bool isAlpha(char c){return (c>='A'&&c<='Z') || (c>='a'&&c<='z');}
inline bool isAlphaNumeric(char c){return (c>='A'&&c<='Z') || (c>='a'&&c<='z') || (c>='0'&&c<='9');}
inline bool isAlphaPL(char c){if((c>='A'&&c<='Z') || (c>='a'&&c<='z')) return true; return (c=='¡'||c=='±'||c=='Ê'||c=='ê'||c=='Ó'||c=='ó'||c=='£'||c=='³'||c=='æ'||c=='Æ'||c=='¦'||c=='¶'||c=='¯'||c=='¿'||c=='¬'||c=='¼'||c=='Ñ'||c=='ñ');}
inline bool isAlphaNumericPL(char c){if((c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9'))return true;return (c=='¡'||c=='±'||c=='Ê'||c=='ê'||c=='Ó'||c=='ó'||c=='£'||c=='³'||c=='æ'||c=='Æ'||c=='¦'||c=='¶'||c=='¯'||c=='¿'||c=='¬'||c=='¼'||c=='Ñ'||c=='ñ');}
inline bool isUpper(char c){return c>='A'&&c<='Z';}
inline bool isLower(char c){return c>='a'&&c<='z';}
inline bool setUpper(char &ioC){if(!isAlpha(ioC)||isUpper(ioC))return false; ioC-='z'-'Z';return true;}
inline bool setLower(char &ioC){if(!isAlpha(ioC)||isLower(ioC))return false; ioC+='z'-'Z';return true;}
inline char getUpper(char c){return c-'z'-'Z';}
inline char getLower(char c){return c+'z'-'Z';}
inline bool isWhiteSpace(char c){return (c==' '||c=='\n'||c=='\t'||c=='\v'||c=='\r');}
inline bool isLineBreak(char c) { return c == '\n' || c == '\r'; }
inline bool isHex(char c){return (c>='0'&&c<='9')||(c>='A'&&c<='F')||(c>='a'&&c<='f');}
inline bool isOct(char c){return c>='0'&&c<='7';}
inline bool isBin(char c){return (c=='0'||c=='1');}
inline char getAlphaToDigit(char c){return (c-'0');}
inline char getDigitToAlpha(char ii){return (ii+'0');}

// string utilities
void lowerCase(string &s);
void upperCase(string &s);
void trimWhitespaces(string &s);
void findAndDelete(string &s,const string &what);
void findAndReplace(string &s,const string &what,const string &replacement);
void findAndCutAfter(string &s,const string &what);
void findAndCutBefore(string &s,const string &what);
void deleteWhitespaces(string &s);
bool containsSubstr(const string &s,const string &search);
void divideString(const string &s,char divChar,std::vector<string> &result);
void divideString(const string &s,char divChar,std::vector<stringRange> &result);
float matchString(const string &a,const string &b);

// paths/file names string utilities
string extractFileName(const string &s);
string extractFilePath(const string &s);
string extractExt(const string &s);
string changeExt(const string &s,const string &ext);
string cutLongPath(const string &s);

// conversions: from string to T
// fast iterator versions:
template <typename T>T strToSigned(const stringRange &range);
template <typename T>T strToUnsigned(const stringRange &range);
template <typename T>T strToReal(const stringRange &range);
bool strToBool(const stringRange &range);

// testing functions
inline bool isInteger(const stringRange &range);
inline bool isFloat(const stringRange &range);
inline bool isInteger(const string &s) { return isInteger(stringRange(s)); };
inline bool isFloat(const string &s) { return isFloat(stringRange(s)); };

// templated (can not be overloaded thats why template)
template<typename T>inline T fromStr(const stringRange &range){gassert(false,"conversion from string: unknown type, conversion not specialized"); return T();}
template<>inline int8 fromStr<int8>(const stringRange &range){return strToSigned<int8>(range);}
template<>inline uint8 fromStr<uint8>(const stringRange &range){return strToUnsigned<uint8>(range);}
template<>inline int16 fromStr<int16>(const stringRange &range){return strToSigned<int16>(range);}
template<>inline uint16 fromStr<uint16>(const stringRange &range){return strToUnsigned<uint16>(range);}
template<>inline int32 fromStr<int32>(const stringRange &range){return strToSigned<int32>(range);}
template<>inline uint32 fromStr<uint32>(const stringRange &range){return strToUnsigned<uint32>(range);}
template<>inline int64 fromStr<int64>(const stringRange &range){return strToSigned<int64>(range);}
template<>inline uint64 fromStr<uint64>(const stringRange &range){return strToUnsigned<uint64>(range);}
template<>inline float fromStr<float>(const stringRange &range){return strToReal<float>(range);}
template<>inline double fromStr<double>(const stringRange &range){return strToReal<double>(range);}
template<>inline bool fromStr<bool>(const stringRange &range){return strToBool(range);}

// whole string
inline int8 strToInt8(const string &s){return strToSigned<int8>(stringRange(s));}
inline uint8 strToUInt8(const string &s){return strToUnsigned<int8>(stringRange(s));}
inline int16 strToInt16(const string &s){return strToSigned<int16>(stringRange(s));}
inline uint16 strToUInt16(const string &s){return strToUnsigned<int16>(stringRange(s));}
inline int32 strToInt32(const string &s){return strToSigned<int32>(stringRange(s));}
inline uint32 strToUInt32(const string &s){return strToUnsigned<uint32>(stringRange(s));}
inline int64 strToInt64(const string &s){return strToSigned<int64>(stringRange(s));}
inline uint64 strToUInt64(const string &s){return strToUnsigned<uint64>(stringRange(s));}
inline int strToInt(const string &s){return strToSigned<int>(stringRange(s));}
inline uint strToUInt(const string &s){return strToUnsigned<uint>(stringRange(s));}
inline float strToFloat(const string &s){return strToReal<float>(stringRange(s));}
inline double strToDouble(const string &s){return strToReal<double>(stringRange(s));}
inline bool strToBool(const string &s){return strToBool(stringRange(s));}

// templated (no int and uint! using int32 and uint32)
template<typename T>inline T fromStr(const string &s){gassert(false,"conversion from string: unknown type, conversion not specialized"); return T();};
template<>inline int8 fromStr<int8>(const string &s){return strToInt8(s);}
template<>inline uint8 fromStr<uint8>(const string &s){return strToUInt8(s);}
template<>inline int16 fromStr<int16>(const string &s){return strToInt16(s);}
template<>inline uint16 fromStr<uint16>(const string &s){return strToUInt16(s);}
template<>inline int32 fromStr<int32>(const string &s){return strToInt32(s);}
template<>inline uint32 fromStr<uint32>(const string &s){return strToUInt32(s);}
template<>inline int64 fromStr<int64>(const string &s){return strToInt64(s);}
template<>inline uint64 fromStr<uint64>(const string &s){return strToUInt64(s);}
template<>inline float fromStr<float>(const string &s){return strToFloat(s);}
template<>inline double fromStr<double>(const string &s){return strToDouble(s);}
template<>inline bool fromStr<bool>(const string &s){return strToBool(s);}

// conversions: from T to string
// conversion functions
template <typename T> stringRange signedToStr(const T &i,string &os);
template <typename T> stringRange unsignedToStr(const T &u,string &os);
template <typename T> stringRange realToStr(const T &r,string &os,int precision);
inline stringRange floatToStr(const float &f,string &os,int precision=7){return realToStr<float>(f,os,precision);}
inline stringRange doubleToStr(const double &f,string &os,int precision=13){return realToStr<double>(f,os,precision);}
stringRange boolToStr(const bool &b,string &os);
inline string floatToStr(const float &f,int precision=7){string os(0,' '); os.resize(7+precision); return realToStr<float>(f,os,precision).str();}
inline string doubleToStr(const double &f,int precision=13){string os(0,' '); os.resize(13+precision); return realToStr<double>(f,os,precision).str();}
inline string boolToStr(const bool &b);

// overloaded (there is no point templating this)
inline stringRange toStr(const int8 &i,string &os){return signedToStr<int8>(i,os);}
inline stringRange toStr(const uint8 &i,string &os){return unsignedToStr<uint8>(i,os);}
inline stringRange toStr(const int16 &i,string &os){return signedToStr<int16>(i,os);}
inline stringRange toStr(const uint16 &i,string &os){return unsignedToStr<uint16>(i,os);}
inline stringRange toStr(const int32 &i,string &os){return signedToStr<int32>(i,os);}
inline stringRange toStr(const uint32 &i,string &os){return unsignedToStr<uint32>(i,os);}
inline stringRange toStr(const int64 &i,string &os){return signedToStr<int64>(i,os);}
inline stringRange toStr(const uint64 &i,string &os){return unsignedToStr<uint64>(i,os);}
inline stringRange toStr(const float &i,string &os){return realToStr<float>(i,os,7);}
inline stringRange toStr(const double &i,string &os){return realToStr<double>(i,os,13);}
inline stringRange toStr(const bool &i,string &os){return boolToStr(i,os);}
stringRange toStr(const string &s,string &os);
stringRange toStr(const char *s,string &os);

// string building (here just prototypes - for clarity)
template<typename... Args> string strs(const Args&... args);
template<typename... Args> string strf(const char *format,const Args&... args);


// implementations:
template <typename T>T strToSigned(const stringRange &range){
	string::const_iterator it(range.begin);
	T sign,ret=0;
	if((*range.begin)=='-'){
		it++;
		sign=-1;
	}
	else sign=1;
	for(;it!=range.end;++it){
		gassert(isDigit(*it),"parsing string to integer - non numeric character");
		ret=ret*10+getAlphaToDigit(*it);
	}
	return ret*sign;
}

template <typename T>T strToUnsigned(const stringRange &range){
	T ret=0;
	for(string::const_iterator it(range.begin);it!=range.end;++it){
		gassert(isDigit(*it),"parsing string to integer - non numeric character");
		ret=ret*10+getAlphaToDigit(*it);
	}
	return ret;
}

template <typename T>T strToReal(const stringRange &range){
	string::const_iterator it(range.begin);
	T sign;
	if((*range.begin)=='-'){
		++it;
		sign=-static_cast<T>(1.0);
	}
	else sign=static_cast<T>(1.0);
	T ret=static_cast<T>(0.0);
	for(;it!=range.end;it++){
		if(*it=='.'){
			++it;
			break;
		}
		ret=ret*static_cast<T>(10.0)+static_cast<T>(getAlphaToDigit(*it));
		gassert(isDigit(*it), "parsing string to real - non numeric character");
	}
	T mant=static_cast<T>(0.0),fact=static_cast<T>(1.0);
	for(;it!=range.end;it++){
		fact*=static_cast<T>(0.1);
		mant+=static_cast<T>(getAlphaToDigit(*it))*fact;
		gassert(isDigit(*it), "parsing string to real - non numeric character");
	}
	return (mant+ret)*sign;
}

bool isInteger(const stringRange &range) {
	string::const_iterator it(range.begin);
	if((*range.begin) == '-')
		it++;
	for(; it != range.end; ++it) {
		if(!isDigit(*it))
			return false;
	}
	return true;
}

bool isFloat(const stringRange &range) {
	string::const_iterator it(range.begin);
	if((*range.begin) == '-')
		++it;
	bool dotFound = false;
	for(; it != range.end; it++) {
		if(*it=='.' && !dotFound){
			++it;
			dotFound = true;
		}
		if(!isDigit(*it))
			return false;
	}
	return true;
}

template <typename T> stringRange signedToStr(const T &i,string &os){
	string::iterator p=os.begin()+os.capacity()-1;
	if(i==0){
		*p='0';
		gassert(p-1>=os.begin(),"string to signed - index out of buffer");
		return stringRange(p-1,os.end());
	}
	T v=i<0?-i:i;
	while(v){
		gassert(p+1>=os.begin(),"string to unsigned - index out of buffer");
		*p=getDigitToAlpha(v%10);
		v/=10;
		p--;
	}
	if(i<0){
		*p='-';
		p--;
	}
	gassert(p+1>=os.begin(),"string to unsigned - index out of buffer");
	return stringRange(p+1,os.end());
}

template <typename T> stringRange unsignedToStr(const T &u,string &os){
	string::iterator p=os.begin()+os.capacity()-1;
	if(u==0){
		*p='0';
		gassert(p-1>=os.begin(),"string to unsigned - index out of buffer");
		return stringRange(p-1,os.end());
	}
	T v=u;
	while(v){
		gassert(p+1>=os.begin(),"string to unsigned - index out of buffer");
		*p=getDigitToAlpha(v%10);
		v/=10;
		p--;
	}
	gassert(p+1>=os.begin(),"string to unsigned - index out of buffer");
	return stringRange(p+1,os.end());
}

template <typename T> stringRange realToStr(const T &r,string &os,int precision){
	string::iterator p=os.begin();
	
	// base ###.
	uint32 i=uint32(r);
	i=i<0?-i:i;
	T base=T(i);
	T sbase=r<static_cast<T>(0.0)?-r:r;
	if(i==0){
		*p='0';
		p++;
	}
	while(i){
		*p=getDigitToAlpha(i%10);
		i/=10;
		p++;
	}

	// dot
	*p='.';
	p++;

	// mantissa .###
	T man=sbase-base;
	while(precision--){
		gassert(p<os.end(),"real to unsigned - index out of buffer");
		char dig=char(man*=static_cast<T>(10.0));
		*p=getDigitToAlpha(dig);
		man-=T(dig);
		p++;
	}

	// sign
	if(r<static_cast<T>(0.0)){
		*p='-';
		p++;
	}
	gassert(p<os.end(),"real to unsigned - index out of buffer");
	return stringRange(os.begin(),p-1);
}

inline string boolToStr(const bool &b){
	return string(b?"true":"false");
}

// detail:
namespace detail{
inline void strf(string &buffer,string &out,const char *format){
	while(*format){
		if(*format=='%'){
			gassert(*(format+1)=='%',"formatted string: missing function arguments");
			if(*(format+1)=='%')
				++format;
		}
		out.append(1,*format);
		++format;
	}
}
template <typename T,typename... Args> void strf(string &buffer,string &out,const char *format,const T &v,const Args&... args){
	while(*format){
		if(*format=='%'){
			if(*(format+1)=='%') // replace %% -> %
				++format;
			else{
				stringRange r=toStr(v,buffer);
				out.append(r.begin,r.end);
				strf(buffer,out,format+1,args...);
				return;
			}
		}
		out.append(1,*format);
		++format;
	}
	gassert(false,"formatted string: extra arguments passed to function");
}
template <typename T> void strs(string &buffer,string &out,const T &v){
	stringRange r=toStr(v,buffer);
	out.append(r.begin,r.end);
}
template <typename T,typename... Args> void strs(string &buffer,string &out,const T &val,const Args&... args){
	strs(buffer,out,val);
	strs(buffer,out,args...);
}
}

template<typename... Args> string strf(const char *format,const Args&... args){
	size_t s=strlen(format);
	string ret,buffer;
	buffer.resize(30);
	ret.reserve(s+sizeof...(Args)*5); // 5 chars per argument
	detail::strf(buffer,ret,format,args...);
	return ret;
}

template<typename... Args> string strs(const Args&... args){
	string buff,ret;
	buff.resize(30);
	detail::strs(buff,ret,args...);
	return ret;
}

}}
