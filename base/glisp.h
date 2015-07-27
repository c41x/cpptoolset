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
#include "includes.h"
#include "math.h"
#include "stream.h"
#include "fs.h"

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

	// constants
	const static cell nil;
	const static cell t;
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

	static cells_t parse(const string &s);
	string eval(cells_t &code);
	string eval(cells_t &&code);
	string eval(const string &s);
	const std::vector<string> &getError();
	void addProcedure(const string &name, procedure_t fx);
	void addVariable(const string &name, cell value);
};

#include "glisp.inc.h"

}}
//~
