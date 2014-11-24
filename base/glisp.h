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
#include "gstdlib.h"

namespace granite { namespace base {

//- 1) just data (variant) -
class cell {
public:
	enum type_t {
		typeIdentifier = 0,
		typeInt = 1,
		typeList = 1 << 1
	};

	type_t type;
	string s;
	int i;

	cell() {}
	cell(type_t t, const string &v) : type(t), s(v) {}
	cell(type_t t, int v) : type(t), i(v){}

	operator string() const {
		const char *type_ts[] = {
			"id",
			"int",
			"list"
		};
		return strf("type: % | i = % | s = %", type_ts[type], i, s);
	}
};
typedef std::vector<cell> cells_t;
typedef std::vector<cell>::iterator cell_t;

//- 2) parser -
cells_t parse(const string &s) {
	// create tokenizer
	enum tokenType {
		tokenSymbol = -1,
		tokenWhiteSpace,
		tokenOpenPar,
		tokenClosePar,
		tokenComment,
		tokenString
	};

	tokenizer tok;
	tok.addRule(tokenWhiteSpace, true, " \t\v\n\r");
	tok.addRule(tokenOpenPar, false, "(");
	tok.addRule(tokenClosePar, false, ")");
	tok.addRule(tokenComment, true, ";", "", "\n\r");
	tok.addRule(tokenString, false, "\"", "\\", "\"");

	// actual parsing
	std::stack<std::tuple<size_t, int>> openPars; // index / count
	cells_t cells;

	for(auto t = tok.begin(s, false); t; t = tok.next()) {
		if(t.id == tokenOpenPar) {
			// increase list elements count (list is element too)
			if(openPars.size() > 0)
				std::get<1>(openPars.top())++;

			// push new list to stack
			openPars.push(std::make_tuple(cells.size(), 0));

			// add list to cells (count == 0 for now)
			cells.push_back(cell(cell::typeList, 0));
		}
		else if(t.id == tokenClosePar) {
			// actualize list elements count and pop stack
			cells[std::get<0>(openPars.top())].i = std::get<1>(openPars.top());
			openPars.pop();
		}
		else if(t.id == tokenSymbol) {
			// increase list elements count
			if(openPars.size() > 0)
				std::get<1>(openPars.top())++;

			// determine token type and add atom to cells
			if(isInteger(t.value))
				cells.push_back(cell(cell::typeInt, fromStr<int>(t.value)));
			else cells.push_back(cell(cell::typeIdentifier, t.value));
		}
	}

	return cells;
}

// counts elements of given cell
size_t countElements(cell_t c) {
	if(c->type == cell::typeList) {
		cell_t i = c + 1;
		cell_t delim = c + c->i;
		while(i <= delim) {
			if(i->type == cell::typeList)
				delim += i->i;
			++i;
		}
		return std::distance(c, i);
	}
	return 1;
}

string toString(const cells_t &cells) {
	string r;
	for(const cell &c : cells) {
		if(c.type == cell::typeList) {
			r += "[list:" + toStr(c.i) + "]";
		}
		else if(c.type == cell::typeInt) {
			r += toStr(c.i);
		}
		else if(c.type == cell::typeIdentifier) {
			r += c.s;
		}
		else {
			r += "[unknown]";
		}
		r += " ";
	}
	return r;
}

//- 3) dynamic scoping / stack / variable memory -
typedef std::tuple<string, size_t> var_t;
typedef std::vector<var_t> vars_t; // name, stack position
cells_t stack;
vars_t variables;

string printStack() {
	return toStr(stack.size()) + " > " + toString(stack);
}

vars_t::iterator findVariable(const string &name) {
	// searching backwards on stack will find variable in outer scopes
	auto r = std::find_if(variables.rbegin(), variables.rend(),
						  [name](const var_t &e) {
							  return std::get<0>(e) == name;
						  });

	// return proper invalid pointer
	if(r == variables.rend())
		return variables.end();
	return std::prev(r.base());
}

// checks if given iterator / address is valid
bool isVariableValid(cell_t i) {
	return i != stack.end();
}

bool isVariableValid(vars_t::iterator i) {
	return i != variables.end();
}

// returns variable
cell_t getVariable(const string &name) {
	// find variable
	auto r = findVariable(name);

	// if variable found -> return iterator
	if(isVariableValid(r))
		return stack.begin() + std::get<1>(*r);

	// variable name not found
	std::cout << "variable \"" << name << "\" not found" << std::endl;
	for(auto v : variables) {
		std::cout << "> " << std::get<0>(v) << " / " << std::get<1>(v) << std::endl;
	}
	return stack.end();
}

// push variable and allocate memory
cell_t pushVariable(const string &name, size_t count) {
	// add new variable
	variables.push_back(std::make_tuple(name, stack.size()));

	// resize stack and return iterator to first allocated element
	stack.resize(stack.size() + count);
	return stack.begin() + std::get<1>(variables.back());
}

// assign address to memory
void pushVariable(const string &name, cell_t addr) {
	std::cout << "push variable (" << name << ") addr: " << std::distance(stack.begin(), addr) << std::endl;
	variables.push_back(std::make_tuple(name, std::distance(stack.begin(), addr)));
}

// push data on top of stack (copy all addr cell) return data address on stack
cell_t pushData(cell_t addr) {
	size_t elemsCount = countElements(addr);
	cell_t whence = stack.end();
	stack.resize(stack.size() + elemsCount);
	std::copy(addr, addr + elemsCount, whence);
	return whence;
}

// push one element to stack
cell_t pushCell(cell c) {
	stack.push_back(c);
	return stack.begin() + stack.size();
}

// push cdr of given list (create list without first element)
cell_t pushCdr(cell_t l) {
	size_t elemsCount = countElements(l);
	size_t elemsCountFirst = countElements(l + 1);
	size_t realElemsCount = elemsCount - elemsCountFirst;
	cell_t whence = stack.end();
	stack.push_back(cell(cell::typeList, l->i - 1)); // logical elements without first element
	stack.resize(stack.size() + realElemsCount - 1);
	std::copy(l + 1 + elemsCountFirst, l + elemsCount, whence + 1);
	return whence;
}

// push car of given list (basically just copy cell to stack)
cell_t pushCar(cell_t l) {
	stack.push_back(*l);
	return stack.begin() + stack.size();
}

// reallocate variable
cell_t resizeVariable(const string &name, size_t insertPos, size_t elements) {
	auto var = findVariable(name);
	if(isVariableValid(var)) {
		// move variable positions by offset
		mapc(var + 1, variables.end(),
			 [elements](var_t &v) {
				 return std::make_tuple(std::get<1>(v) += elements);
			 });

		// resize stack and return variable address
		auto varMemLocation = stack.begin() + std::get<1>(*var);
		stack.insert(varMemLocation + insertPos, elements, cell());
		return varMemLocation;
	}

	// return invalid variable address
	return stack.end();
}

// get address (just size_t number - do not use in logic code)
size_t getAddress(cell_t c) {
	return std::distance(stack.begin(), c);
}

// call stack
std::stack<size_t> callStack;

void pushCallStack() {
	callStack.push(stack.size());
}

void popCallStack() {
	stack.resize(callStack.top());
	callStack.pop();
}

//- 4) initialization consts and intrinsics -

// shortcuts to constants
cell_t c_nil;
cell_t c_t;

typedef std::function<cell_t(cell_t)> intrinsic_fx_t;
typedef std::tuple<string, intrinsic_fx_t> intrinsic_tuple_t;
typedef std::vector<intrinsic_tuple_t> intrinsics_t;
typedef intrinsics_t::iterator intrinsic_t;
intrinsics_t intrinsics;

cell_t c_message(cell_t c) {
	std::cout << "*intrinsic* ";
	std::cout << (c + 1)->i << std::endl;
	return c;
}

cell_t c_mul(cell_t c) {
	std::cout << "*intrinsic* ";
	int r = 1;
	for(cell_t i = c + 1; i != c + 1 + c->i; ++i) {
		r *= i->i;
	}
	return pushCell(cell(cell::typeInt, r));
}

// search for intrinsic address
intrinsic_t getIntrinsic(const string &name) {
	std::cout << "get intrinsic: " << name << std::endl;
	return std::find_if(std::begin(intrinsics), std::end(intrinsics),
						[name](intrinsic_tuple_t &e) {
							return name == std::get<0>(e);
						});
}

// test intrinsic iterator for validity
bool isIntrinsicValid(intrinsic_t i) {
	return i != intrinsics.end();
}

// adds intrinsic to list
bool addIntrinsic(const string &name, intrinsic_fx_t fx) {
	if(!isIntrinsicValid(getIntrinsic(name))) {
		intrinsics.push_back(std::make_tuple(name, fx));
		return true;
	}
	else ; // intrinsic already defined
	return false;
}

// initialize interpreter
void init(size_t stackSize) {
	// resize stack
	stack.reserve(stackSize);

	// constant variables / keywords
	auto nil = pushVariable("nil", 1);
	*nil = cell(cell::typeIdentifier, "nil");
	c_nil = nil;
	auto t = pushVariable("t", 1);
	*t = cell(cell::typeIdentifier, "t");
	c_t = t;

	// intrinsics
	addIntrinsic("*", &c_mul);
	addIntrinsic("message", &c_message);
}

//- 5) eval -

void tab() {
	for(size_t i = 0; i < callStack.size(); ++i) {
		std::cout << "  ";
	}
}
cell_t eval(cell_t d) {
	tab();
	std::cout << "eval " << string(*d)
			  << "  > stack " << printStack() << std::endl;

	if(d->type == cell::typeInt) {
		stack.push_back(*d);
		return --stack.end();
	}
	else if(d->type == cell::typeIdentifier) {
		auto addr = getVariable(d->s);
		if(isVariableValid(addr))
			return addr;
		// variable not found
	}
	else if(d->type == cell::typeList) {
		// empty list evaluates to nil
		if(d->i == 0)
			return c_nil;

		// first argument must be identifier
		// TODO: or lambda/list
		cell_t fxName = d + 1;
		if(fxName->type != cell::typeIdentifier) {
			std::cout << "function name must be ID" << std::endl;
		}

		// is fx name built in function
		if(fxName->s == "defvar") {
			// 3 elements min!
			cell_t varName = d + 2;
			cell_t varValue = eval(d + 3);
			pushVariable(varName->s, varValue);
			return varValue;
		}
		else if(fxName->s == "quote") {
			// just copy quote body to stack
			return pushData(d + 2);
		}
		else if(fxName->s == "lambda") {
			// copy cdr of lambda, first element is "lambda" identifier, we dont need it
			return pushCdr(d);
		}
		else if(fxName->s == "+") {
			tab(); std::cout << "+" << std::endl;
			// sum
			size_t count = d->i - 1;
			int res = 0;
			for(size_t i = 0, j = 0; j < count; ++j) {
				pushCallStack();
				res += eval(d + 2 + i)->i;
				popCallStack();

				if((d + 2 + i)->type == cell::typeList)
					i += countElements(d + 2 + i);
				else ++i;
			}

			// return
			size_t retPos = stack.size();
			stack.resize(stack.size() + 1);
			cell &ret = stack[retPos];
			ret.type = cell::typeInt;
			ret.i = res;

			return stack.begin() + retPos;
		}
		// TODO: undef variable / delete variable / test variable
		// TODO: intrinsic, simple array ops? what kind of functionality needed?
		else if(fxName->s == "if") {
			// test
			pushCallStack();
			bool test = eval(d + 2) != c_nil;
			popCallStack();

			// test and eval
			if (test)
				return eval(d + 2 + countElements(d + 2));
			else return eval(d + 3 + countElements(d + 2));
		}
		else if(fxName->s == "=") {
			cell_t a1 = d + 2;
			cell_t a2 = d + 3;
			if(a1->type == cell::typeInt && a2->type == cell::typeInt) {
				if(a1->i == a2->i)
					return c_t;
				else return c_nil;
			}
			return c_nil;
		}

		// get fx address
		cell_t fx = getVariable(fxName->s);
		if(isVariableValid(fx)) {
			if(fx->type == cell::typeList) {
				int count = fx->i;
				cell_t args = fx + 1;
				cell_t args_vals = d + 2;

				// bind args
				// args->i == (d + 2)->i
				for(int i = 0; i < args->i; ++i) {
					auto v = eval(args_vals + i);
					pushVariable((args + i + 1)->s, v);
					std::cout << (args + i + 1)->s << " = "
							  << (args_vals + i)->i << std::endl;
				}

				// evaluate body
				int bodyCount = count - 1;
				cell_t body = fx + 1 + countElements(fx + 1);
				cell_t ret;
				for(int i = 0, j = 0; j < bodyCount; ++j) {
					tab();std::cout << " > body eval: " << j << std::endl;
					tab();std::cout << " > body is: " << std::distance(stack.begin(), body + i)
									<< std::endl;
					if(j != count - 1)
						;//callStack.push(stack.size());

					ret = eval(body + i);
					tab();std::cout << " >" << string(*ret) << std::endl;

					if(j != count - 1) {
						//stack.resize(callStack.top());
						//callStack.pop();
					}

					if((body + i)->type == cell::typeList)
						i += countElements(body + i);
					else ++i;
				}
				return ret;
			}
			std::cout << "not function!" << std::endl;
			return fx;
		}
		else {
			intrinsic_t i = getIntrinsic(fxName->s);
			if(isIntrinsicValid(i)) {
				std::cout << "valid intrinsic: " << std::get<0>(*i) << std::endl;
				cell_t r = stack.end();
				pushCell(cell(cell::typeList, d->i - 1)); // save count
				for(cell_t a = d + 2; a != d + 2 + d->i - 1; ++a) // COMPLEX call!
					eval(a);
				std::cout << "calling intrinsic" << std::endl;
				return std::get<1>(*i)(r);
			}
			std::cout << "intrinsic not found" << std::endl;
		}
	}

	// tmp, should report some error (syntax, logic error?)
	return stack.end();
}

}}
