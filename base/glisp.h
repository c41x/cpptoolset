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
#include "tokenizer.h"
#include "gstdlib.h"

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
			int i;
			int j;
		};
		float f;
		int64 ii;
		__m128 xmm;
	};

	cell() {}
	cell(type_t t, const string &v) : type(t), s(v) {}
	cell(type_t t, int v) : type(t), i(v) {}
	cell(type_t t, int _i, int _j, const string &_s) : type(t), s(_s), i(_i), j(_j) {}
	cell(float _f) : type(typeFloat), f(_f) {}
	cell(int _i) : type(typeInt), i(_i) {}
	cell(int64 _i64) : type(typeInt64), ii(_i64) {}
	cell(vec _xmm) : type(typeVector), xmm(_xmm) {}
	cell(const string &_s) : type(typeString), s(_s) {}

	const string getStr() const;

	// constants
	static cell nil;
	static cell t;
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
	string eval(const string &s);
	void addProcedure(const string &name, procedure_t fx);
	void addVariable(const string &name, cell value);
};

}}

// TODO: numeric operations
// TODO: error handling
// TODO: change __m128 to vec4f, maybe add some math 2d stuff
/*
 * add-to-list (unique)
 * add-to-ordered-list (sorted)
 * reverse
 * sort
 * delete
 * assoc-delete
 */
