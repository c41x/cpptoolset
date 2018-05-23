#include "string.hpp"
#include "math.hpp"

namespace granite { namespace base {

int8 floatPrecision = 6;

void lowerCase(string &s) {
    for (auto &it : s)
        setLower(it);
}

void upperCase(string &s) {
    for (auto &it : s)
        setUpper(it);
}

string lowerCase(const string &s) {
    string r = s;
    lowerCase(r);
    return r;
}

string upperCase(const string &s) {
    string r = s;
    upperCase(r);
    return r;
}

void trimWhitespaces(string &s) {
    auto lmbIsNotWhitespace = [](const char &c) -> bool { return !isWhiteSpace(c); };
    s.erase(s.begin(), std::find_if (s.begin(), s.end(), lmbIsNotWhitespace)); // leading
    s.erase(std::find_if (s.rbegin(), s.rend(), lmbIsNotWhitespace).base(),s.end()); // trailing
}

void findAndDelete(string &s, const string &what) {
    size_t pos = 0;
    while ((pos = s.find(what,pos)) != s.npos)
        s.erase(pos, what.length());
}

void findAndReplace(string &s, const string &what, const string &replacement) {
    size_t pos = 0;
    while ((pos = s.find(what, pos)) != s.npos) {
        s.replace(pos, what.length(), replacement);
        pos += replacement.length();
    }
}

void findAndReplace(string &s, char what, char replacement) {
    size_t pos = 0;
    while ((pos = s.find(what, pos)) != s.npos) {
        s.replace(pos, 1, 1, replacement);
        ++pos;
    }
}

void findAndCutAfter(string &s, const string &what) {
    size_t pos = s.find(what);
    if (pos != s.npos)
        s.erase(pos + what.length());
}

void findAndCutBefore(string &s, const string &what) {
    size_t pos = s.find(what);
    if (pos != s.npos)
        s.erase(0, pos);
}

void deleteWhitespaces(string &s) {
    s.erase(std::remove_if (s.begin(), s.end(), isWhiteSpace), s.end());
}

bool containsSubstr(const string &s, const string &search) {
    return string::npos != s.find(search);
}

std::vector<stringRange> divideString(const string &s, char divChar) {
    std::vector<stringRange> result;
    size_t pos = 0, bpos = 0;
    while ((pos = s.find(divChar, bpos)) != s.npos) {
        result.push_back(stringRange(s.begin() + bpos, s.begin() + pos));
        bpos = pos + 1;
    }
    result.push_back(stringRange(s.begin() + bpos, s.begin() + s.length()));
    return result;
}

float matchString(const string &a, const string &b){ // check string similarity in 0-1 scale (GP Gems?)
    char const *pa = a.c_str();
    char const *pb = b.c_str();
    uint32 lena = (uint32)a.size();
    uint32 lenb = (uint32)b.size();
    uint32 len = std::max(lena, lenb);
    float ret = 0.f;
    while ((pa < (a.c_str() + lena)) && (pb < (b.c_str() + lenb))) {
        if (*pa == *pb) {
            ret += 1.f / len;
            ++pa;
            ++pb;
        }
        else if (getLower(*pa) == getLower(*pb)) {
            ret += .9f / len;
            ++pa;
            ++pb;
        }
        else{
            char const *pba = a.c_str() + lena;
            char const *pbb = b.c_str() + lenb;
            int tcount = 0, bbcount = 2147483640, acount = 0, bcount = 0;
            for (char const *ppa = pa; (ppa < (a.c_str() + lena)) && ((acount + bcount) < bbcount); ++ppa) {
                for (char const *ppb = pb; (ppb < (b.c_str() + lenb)) && ((acount + bcount) < bbcount); ++ppb) {
                    if (getLower(*ppa) == getLower(*ppb)) {
                        tcount += bcount + acount;
                        if (tcount < bbcount) {
                            bbcount = tcount;
                            pba = ppa;
                            pbb = ppb;
                        }
                    }
                    ++bcount;
                }
                ++acount;
                bcount = 0;
            }
            pa = pba;
            pb = pbb;
        }
    }
    return clip(0.f, ret, 1.f);
}

string extractFileName(const string &s) {
    size_t pos = s.find_last_of(GE_DIR_SEPARATOR);
    if (pos != s.npos)
        return s.substr(pos + 1);
    return string("");
}

string extractFilePath(const string &s) {
    size_t pos = s.find_last_of(GE_DIR_SEPARATOR);
    if (pos != s.npos)
        return s.substr(0, pos);
    return string("");
}

string extractExt(const string &s) {
    size_t pos = s.find_last_of('.');
    if (pos != s.npos)
        return s.substr(pos + 1);
    return string("");
}

string changeExt(const string &s, const string &ext) {
    size_t pos = s.find_last_of('.');
    string ret = s;
    if (pos != s.npos)
        ret.replace(pos, ret.npos, ext);
    return ret;
}

string cutLongPath(const string &s) {
    string ret("");
    size_t first_sep = s.find_first_of(GE_DIR_SEPARATOR);
    if (first_sep != s.npos)
        ret += s.substr(0, first_sep + 1);
    else return string("");

    ret += string("..");

    size_t last_sep = s.find_last_of(GE_DIR_SEPARATOR);
    if (last_sep != s.npos)
        ret += s.substr(last_sep);

    return ret;
}

stringRange initToken(stringRange s) {
    return stringRange(s.begin, s.begin);
}

stringRange nextToken(stringRange s, stringRange &tok) {
    for (tok.begin = tok.end; tok.end != s.end; ++tok.end) {
        if (isWhiteSpace(*tok.end)) {
            if (tok.end == tok.begin)
                ++tok.begin;
            else return tok;
        }
    }
    return tok;
}

bool endToken(stringRange s, stringRange tok) {
    while(tok.end != s.end && isWhiteSpace(*tok.end))
        ++tok.end;
    return tok.end == s.end;
}

}}
