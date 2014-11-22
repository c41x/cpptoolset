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
size_t countElements(cells_t::iterator c) {
	if(c->type == cell::typeList) {
		cells_t::iterator i = c + 1;
		cells_t::iterator delim = c + c->i;
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
	return std::prev(std::find_if(variables.rbegin(), variables.rend(),
									  [name](const var_t &e) {
										  return std::get<0>(e) == name;
									  }).base());
}

// returns variable
cells_t::iterator getVariable(const string &name) {
	// find variable
	auto r = findVariable(name);

	// if variable found -> return iterator
	if(r != variables.end())
		return stack.begin() + std::get<1>(*r);

	// variable name not found
	std::cout << "variable \"" << name << "\" not found" << std::endl;
	for(auto v : variables) {
		std::cout << "> " << std::get<0>(v) << " / " << std::get<1>(v) << std::endl;
	}
	return stack.end();
}

// checks if given iterator / address is valid
bool isVariableValid(cells_t::iterator i) {
	return i != stack.end();
}

bool isVariableValid(vars_t::iterator i) {
	return i != variables.end();
}

// push variable and allocate memory
cells_t::iterator pushVariable(const string &name, size_t count) {
	// add new variable
	variables.push_back(std::make_tuple(name, stack.size()));

	// resize stack and return iterator to first allocated element
	stack.resize(stack.size() + count);
	return stack.begin() + std::get<1>(variables.back());
}

// assign address to memory
void pushVariable(const string &name, cells_t::iterator addr) {
	std::cout << "push variable (" << name << ") addr: " << std::distance(stack.begin(), addr) << std::endl;
	variables.push_back(std::make_tuple(name, std::distance(stack.begin(), addr)));
}

// reallocate variable
cells_t::iterator resizeVariable(const string &name, size_t insertPos, size_t elements) {
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

//- 4) consts and intrinsics -


//- 5) eval -
size_t addrd(cells_t::iterator i) {
	return std::distance(stack.begin(), i);
}


/*
void init() {
	auto nil = pushVariable("nil", 1);
	*nil = cell(cell::typeIdentifier, "nil");
}
*/

std::stack<size_t> callStack;
void tab() {
	for(size_t i = 0; i < callStack.size(); ++i) {
		std::cout << "  ";
	}
}
cells_t::iterator eval(cells_t::iterator d) {
	static bool aaa = false;
	if(!aaa)
		stack.reserve(100000);
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
		if(d->i == 0) {
			// return empty
			std::cout << "empty list" << std::endl;
		}

		// first argument must be identifier
		cells_t::iterator fxName = d + 1;
		if(fxName->type != cell::typeIdentifier) {
			// function name must be id
			std::cout << "function name must be ID" << std::endl;
		}

		// is fx name built in function
		if(fxName->s == "defvar") {
			// 3 elements min!
			cells_t::iterator varName = d + 2;
			cells_t::iterator varValue = eval(d + 3);
			pushVariable(varName->s, varValue);
			return varValue;
		}
		else if(fxName->s == "quote") {
			size_t elemsCount = countElements(d + 2);
			size_t whence = stack.size();
			stack.resize(stack.size() + elemsCount);
			std::copy(d + 2, d + 2 + elemsCount, stack.begin() + whence);
			return stack.begin() + whence;
		}
		else if(fxName->s == "lambda") {
			size_t lambdaBodyCount = d->i - 1; // not counting name
			size_t elemsCount = countElements(d) - 2; // not counting list and name
			size_t whence = stack.size();
			stack.resize(stack.size() + elemsCount + 1);
			std::copy(d + 2, d + 2 + elemsCount, stack.begin() + whence + 1);
			stack[whence] = cell(cell::typeList, lambdaBodyCount);
			return stack.begin() + whence;
		}
		else if(fxName->s == "+") {
			tab(); std::cout << "+" << std::endl;
			// sum
			size_t count = d->i - 1;
			int res = 0;
			for(size_t i = 0, j = 0; j < count; ++j) {
				callStack.push(stack.size());
				res += eval(d + 2 + i)->i;
				stack.resize(callStack.top());
				callStack.pop();

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

		// get fx address
		cells_t::iterator fx = getVariable(fxName->s);
		if(isVariableValid(fx)) {
			if(fx->type == cell::typeList) {
				int count = fx->i;
				cells_t::iterator args = fx + 1;
				cells_t::iterator args_vals = d + 2;

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
				cells_t::iterator body = fx + 1 + countElements(fx + 1);
				cells_t::iterator ret;
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
	}

	// tmp
	return stack.end();
}

/*

// consts
const cell true_cell(cell::typeIdentifier, "#t");
const cell false_cell(cell::typeIdentifier, "#f");
const cell nil(cell::typeIdentifier, "nil");

//- fx
cell fx_add(const std::vector<cell> &c) {
	int v = 0;
	for(auto &e : c)
		v += fromStr<int>(e.val);
	return cell(cell::typeInt, toStr(v));
}

cell fx_print(const std::vector<cell> &c) {
	for(const auto &e : c) {
		std::cout << e.val;
	}
	std::cout << std::endl;
	return c.back();
}

//- interpreter core
std::stack<scope> call_stack;

void initInterpreter() {
	stack.resize(1024);
	variables.resize(1024);
	stack_top = -1;

	call_stack.push(scope());
	scope *g = &call_stack.top();

	g->push("nil", nil);
	g->push("#f", false_cell);
	g->push("#t", true_cell);
	g->push("add", cell(cell::typeFunction, &fx_add));
	g->push("+", cell(cell::typeFunction, &fx_add));
	g->push("print", cell(cell::typeFunction, &fx_print));
	g->push("pi", cell(cell::typeInt, "3.14"));
}

cell eval(cell c) {
	scope *s = &call_stack.top();
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
			bool if_p = eval(c.cdr[1]).val != "nil";
			bool else_p = c.cdr.size() > 3;
			return eval(if_p ? c.cdr[2] : (else_p ? c.cdr[3] : nil));
		}
		else if(c.cdr[0].val == "defvar") {
			return *s->push(c.cdr[1].val, eval(c.cdr[2]));
		}
		else if(c.cdr[0].val == "lambda") {
			c.type = cell::typeLambda;
			return c;
		}
		else if(c.cdr[0].val == "defun") {
			c.type = cell::typeLambda;
			string name = c.cdr[1].val;
			c.cdr.erase(c.cdr.begin() + 1);
			s->push(name, c);
			return c;
		}
		else {
			// intrinsic / lambda call
			cell proc(eval(c.cdr[0]));
			std::vector<cell> body;
			for(auto e = c.cdr.begin() + 1; e != c.cdr.end(); ++e) {
				body.push_back(eval(*e));
			}

			// call
			if(proc.type == cell::typeLambda) {
				call_stack.push(scope());
				for(size_t i = 0; i < proc.cdr[1].cdr.size(); ++i) // check if size is the same
					call_stack.top().push(proc.cdr[1].cdr[i].val, body[i]);
				cell ret;
				for(size_t i = 2; i < proc.cdr.size(); ++i)
					ret = eval(proc.cdr[i]);
				call_stack.pop();
				return ret;
			}
			else if(proc.type == cell::typeFunction) {
				return proc.fx(body);
			}
		}
	}

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

	//TODO: minimize output (function names?)
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
*/

}}
