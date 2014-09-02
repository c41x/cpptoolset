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

// heap container
template <typename T> class heap {
	typedef typename std::vector<T>::iterator data_iterator;
	typedef struct { data_iterator begin, end; } pointer;
	typedef typename std::vector<pointer>::iterator pointer_iterator;
	std::vector<T> data;
	std::vector<pointer> pointers;
	data_iterator top;

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
		top = data.begin();
	}
	~heap() {

	}
	size_t push(const T &i) {
		// need to free some space
		if(top + 1 >= data.end()) {
			data_iterator fs = find_space(1);
			if(fs + 1 >= data.end()) {
				gc();
				if(top + 1 >= data.end()) {
					std::cout << "out of memory!" << std::endl;
					return -1;
					// out of memory
				}
			}
			else {
				*fs = i;
				pointers.push_back({fs, fs + 1});
				return pointers.size() - 1;
			}
		}

		*top = i;
		pointers.push_back({top, top + 1});
		++top;
		return pointers.size() - 1;
	}
	void pop(size_t ptr) {
		const pointer &p = pointers[ptr];

		// if this is pointer in top of the stack -> adjust top pointer
		if(p.end == top)
			top = p.begin;

		// erase only pointer, leave garbage in heap
		pointers.erase(pointers.begin() + ptr);
	}
	void gc() {
		// sort pointers (only comparing begin, they are not overlapping)
		std::sort(std::begin(pointers), std::end(pointers),
				  [this](const pointer &l, const pointer &r) {
					  return l.begin < r.begin;
				  });

		// rearrange data and shrink vector
		for(pointer_iterator p = pointers.begin() + 1; p != pointers.end(); ++p) {
			pointer_iterator pp = p - 1;
			std::move(p->begin, p->end, pp->end);
		}

		// adjust top pointer
		top = pointers.back().end;
	}
	void print() {
		// sort pointers (only comparing begin, they are not overlapping)
		std::sort(std::begin(pointers), std::end(pointers),
				  [this](const pointer &l, const pointer &r) {
					  return l.begin < r.begin;
				  });

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

typedef std::vector<cell> cells;

// consts
const cell true_cell(cell::typeIdentifier, "#t");
const cell false_cell(cell::typeIdentifier, "#f");
const cell nil(cell::typeIdentifier, "nil");

//- scope (variable container/stack)
class scope {
public:
	typedef std::tuple<string, cell> var_t;
	std::vector<var_t> variables;
	scope *outer;

	scope(scope *outerScope = nullptr) : outer(outerScope) { }
	cell *get(const string &name) {
		auto r = std::find_if(std::begin(variables),
							  std::end(variables),
							  [&name](const var_t &c){ return std::get<0>(c) == name; });
		if(r != variables.end())
			return &std::get<1>(*r);
		else if(outer)
			return outer->get(name);
		else return nullptr; // variable name not found
	}
	cell *add(const string &name, const cell &c) {
		variables.push_back(std::make_tuple(name, c));
		return &std::get<1>(variables.back());
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
	g->add("nil", nil);
	g->add("#f", false_cell);
	g->add("#t", true_cell);
	g->add("add", cell(cell::typeFunction, &fx_add));
	g->add("+", cell(cell::typeFunction, &fx_add));
	g->add("pi", cell(cell::typeInt, "3.14"));
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
	std::stack<cells*> cp;

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
