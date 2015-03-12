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

//- just data (variant) -
class cell {
public:
	enum type_t {
		typeIdentifier,
		typeInt,
		typeInt64,
		typeFloat,
		typeVector,
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

	const string getStr() const;
};

typedef std::vector<cell> cells_t;
typedef std::vector<cell>::iterator cell_t;
typedef std::function<cell_t(cell_t)> intrinsic_fx_t;
typedef std::tuple<string, size_t> var_key_t; // name, stack position
typedef std::vector<var_key_t> vars_t;
typedef std::map<var_key_t, cells_t> lists_t;
typedef vars_t::iterator var_t;
typedef std::stack<size_t> call_stack_t;
typedef std::tuple<string, intrinsic_fx_t> intrinsic_tuple_t;
typedef std::vector<intrinsic_tuple_t> intrinsics_t;
typedef intrinsics_t::iterator intrinsic_t;

//- client side -
class lisp {
	cells_t _stack;
	vars_t _variables;
	call_stack_t _callStack;
	intrinsics_t _intrinsics;
	// TODO: add lists
public:
	lisp() {}
	~lisp() {}

	void init(size_t memSize = 100000);
	void close();
	string eval(const string &s);
	void addIntrinsic(const string &name, intrinsic_fx_t fx);
	void addVariable(const string &name, cell value);
};

}}

// TODO: lists management
// TODO: resizable memory (add-to-list etc.)
// TODO: loops? mapping functions
// TODO: cons? assoc
// TODO: cond
