/*
 * granite engine 1.0 | 2006-2015 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
 * file: glisp
 * created: 29-08-2014
 *
 * description: LISP interpreter
 *
 * changelog:
 * - 29-08-2014: file created
 */

#pragma once
#include "includes.hpp"
#include "math.hpp"
#include "stream.hpp"
#include "fs.hpp"

// when defined - enables error checking via assert
//#define GLISP_DEBUG_ERROR

// when defined - enables error checking via array (getErrors)
#define GLISP_DEBUG_ERROR_ARRAY

// when defined - enables error checking via std output
//#define GLISP_DEBUG_ERROR_STDOUT

// when defined - prints debug info on std output
//#define GLISP_DEBUG_STATE


namespace granite { namespace base {

// just data (variant)
class cell {
    template <typename T>
    struct anyOf_ {
        T t1, t2;
        anyOf_(T _t1, T _t2) : t1(_t1), t2(_t2) {}
    };
public:
    enum type_t {
        typeIdentifier,
        typeInt,
        typeInt64,
        typeFloat,
        typeVector,
        typeString,
        typeList,
        typeDetach
    };

    type_t type;
    string s;
    union {
        struct {
            int32 i;
            int32 j;
        };
        float f;
        int64 ii;
        float v4[4];
    };

    cell() {}
    cell(type_t t, const string &v) : type(t), s(v) {}
    cell(type_t t, int v) : type(t), i(v) {}
    cell(type_t t, int _i, int _j, const string &_s) : type(t), s(_s), i(_i), j(_j) {}
    cell(float _f) : type(typeFloat), f(_f) {}
    cell(int _i) : type(typeInt), i(_i) {}
    cell(int64 _i64) : type(typeInt64), ii(_i64) {}
    cell(const vec &_vec) : type(typeVector) { vec4f _v = _vec; v4[0] = _v.data[0]; v4[1] = _v.data[1]; v4[2] = _v.data[2]; v4[3] = _v.data[3]; }
    cell(const vec4f &_v) : type(typeVector) { v4[0] = _v.data[0]; v4[1] = _v.data[1]; v4[2] = _v.data[2]; v4[3] = _v.data[3]; }
    cell(const string &_s) : type(typeString), s(_s) {}

    const string getStr() const;
    bool isNil() const;
    int32 listSize() const;

    // builders
    static cell list(int32 size);

    // constants
    const static cell nil;
    const static cell t;
    const static cell quote;

    // types for validator
    struct listRange {
        int32 min, max;
        listRange(int32 emin, int32 emax = -1) : min(emin), max(emax) {}
    };

    typedef anyOf_<type_t> anyOf;
    typedef anyOf_<cell> any;
};

// lisp state
struct lispState;

// some basic typedefs
typedef std::vector<cell> cells_t;
typedef cells_t::iterator cell_t;
typedef std::function<cell_t(cell_t, cells_t&)> procedure_t; // input, output

// interpreter
class lisp {
    std::unique_ptr<lispState> _s;

public:
    lisp();
    ~lisp();

    void init(size_t memSize = 100000);
    void close();

    // parsing and evaluation
    static cells_t parse(const string &s);
    string eval(cells_t &code);
    string eval(cells_t &&code);
    string eval(const string &s);

    // error report and debugging
    const std::vector<string> &getError();
    static inline bool validate(cell_t);
    template <typename... Args> static bool validate(cell_t c, cell::type_t tt, Args... t);
    template <typename... Args> static bool validate(cell_t c, const cell &tt, Args... t);
    template <typename... Args> static bool validate(cell_t c, const cell::listRange &tt, Args... t);
    template <typename... Args> static bool validate(cell_t c, const cell::anyOf &tt, Args... t);
    template <typename... Args> static bool validate(cell_t c, const cell::any &tt, Args... t);
    static inline string validateStr(cell::type_t tt);
    static inline string validateStr(const cell &tt);
    static inline string validateStr(const cell::listRange &tt);
    static inline string validateStr(const cell::anyOf tt);
    static inline string validateStr(const cell::any tt);
    template <typename... Args, typename T> static string validateStr(T tt, Args... t);
    void signalError(const string &description);

    // utils
    static void mapc(cell_t c, std::function<void(cell_t)> fx);
    static int32 size(cell_t c);

    // extending API
    void addProcedure(const string &name, procedure_t fx);
    void addVariable(const string &name, cell value);

    // access to in-memory constants
    cell_t nil() const;
    cell_t t() const;

};

#include "glisp.inc.hpp"

}}
//~
