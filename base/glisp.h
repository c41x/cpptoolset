/*
 * granite engine 1.0 | 2006-2014 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
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

namespace granite { namespace base {
/*
// heap container
template <typename T> class heap {
	typedef typename std::vector<T>::iterator data_iterator;
	typedef struct { data_iterator begin, end; } pointer;
	typedef typename std::vector<pointer>::iterator pointer_iterator;
	std::vector<T> data;
	std::vector<pointer> pointers;

	size_t add_pointer_in_order(const pointer &p) {
		// insert new pointer at upper bound
		auto insert_pos = pointers.insert(
			std::upper_bound(std::begin(pointers), std::end(pointers), p,
							 [](const pointer &t, const pointer &val) {
								 return t.begin < val.begin;
							 }), p);

		// return position
		return std::distance(pointers.begin(), insert_pos);
	}

	data_iterator find_space(size_t count) {
		for(pointer_iterator p = pointers.begin(); p != pointers.end() - 1; ++p) {
			pointer_iterator pn = p + 1;
			if(std::distance(p->end, pn->begin) >= count)
				return p->end;
		}
		return data.end();
	}

public:
	heap(size_t heapSize) {
		data.resize(heapSize);
	}

	~heap() {

	}

	size_t push(const T &i) {
		// check if we need to free some space
		if(pointers.size() > 0 && pointers.back().end + 1 >= data.end()) {
			std::cout << "finding space for: " << i << std::endl;
			data_iterator fs = find_space(1);
			if(fs + 1 >= data.end()) {
				std::cout << "space not found, performing gargage collect\n";
				gc();
				if(pointers.back().end + 1 >= data.end()) {
					std::cout << "out of memory!" << std::endl;
					return 0;
					// out of memory
				}
				std::cout << "gc ok\n";
			}
			else {
				std::cout << "space found!\n";
				*fs = i;
				return add_pointer_in_order({fs, fs + 1});
			}
		}

		if(pointers.size() == 0) {
			pointers.push_back({data.begin(), data.begin() + 1});
			data[0] = i;
			return 0;
		}
		else {
			size_t pos = add_pointer_in_order({pointers.back().end, pointers.back().end + 1});
			*(pointers.back().begin) = i;
			return pos;
		}
	}

	size_t push(std::vector<T> array) {
		// check if we need to free some space
		const size_t count = array.size();
		if(pointers.size() > 0 && pointers.back().end + count >= data.end()) {
			std::cout << "finding space for: array" << std::endl;
			data_iterator fs = find_space(count);
			if(fs + count >= data.end()) {
				std::cout << "space not found, performing gargage collect\n";
				gc();
				if(pointers.back().end + count >= data.end()) {
					std::cout << "out of memory!" << std::endl;
					return 0;
					// out of memory
				}
				std::cout << "gc ok\n";
			}
			else {
				std::cout << "space found!\n";
				std::move(std::begin(array), std::end(array), fs);
				return add_pointer_in_order({fs, fs + count});
			}
		}

		if(pointers.size() == 0) {
			pointers.push_back({data.begin(), data.begin() + count});
			std::move(std::begin(array), std::end(array), std::begin(data));
			return 0;
		}
		else {
			size_t pos = add_pointer_in_order({pointers.back().end, pointers.back().end + count});
			std::move(std::begin(array), std::end(array), pointers.back().begin);
			return pos;
		}
	}

	void pop(size_t ptr) {
		// erase only pointer, leave garbage in heap
		pointers.erase(pointers.begin() + ptr);
	}

	void gc() {
		// rearrange data and shrink vector
		for(pointer_iterator p = pointers.begin() + 1; p != pointers.end(); ++p) {
			pointer_iterator pp = p - 1;
			std::move(p->begin, p->end, pp->end);
		}
	}

	void print() {
		for(pointer_iterator p = pointers.begin(); p != pointers.end(); ++p) {
			pointer_iterator np = p + 1;
			if(np != pointers.end() && std::distance(p->end, np->begin) > 0) {
				std::cout << " ." << std::distance(p->end, np->begin) << ". ";
			}

			std::cout << " < ";
			std::for_each(p->begin, p->end,
						  [](const T &pp) {
							  std::cout << pp << " ";
						  });
			std::cout << ">";
		}
	}
};
*/

// just data (variant)
class cell {
public:
	//- definitions
	// what is inside
	enum type_t {
		typeIdentifier = 0,
		typeString = 1,
		typeInt = 1 << 1,
		typeFloat = 1 << 2,
		typeFunction = 1 << 3,
		typeList = 1 << 4
	};

	// cons (pointers to data)
	typedef struct {
		size_t car;
		size_t cdr;
	} cons_t;

	typedef cell (*fx_t)(const std::vector<cell>&);

	//- data (only string)
	cons_t cons;
	type_t type;
	std::vector<cell> cdr;
	string val;
	fx_t fx;

	//cell(cell &&ms) {

	//}

	cell(type_t t, const string &v) {
		type = t;
		val = v;
	}
	cell(type_t t, fx_t proc) {
		type = t;
		fx = proc;
	}
	cell() {}
	cell(type_t t) {
		type = t;
	}
};

std::vector<cell> stack;
std::vector<string> variables;
size_t stack_top;

// consts
const cell true_cell(cell::typeIdentifier, "#t");
const cell false_cell(cell::typeIdentifier, "#f");
const cell nil(cell::typeIdentifier, "nil");

//- scope (variable container/stack)
class scope {
public:
	size_t stack_bottom;

	scope() {
		// define stack frame
		stack_bottom = stack_top;
	}

	~scope() {
		// revert stack pointer ("free" memory)
		stack_top = stack_bottom;
	}

	cell *get(const string &name) {
		// searching backwards on stack will find variable in outer scopes
		auto r = std::find(variables.rend() - stack_top - 1, variables.rend(), name);
		if(r != variables.rend())
			return &stack[std::distance(r, variables.rend() - 1)];
		else return nullptr; // variable name not found
	}

	cell *push(const string &name, const cell &c) {
		++stack_top;
		// resizing stack
		variables[stack_top] = name;
		stack[stack_top] = c;
		return &stack[stack_top];
	}
};

//- fx
cell fx_add(const std::vector<cell> &c) {
	int v = 0;
	for(auto &e : c)
		v += fromStr<int>(e.val);
	return cell(cell::typeInt, toStr(v));
}

//- interpreter core
void initInterpreter(scope *g) {
	stack.resize(1024);
	variables.resize(1024);
	stack_top = -1;
	g->push("nil", nil);
	g->push("#f", false_cell);
	g->push("#t", true_cell);
	g->push("add", cell(cell::typeFunction, &fx_add));
	g->push("+", cell(cell::typeFunction, &fx_add));
	g->push("pi", cell(cell::typeInt, "3.14"));
}

cell eval(cell c, scope *s) {
	if(c.type == cell::typeIdentifier) {
		cell *v = s->get(c.val);
		if(!v)  {
			std::cout << strf("- undefined variable %.", c.val) << std::endl;
			return nil;
		}
		return *v;
	}
	else if(c.type == cell::typeInt) {
		return c;
	}
	else if(c.cdr.empty()) {
		return nil;
	}
	else if(c.cdr[0].type == cell::typeIdentifier) {
		if(c.cdr[0].val == "quote")
			return c.cdr[1];
		else if(c.cdr[0].val == "if") {
			bool if_p = eval(c.cdr[1], s).val != "nil";
			bool else_p = c.cdr.size() > 3;
			return eval(if_p ? c.cdr[2] : (else_p ? c.cdr[3] : nil), s);
		}
		else if(c.cdr[0].val == "defvar") {
			return *s->push(c.cdr[1].val, eval(c.cdr[2], s));
		}
		else if(c.cdr[0].val == "defun") {
			return *s->push(c.cdr[1].val, cell(cell::typeFunction, c.cdr[2]));
		}
	}

	cell proc(eval(c.cdr[0], s));
	std::vector<cell> body;
	for(auto e = c.cdr.begin() + 1; e != c.cdr.end(); ++e) {
		body.push_back(eval(*e, s));
	}
	return proc.fx(body);

	std::cout << "- unknown cell type!" << std::endl;
	return nil;
}

cell parse(const string &s) {
	enum tokenType {
		tokenSymbol = -1,
		tokenWhiteSpace,
		tokenOpenPar,
		tokenClosePar,
		tokenComment,
		tokenString,
		tokenOperator,
		tokenSymbolicOperator
	};

	tokenizer tok;
	tok.addRule(tokenWhiteSpace, true, " \t\v\n\r");
	tok.addRule(tokenOpenPar, false, "(");
	tok.addRule(tokenClosePar, false, ")");
	tok.addRule(tokenComment, true, ";", "", "\n\r");
	tok.addRule(tokenString, false, "\"", "\\", "\"");

	cell r;
	std::stack<std::vector<cell>*> cp;

	for(auto t = tok.begin(s, false); t; t = tok.next()) {
		if(t.id == tokenOpenPar) {
			if(cp.empty()) {
				r.type = cell::typeList;
				cp.push(&r.cdr);
			}
			else {
				cp.top()->push_back(cell(cell::typeList));
				cp.push(&cp.top()->back().cdr);
			}
		}
		else if(t.id == tokenClosePar) {
			cp.pop();
		}
		else if(t.id == tokenSymbol) {
			cell cc;

			// detect type
			if(isInteger(t.value))
				cc.type = cell::typeInt;
			else cc.type = cell::typeIdentifier;
			cc.val = t.value;

			// insert/return atom
			if(cp.empty()) {
				return cc;
			}
			else {
				cp.top()->push_back(cc);
			}
		}
	}

	return r;
}

string toString(const cell &c) {
	string r;
	if(c.type == cell::typeList) {
		bool first = true;
		for(auto a : c.cdr) {
			r += (first ? "(" : ", ") + toString(a);
			first = false;
		}
		return r + ")";
	}
	else if(c.type == cell::typeInt) {
		return c.val;
	}
	else if(c.type == cell::typeFunction) {
		return "<func>";
	}
	return c.val;
}

}}
