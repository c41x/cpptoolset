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

#include "glisp.h"

namespace granite { namespace base {

//- cell -
const string cell::getStr() const {
	if (type == typeInt) return strs(i);
	else if (type == typeIdentifier || type == typeString) return s;
	else if (type == typeInt64) return strs(ii);
	else if (type == typeFloat) return strs(f);
	else if (type == typeVector) {
		vec4f v = vec(xmm);
		return strs(v.x, v.y, v.z, v.w);
	}
	return "";
}

const bool operator==(const cell &l, const cell &r) {
	if (l.type == r.type) {
		if (l.type == cell::typeInt) return l.i == r.i;
		else if (l.type == cell::typeString || l.type == cell::typeIdentifier) return l.s == r.s;
		else if (l.type == cell::typeFloat) return l.f == r.f;
		else if (l.type == cell::typeInt64) return l.ii == r.ii;
	}
	return false;
}

const bool operator<(const cell &l, const cell &r) {
	if (l.type == r.type) {
		if (l.type == cell::typeInt) return l.i < r.i;
		else if (l.type == cell::typeString || l.type == cell::typeIdentifier) return l.s < r.s;
		else if (l.type == cell::typeFloat) return l.f < r.f;
		else if (l.type == cell::typeInt64) return l.ii < r.ii;
	}
	return false;
}

const bool operator!=(const cell &l, const cell &r) { return !(l == r); }
const bool operator>=(const cell &l, const cell &r) { return !(l < r); }
const bool operator>(const cell &l, const cell &r) { return r < l; }
const bool operator<=(const cell &l, const cell &r) { return !(l > r); }

cell cell::nil = cell(cell::typeIdentifier, "nil");
cell cell::t = cell(cell::typeIdentifier, "t");

//- state -
typedef std::tuple<string, size_t> var_key_t; // name, stack position
typedef std::vector<var_key_t> vars_t;
typedef std::map<var_key_t, cells_t> lists_t;
typedef vars_t::iterator var_t;
typedef std::stack<size_t> call_stack_t;
typedef std::tuple<string, procedure_t> procedure_tuple_t;
typedef std::vector<procedure_tuple_t> procedures_t;
typedef procedures_t::iterator proc_t;

struct lispState {
	// stack / variable memory
	cells_t stack;
	vars_t variables;

	// aux memory
	lists_t lists;

	// shortcuts to constants
	cell_t c_nil;
	cell_t c_t;

	// stack containing frames begin
	call_stack_t callStack;

	// procedures list (user fx)
	procedures_t procedures;
};

namespace detail {

//#define GLISP_DEBUG_LOG
#ifdef GLISP_DEBUG_LOG
#define dout(param) std::cout << param
#else
#define dout(param)
#endif

//- parser -
cells_t parse(const string &s) {
	// create tokenizer
	enum tokenType {
		tokenSymbol = -1,
		tokenWhiteSpace,
		tokenOpenPar,
		tokenClosePar,
		tokenComment,
		tokenString,
		tokenQuote
	};

	// define rules
	tokenizer tok;
	tok.addRule(tokenWhiteSpace, true, " \t\v\n\r");
	tok.addRule(tokenOpenPar, false, "(");
	tok.addRule(tokenClosePar, false, ")");
	tok.addRule(tokenComment, true, ";", "", "\n\r");
	tok.addRule(tokenString, false, "\"", "\\", "\"");
	tok.addRule(tokenQuote, false, "\'");

	// actual parsing
	std::stack<std::tuple<size_t, bool>> openPars; // index, quote
	cells_t cells;

	// helpers
	auto checkQuoteDelim = [&openPars, &cells]() {
		// it's quote - check delimiter and terminate (quote always has 2 elements)
		// and we need to terminate it then
		if (openPars.size() > 0
			&& std::get<1>(openPars.top())
			&& cells[std::get<0>(openPars.top())].i > 1)
			openPars.pop();
	};

	auto onNewElement = [&openPars, &cells, &checkQuoteDelim]() {
		// increase list elements count
		if (openPars.size() > 0)
			cells[std::get<0>(openPars.top())].i++;
	};

	// tokenizer loop
	for (auto t = tok.begin(s, false); t; t = tok.next()) {
		if (t.id == tokenOpenPar || t.id == tokenQuote) {
			// check if we need close quote first
			checkQuoteDelim();

			// add new element (list is element too)
			onNewElement();

			// push new list to stack
			openPars.push(std::make_tuple(cells.size(), t.id == tokenQuote));

			// add list to cells (initialize count value)
			cells.push_back(cell(cell::typeList, t.id == tokenQuote ? 1 : 0));

			// add 'quote' id if needed
			if (t.id == tokenQuote)
				cells.push_back({cell::typeIdentifier, "quote"});
		}
		else if (t.id == tokenClosePar) {
			// just pop pars stack and check ' term
			openPars.pop();
			checkQuoteDelim();
		}
		else if (t.id == tokenSymbol) {
			// adds new element
			onNewElement();

			// determine token type and add atom to cells
			if (isInteger(t.value))
				cells.push_back(cell(cell::typeInt, fromStr<int>(t.value)));
			else cells.push_back(cell(cell::typeIdentifier, t.value));

			// after element is added - check if we need to close quote
			checkQuoteDelim();
		}
	}

	return cells;
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

// iterating over lists: first element
cell_t firstCell(cell_t c) {
	return c + 1;
}

// iterating over lists: ++
cell_t nextCell(cell_t c) {
	cell_t delim = c;
	while (c <= delim) {
		if(c->type == cell::typeList || c->type == cell::typeDetach) // TODO: leave detach test?
			delim += c->i;
		++c;
	}
	return c;
}

// iterating over list: nth element of list
cell_t nthCell(cell_t c, size_t n) {
	for (c = firstCell(c); n--; c = nextCell(c));
	return c;
}

// iterating over lists: cell after last one
cell_t endCell(cell_t c) {
	return nextCell(c);
}

// last cell of list
cell_t lastCell(cell_t c) {
	cell_t e = endCell(c);
	cell_t r = c;
	c = firstCell(c);
	while (true) {
		if (c >= e)
			return r;
		else r = c;
		c = nextCell(c);
	}
	return c;
}

// counts elements of given cell
size_t countElements(cell_t c) {
	return std::distance(c, endCell(c));
}

// nice print cell
string toString(const cell_t c) {
	if (c->type == cell::typeList) {
		string s = "(";
		for (auto i = firstCell(c); i != endCell(c); i = nextCell(i)) {
			s += toString(i);
			if (nextCell(i) != endCell(c))
				s += " ";
		}
		s += ")";
		return s;
	}
	return c->getStr();
}

//- dynamic scoping / stack / variable memory -
// comparsion operator for variables (compares by address)
bool operator<(const var_key_t &a, const var_key_t &b) {
	return std::get<1>(a) < std::get<1>(b);
}

// detach variable to aux memory
cell_t detachVariable(lispState &s, cell_t addr, cell_t addr_end, cell_t val_begin, cell_t val_end, var_key_t key) {
	// move data
	auto e = s.lists.find(key);
	if (e == s.lists.end()) {
		cells_t &m = s.lists[key];

		// reserve space
		size_t selfSize = std::distance(addr, addr_end);
		size_t valSize = std::distance(val_begin, val_end);
		m.reserve(selfSize + valSize);

		// copy contents
		m.insert(m.end(), addr, addr_end);
		m.insert(m.end(), val_begin, val_end);

		// leave id on stack
		*addr = {cell::typeDetach,
				 addr->type == cell::typeList ? addr->i : 1,
				 (int)std::get<1>(key), std::get<0>(key)};

		// return new address
		return m.begin();
	}

	// return existing variable
	return e->second.begin();
}

// shortcuts for detach variable
cell_t detachVariable(lispState &s, cell_t addr, var_key_t key) {
	return detachVariable(s, addr, addr + countElements(addr), addr, addr, key);
}

cell_t detachVariable(lispState &s, cell_t addr, cell_t val, var_key_t key) {
	return detachVariable(s, addr, addr,
						  val, val + countElements(val), key);
}

// checks if given iterator is valid
bool isVariableValid(lispState &s, var_t i) {
	return i != s.variables.end();
}

// search for variable in index by name
var_t findVariable(lispState &s, const string &name) {
	auto ret = find_if_backwards(s.variables.begin(), s.variables.end(),
								 [name](const var_key_t &e) {
									 return std::get<0>(e) == name;
								 });

	// just prints info that variable is not found
	if (!isVariableValid(s, ret)) {
		// variable name not found
		dout("variable \"" << name << "\" not found" << std::endl);
		for(auto v : s.variables) {
			dout(" > " << std::get<0>(v) << " = " << std::get<1>(v) << std::endl);
		}
	}

	return ret;
}

// get variable address on stack
cell_t getVariableStackAddress(lispState &s, var_t v) {
	return s.stack.begin() + std::get<1>(*v);
}

string &getVariableName(var_t v) {
	return std::get<0>(*v);
}

size_t &getVariablePosition(var_t v) {
	return std::get<1>(*v);
}

// gets address to variable data
cell_t getVariableAddress(lispState &s, var_t v) {
	cell_t addr = getVariableStackAddress(s, v);

	if (addr->type == cell::typeDetach)
		// TODO: check?
		return s.lists[*v].begin();
	return addr;
}

// gets detached memory vector
cells_t &getVariableContainer(lispState &s, var_t v) {
	// TODO: check?
	return s.lists[*v];
}

// gather all variable data <is valid, is detached, data address, variable it, container>
std::tuple<bool, bool, cell_t, var_t, cells_t*> fetchVariable(lispState &s, cell_t addr) {
	const string &name = addr->s;
	var_t var = findVariable(s, name);
	bool isValid = isVariableValid(s, var);
	if (isValid) {
		bool isDetached = getVariableStackAddress(s, var)->type == cell::typeDetach;
		if (isDetached)
			return std::make_tuple(true, true,
								   getVariableContainer(s, var).begin(),
								   var,
								   &getVariableContainer(s, var));
		return std::make_tuple(true, false, getVariableStackAddress(s, var), var, &s.stack);
	}
	return std::make_tuple(false, false, s.stack.end(), var, &s.stack);
}

// push variable and allocate memory
cell_t pushVariable(lispState &s, const string &name, size_t count) {
	// add new variable
	s.variables.push_back(std::make_tuple(name, s.stack.size()));

	// resize stack and return iterator to first allocated element
	s.stack.resize(s.stack.size() + count);
	return s.stack.begin() + std::get<1>(s.variables.back());
}

// TODO: detect if inserting to variables in order
// assign address to memory
void pushVariable(lispState &s, const string &name, cell_t addr) {
	dout("push variable (" << name << ") addr: " << std::distance(s.stack.begin(), addr)
		 << " value: " << toString(addr) << std::endl);
	s.variables.push_back(std::make_tuple(name, std::distance(s.stack.begin(), addr)));
}

// push data on top of stack (copy all addr cell) return data address on stack
cell_t pushData(lispState &s, cell_t addr) {
	size_t elemsCount = countElements(addr);
	cell_t whence = s.stack.end();
	s.stack.resize(s.stack.size() + elemsCount);
	std::copy(addr, addr + elemsCount, whence);
	return whence;
}

// push one element to stack (carefull with lists!)
cell_t pushCell(lispState &s, cell c) {
	s.stack.push_back(c);
	return s.stack.begin() + s.stack.size() - 1;
}

// push cdr of given list (create list without first element)
cell_t pushCdr(lispState &s, cell_t l) {
	cell_t whence = s.stack.end();
	size_t elemsCount = countElements(l);
	if (elemsCount > 0) {
		size_t elemsCountFirst = countElements(l + 1);
		size_t realElemsCount = elemsCount - elemsCountFirst;
		s.stack.push_back(cell(cell::typeList, l->i - 1)); // logical elements without first element
		s.stack.resize(s.stack.size() + realElemsCount - 1);
		std::copy(l + 1 + elemsCountFirst, l + elemsCount, whence + 1);
		return whence;
	}
	return s.c_nil;
}

// get address (just size_t number - do not use in logic code)
size_t getAddress(lispState &s, cell_t c) {
	return std::distance(s.stack.begin(), c);
}

void tab(lispState &s) {
	for(size_t i = 0; i < s.callStack.size(); ++i) {
		dout("  ");
	}
}

void printVariables(lispState &s) {
	dout("defined variables(" << s.variables.size() << ")" << std::endl);
	for (auto &v : s.variables) {
		dout(std::get<0>(v) << " = " << s.stack[std::get<1>(v)].getStr() << std::endl);
	}
}

void printState(lispState &s) {
	// reverse call stack
	auto cs = s.callStack;
	call_stack_t rcs;
	while (cs.size()) {
		rcs.push(cs.top());
		cs.pop();
	}

	// print all elements
	size_t varsLeft = s.variables.size();
	for (size_t i = 0; i < s.stack.size(); ++i) {
		// put call stack bottom frame
		if (rcs.size() > 0 && i == rcs.top()) {
			dout("| ");
			rcs.pop();
		}

		// variable name
		auto v = std::find_if(s.variables.begin(), s.variables.end(), [&i](var_key_t &v) {
				return std::get<1>(v) == i;
			});
		if (v != s.variables.end()) {
			--varsLeft;
			dout("#" << std::get<0>(*v) << " ");
		}

		// print element
		auto &e = s.stack[i];
		if (e.type == cell::typeList) {
			dout("[list:" << e.i << "] ");
		}
		else if (e.type == cell::typeIdentifier) {
			dout(e.s << " ");
		}
		else if (e.type == cell::typeInt) {
			dout(e.i << " ");
		}
		else if (e.type == cell::typeDetach) {
			dout("[detach:" << e.i << ":" << e.j << ":" <<
				 (e.j != -1 ? toString(getVariableAddress(s, v)) : "?") << "] ");
		}
	}

	// print last call stack frame
	if (rcs.size() > 0 && rcs.top() == s.stack.size()) {
		dout("| ");
		rcs.pop();
	}

	// TODO: print counts
	if (rcs.size() > 0)
		dout("| call stack corrupted! ");

	if (varsLeft > 0)
		dout("| variables corrupted!");

	dout(std::endl);
}

void pushCallStack(lispState &s) {
	s.callStack.push(s.stack.size());
}

void popVariablesAbove(lispState &s, size_t addr) {
	// no variables defined with address above given (warning - asserting that variables is not empty)
	if (addr <= std::get<1>(s.variables.back())) {
		// there are some variables above addr find last (should always delete sth)
		s.variables.erase(backwards_until(s.variables.begin(), s.variables.end(),
										  [&addr](var_key_t var) {
											  return std::get<1>(var) < addr;
										  }), s.variables.end());
	}
}

void popCallStack(lispState &s) {
	// delete variables
	popVariablesAbove(s, s.callStack.top());

	// "free" data
	s.stack.resize(s.callStack.top());
	s.callStack.pop();
}

// pops call stack and leaves given cell at bottom of current stack frame
cell_t popCallStackLeaveData(lispState &s, cell_t addr) {
	// undefine all variables on this stack frame
	popVariablesAbove(s, s.callStack.top());

	// copy data
	size_t elemsCount = countElements(addr);
	cell_t whence = s.stack.begin() + s.callStack.top();
	std::copy(addr, addr + elemsCount, whence); // safe, not overlapping (src > dst)

	// remove unused data and pop call stack frame
	s.stack.resize(s.callStack.top() + elemsCount);
	s.callStack.pop();
	return whence;
}

// pops call stack and leaves stack untouched (unbounds variables!)
cell_t popCallStackLeaveData(lispState &s) {
	popVariablesAbove(s, s.callStack.top());
	cell_t ret = s.stack.begin() + s.callStack.top();
	s.callStack.pop();
	return ret;
}

// TODO: remove duplicate variables
// removes unused (unbound) data from top stack frame
void sweepStack(lispState &s) {
	// stack offsets - everything 'below' is valid
	size_t stackTopOffset = s.callStack.top();
	cell_t stackTop = s.stack.begin() + stackTopOffset;

	// searches for first variable in stack frame
	auto findFirstVar = [&stackTopOffset, &s]() -> var_t {
		return std::next(find_if_backwards(s.variables.begin(), s.variables.end(),
										   [&stackTopOffset](const var_key_t &var) {
											   return std::get<1>(var) < stackTopOffset;
										   }));
	};

	// relocate all variables to beginning of stack
	auto varTop = findFirstVar();
	while (varTop != s.variables.end()) {
		size_t &srcAddrOffset = std::get<1>(*varTop);
		cell_t srcAddr = s.stack.begin() + srcAddrOffset;
		size_t elements = countElements(srcAddr);

		// erase - detach (unbound variable, 0 cells moved)
		if (srcAddr->type == cell::typeDetach && srcAddr->j == -1) {
			s.variables.erase(varTop);
			elements = 0;
		}
		else {
			// change variable address
			srcAddrOffset -= srcAddrOffset - stackTopOffset;

			// it's detached memory - copy only 1 element (ID)
			if (srcAddr->type == cell::typeDetach) {
				elements = 1;
				srcAddr->i = 1;
			}

			// move data to stack top
			std::copy(srcAddr, srcAddr + elements, stackTop);
		}

		// continue searching
		stackTop += elements;
		stackTopOffset += elements;
		varTop = findFirstVar();
	}

	// discard rest
	s.stack.resize(stackTopOffset);
}

// TODO: removes all unused data (moved previously by detach) - rebuild callstack
// void sweepAll();

//- initialization consts and procedures -
// search for procedure address
proc_t getProcedure(lispState &s, const string &name) {
	return std::find_if(std::begin(s.procedures), std::end(s.procedures),
						[name](procedure_tuple_t &e) {
							return name == std::get<0>(e);
						});
}

// test procedure iterator for validity
bool isProcedureValid(lispState &s, proc_t i) {
	return i != s.procedures.end();
}

// adds procedure to list
bool addProcedure(lispState &s, const string &name, procedure_t fx) {
	if(!isProcedureValid(s, getProcedure(s, name))) {
		s.procedures.push_back(std::make_tuple(name, fx));
		return true;
	}
	else ; // procedure already defined
	return false;
}

// initialize interpreter
void init(lispState &s, size_t stackSize) {
	// resize stack
	s.stack.reserve(stackSize);

	// constant variables / keywords
	auto nil = pushVariable(s, "nil", 1);
	*nil = cell(cell::typeIdentifier, "nil");
	s.c_nil = nil;
	auto t = pushVariable(s, "t", 1);
	*t = cell(cell::typeIdentifier, "t");
	s.c_t = t;
}

//- eval -
cell_t eval(lispState &s, cell_t d, bool temporary = false);

// helper for evaluating lists, evals all elements and returns last
cell_t evalreturn(lispState &s, cell_t begin, cell_t end) {
	cell_t lastResult = s.c_nil; // return nil when evaluating empty list

	// evaluate all cells
	for (cell_t i = begin; i != end; i = nextCell(i))
		lastResult = eval(s, i);

	return lastResult;
}

// evals all elements leaving results on stack (same as above but no address returned)
void evalNoStack(lispState &s, cell_t begin, cell_t end) {
	for (; begin != end; begin = nextCell(begin))
		eval(s, begin);
}

// calls [op] for all (evaluated) elements of list
template <typename T_OP>
void evalmap(lispState &s, cell_t begin, cell_t end, T_OP op, bool temporary = false) {
	for (cell_t i = begin; i != end; i = nextCell(i)) {
		cell_t lastResult = eval(s, i, temporary);
		op(lastResult);
	}
}

// evals all list elements until [op] returns false
template <typename T_OP>
bool evalUntilUnary(lispState &s, cell_t begin, cell_t end, T_OP op, bool temporary = false) {
	for (cell_t i = begin; i != end; i = nextCell(i)) {
		cell_t lastResult = eval(s, i, temporary);
		if (!op(lastResult))
			return false;
	}
	return true;
}

// same as above but binary
template <typename T_OP>
bool evalUntilBinary(lispState &s, cell_t begin, cell_t end, T_OP op, bool temporary = false) {
	cell_t prevResult = begin;
	for (cell_t i = begin; i != end; i = nextCell(i)) {
		cell_t lastResult = eval(s, i, temporary);
		if (i != begin && !op(prevResult, lastResult))
			return false;
		prevResult = lastResult;
	}
	return true;
}

cell_t eval(lispState &s, cell_t d, bool temporary) {
	tab(s); dout("eval: " << toString(d) << std::endl);

	if (d->type == cell::typeInt) {
		// when temporary is true - return value directly (it's in input array!)
		if (temporary)
			return d;

		// leave it on stack
		s.stack.push_back(*d);
		return --s.stack.end();
	}
	else if (d->type == cell::typeIdentifier) {
		var_t var = findVariable(s, d->s);
		if (isVariableValid(s, var)) {
			auto addr = getVariableAddress(s, var);

			// return temporary result
			if (temporary)
				return addr;

			// leave list on stack
			if (addr->type == cell::typeList)
				return pushData(s, addr);

			// it's an atom
			s.stack.push_back(*addr);
			return --s.stack.end();
		}
		// variable not found
	}
	else if (d->type == cell::typeList) {
		// empty list evaluates to nil
		if (d->i == 0)
			return s.c_nil;

		// first argument must be identifier
		cell_t fxName = d + 1;
		if ((d + 1)->type == cell::typeList) {
			// first list element is list -> evaluating lambda
			return eval(s, d + 1, temporary);
		}
		else if (fxName->type != cell::typeIdentifier) {
			dout("function name must be ID" << std::endl);
			return s.c_nil;
		}

		//- builtins -
		// builtins rules:
		// 1) a) returns address to result
		//    b) can return temporary result (in source / stack memory)
		//    c) can use temporary memory in read only
		// 2) a) leaves stack frame at the same point as before execution
		//    b) unbounds all unused variables that ran out of scope
		if (fxName->s == "defvar") {
			// 3 elements min!
			cell_t varName = d + 2;
			cell_t varValue = eval(s, d + 3);
			pushVariable(s, varName->s, varValue);

			// return identifier of created variable
			return pushCell(s, *varName);
		}
		else if (fxName->s == "quote") {
			// return back source, caller will only fetch data
			if (temporary)
				return d + 2;

			// just copy quote body to stack
			return pushData(s, d + 2);
		}
		else if (fxName->s == "lambda") {
			// copy cdr of lambda, first element is "lambda" identifier, we dont need it
			return pushCdr(s, d);
		}
		else if (fxName->s == "+") {
			int sum = 0;
			evalmap(s, firstCell(d) + 1, endCell(d), [&sum](cell_t i){ sum += i->i; });

			// leave return value on stack
			return pushCell(s, {cell::typeInt, sum});
		}
		else if (fxName->s == "if") {
			// test, we dont need return value - discard it with call stack
			pushCallStack(s);
			cell_t testi = eval(s, d + 2, true);
			bool test = *testi != *s.c_nil;

			// test and eval
			if (test)
				return popCallStackLeaveData(s, eval(s, nextCell(d + 2)));
			else {
				cell_t offset = nextCell(nextCell(d + 2)); // else statements offset
				return popCallStackLeaveData(s, evalreturn(s, offset, endCell(d)));
			}
		}
		else if (fxName->s == "progn") {
			// d->i must be > 1
			pushCallStack(s);
			return popCallStackLeaveData(s, evalreturn(s, d + 2, endCell(d)));
		}
		else if (fxName->s == "let") {
			// evaluate and push variables
			pushCallStack(s);
			cell_t args = d + 2;
			for (cell_t a = firstCell(args); a != endCell(args); a = nextCell(a)) {
				cell_t val = eval(s, a + 2);
				pushVariable(s, (a + 1)->s, val);
			}

			// evaluate function body
			return popCallStackLeaveData(s, evalreturn(s, nextCell(args), endCell(d)));
		}
		else if (fxName->s == "boundp") {
			// eval(d + 2) must be ID
			return isVariableValid(s, findVariable(s, eval(s, d + 2, true)->s)) ? s.c_t : s.c_nil;
		}
		else if (fxName->s == "unbound") {
			// eval(d + 2) must be ID (we are not using stack frames)
			cell r = *eval(s, d + 2, true);

			// find and tag variable address as detached
			var_t var = findVariable(s, r.s);
			if (isVariableValid(s, var)) {
				cell_t addr = getVariableAddress(s, var);
				*addr = {cell::typeDetach, addr->i, -1, ""};
			}

			// return argument back
			return pushCell(s, r);
		}
		else if (fxName->s == "list") {
			// empty list evaluates to nil
			if (d->i < 2)
				return s.c_nil;

			// create list and eval its elements
			auto ret = s.stack.begin() + s.stack.size();
			s.stack.push_back({cell::typeList, d->i - 1});
			evalNoStack(s, d + 2, endCell(d));
			return ret;
		}
		else if (fxName->s == "listp") {
			// d->i > 1
			pushCallStack(s);
			auto ret = eval(s, d + 2, true)->type != cell::typeList ? s.c_nil : s.c_t;
			popCallStack(s);
			return ret;
		}
		else if (fxName->s == "car") {
			// d->i must be > 1
			auto arg = d + 2;

			// we could return temporary only if arg is not list
			if (temporary && arg->type != cell::typeList) {
				auto l = eval(s, arg, true); // guaranteed not to leave anything on stack
				if (l->type == cell::typeList) {
					if (l->i > 0)
						return l + 1;
					else return s.c_nil;
				}
				return l;
			}

			// leave whole list on stack
			pushCallStack(s);
			auto r = eval(s, arg);
			if (r->type == cell::typeList) {
				if (r->i > 0)
					return popCallStackLeaveData(s, r + 1);

				// we've got list with no elements - discard r and return nil
				popCallStack(s);
				return s.c_nil;
			}
			popCallStack(s);
			return r;
		}
		else if (fxName->s == "cdr") {
			// d->i > 1
			auto arg = d + 2;

			// we cant return temporary result
			pushCallStack(s);
			auto r = eval(s, arg);
			if (r->type == cell::typeList) {
				if (r->i > 1) {
					r->i--; // we're erasing one element - update list's el count
					s.stack.erase(r + 1, r + 1 + countElements(r + 1));
					return popCallStackLeaveData(s);
				}

				// empty list
				popCallStack(s);
				return s.c_nil;
			}
			popCallStack(s);
			return r;
		}
		else if (fxName->s == "nth") {
			// d->i > N
			// calc N
			pushCallStack(s);
			int n = eval(s, d + 2, true)->i;

			// find address
			cell_t nth = eval(s, d + 3);
			if (nth->type != cell::typeList){
				popCallStack(s);
				return s.c_nil; // error?
			}

			// nth->i must be < N
			// find nth element in list and return result
			return popCallStackLeaveData(s, nthCell(nth, n));
		}
		else if (fxName->s == "defun") {
			// d->i > 4
			cell_t fnName = d + 2;

			// just put list (lambda) on stack
			cell_t va = pushCell(s, {cell::typeList, d->i - 2}); // without "defun" and <fnName>
			s.stack.insert(s.stack.end(), d + 3, endCell(d)); // copy args and body
			pushVariable(s, fnName->s, va);

			// return function id
			return pushCell(s, *fnName);
		}
		else if (fxName->s == "setq" || fxName->s == "set") {
			pushCallStack(s);

			// gather data
			const bool isSetq = fxName->s == "setq";
			bool isValid;
			bool isDetached;
			cell_t addr;
			var_t var;
			cells_t *cont;
			std::tie(isValid, isDetached, addr, var, cont) =
				fetchVariable(s, isSetq ? (d + 2) : eval(s, d + 2, true));

			// perform variable replace
			if (isValid) {
				cell_t val = eval(s, nextCell(d + 2));
				size_t targetSize = countElements(val);

				// cell is already detached -> reassign its contents
				if (isDetached) {
					cont->assign(val, val + targetSize);
				}
				else {
					// calculate elements count
					size_t sourceSize = countElements(addr);

					// check if we need more space
					if (sourceSize >= targetSize) {
						// just replace content on stack
						std::copy(val, val + targetSize, addr);
					}
					else {
						// we must detach variable
						addr = detachVariable(s, addr, val, *var);
					}
				}
			}

			// return address and pop all temporary mess
			popCallStack(s);
			return addr;
		}
		else if (fxName->s == "length") {
			pushCallStack(s);
			cell_t addr = eval(s, d + 2, true);
			int len = addr->type == cell::typeList ? addr->i : 1;
			popCallStack(s);
			return pushCell(s, {cell::typeInt, len});
		}
		else if (fxName->s == "push") {
			// addr must be list, d + 3 must be id
			pushCallStack(s);
			bool isValid;
			bool isDetached;
			cell_t addr;
			var_t var;
			cells_t *cont;
			std::tie(isValid, isDetached, addr, var, cont) = fetchVariable(s, nextCell(d + 2));

			// push variable at list's end
			if (isValid) {
				cell_t val = eval(s, d + 2);

				// insert cell
				if (isDetached) {
					cont->insert(cont->end(), val, val + countElements(val));
					addr = cont->begin();
				}
				else addr = detachVariable(s, addr, addr + countElements(addr),
										   val, val + countElements(val), *var);

				// update count
				addr->i += 1;
			}

			// return address and pop all temporary mess
			popCallStack(s);
			return addr;
		}
		else if (fxName->s == "pop") {
			// addr must be list, d + 3 must be id
			pushCallStack(s);
			bool isValid;
			bool isDetached;
			cell_t addr;
			var_t var;
			cells_t *cont;
			std::tie(isValid, isDetached, addr, var, cont) = fetchVariable(s, d + 2);

			// pop variable and return it's value
			if (isValid) {
				// find last element and leave it on the stack
				cell_t last = lastCell(addr);
				cell_t ret = pushData(s, last);

				// note: free space only for detached!
				if (isDetached)
					cont->erase(last, cont->end());

				// update count
				addr->i -= 1;

				// return value
				return popCallStackLeaveData(s, ret);
			}

			// return address and pop all temporary mess
			popCallStack(s);
			return addr;
		}
		else if (fxName->s == "append") {
			// addr must be list, d + 3 must be id
			pushCallStack(s);
			bool isValid;
			bool isDetached;
			cell_t addr;
			var_t var;
			cells_t *cont;
			std::tie(isValid, isDetached, addr, var, cont) = fetchVariable(s, d + 2);

			// merge 2 lists
			if (isValid) {
				// eval target value
				cell_t val = eval(s, nextCell(d + 2), true);
				size_t elems = countElements(val);

				// merging
				if (isDetached) {
					cont->insert(cont->end(), val + 1, val + elems);
					addr = cont->begin(); // addr may be invalid at this point
				}
				else addr = detachVariable(s, addr, addr + elems, val + 1, val + elems, *var);

				// update count (-1 because countElements includes list header)
				addr->i += elems - 1;
			}

			// return address and pop all temporary mess
			popCallStack(s);
			return addr;
		}
		else if (fxName->s == "setcar") {
			pushCallStack(s);
			bool isValid;
			bool isDetached;
			cell_t addr;
			var_t var;
			cells_t *cont = nullptr;
			std::tie(isValid, isDetached, addr, var, cont) = fetchVariable(s, d + 2);

			if (isValid) {
				cell_t val = eval(s, nextCell(d + 2));
				size_t dstSize = countElements(val);
				size_t srcSize = countElements(addr + 1);

				// check if we need to detach memory
				if (!isDetached && srcSize != dstSize) {
					addr = detachVariable(s, addr, *var);
					cont = &getVariableContainer(s, var);
				}

				// replace memory content
				remove_copy(*cont, addr + 1, addr + 1 + srcSize,
							val, val + dstSize);
			}

			return popCallStackLeaveData(s);
		}
		else if (fxName->s == "setcdr") {
			pushCallStack(s);
			bool isValid;
			bool isDetached;
			cell_t addr;
			var_t var;
			cells_t *cont = nullptr;
			std::tie(isValid, isDetached, addr, var, cont) = fetchVariable(s, d + 2);

			if (isValid) {
				cell_t val = eval(s, nextCell(d + 2));
				size_t dstSize = countElements(val);
				size_t srcCarSize = countElements(addr + 1);
				size_t srcSize = countElements(addr);
				size_t valOffset = val->type == cell::typeList ? 1 : 0;

				// update list count
				addr->i = 1 + (val->type == cell::typeList ? val->i : 1);

				// check if we need to detach memory
				if (!isDetached && (srcSize - srcCarSize - 1) != (dstSize - valOffset)) {
					detachVariable(s, addr, addr + 1 + srcCarSize,
								   val + valOffset, val + dstSize, *var);
				}
				else {
					// replace memory content
					remove_copy(*cont, addr + 1 + srcCarSize, addr + srcSize,
								val + valOffset, val + dstSize);
				}
			}

			return popCallStackLeaveData(s);
		}
		else if (fxName->s == "while") {
			cell_t result = s.c_nil;
			while (true) {
				pushCallStack(s);

				// compute test
				cell_t test = eval(s, d + 2, true);

				// if test is not nil -> eval body, otherwise return last result
				if (*test != *s.c_nil)
					result = evalreturn(s, nextCell(d + 2), endCell(d));
				else return popCallStackLeaveData(s, result);
				popCallStack(s);
			}

			// will never reach here
			return s.c_nil;
		}
		else if (fxName->s == "dotimes") {
			pushCallStack(s);

			// extract name and loop iteration count
			string &name = (d + 2)->s;
			int ntimes = eval(s, d + 3, true)->i;

			// create iterator variable
			cell_t it = pushVariable(s, name, 1);
			cell_t result = s.c_nil;

			// update variable value and eval body
			for (int i = 0; i < ntimes; ++i) {
				*it = { cell::typeInt, i };
				result = evalreturn(s, d + 4, endCell(d));
			}

			// return last eval result or nil
			return popCallStackLeaveData(s, result);
		}
		else if (fxName->s == "dolist") {
			pushCallStack(s);

			// extract iterator name, and eval list
			string &name = (d + 2)->s;
			cell_t lst = eval(s, d + 3);

			// setup variable and get [var] reference
			cell_t it = firstCell(lst);
			cell_t end = endCell(lst);
			cell_t endBody = endCell(d);
			cell_t result = s.c_nil;
			pushVariable(s, name, it);
			var_key_t &var = s.variables.back();

			// iterate through list
			for(; it != end; it = nextCell(it)) {
				// change variable address
				// note: we could do that only in [lst] list range
				// otherwise it will break variables index
				std::get<1>(var) = std::distance(s.stack.begin(), it);

				// eval body
				result = evalreturn(s, nextCell(d + 3), endBody);
			}

			// return last eval or nil
			return popCallStackLeaveData(s, result);
		}
		else if (fxName->s == "cond") {
			// iterate through all clauses
			for (cell_t c = d + 2; c != endCell(d); c = nextCell(c)) {
				// check condition - if true return evaluated result
				if (*eval(s, c + 1, true) != *s.c_nil) {
					pushCallStack(s);
					return popCallStackLeaveData(
						s, evalreturn(s, nextCell(c + 1), endCell(c)));
				}
			}

			// no condition yielded true -> return nil
			return s.c_nil;
		}
		else if (fxName->s == "=") {
			pushCallStack(s);

			// checks if all evaluated cells are equal (supports multiple values)
			cell_t result = evalUntilBinary(s, d + 2, endCell(d),
											[&s](cell_t n, cell_t n1) -> bool {
												return *n == *n1;
											}, true) ? s.c_t : s.c_nil;
			popCallStack(s);
			return result;
		}
		else if (fxName->s == "!=") {
			pushCallStack(s);

			// checks if all evaluated cells are not equal
			// does not support multiple values
			cell_t result = *eval(s, d + 2, true) != *eval(s, nextCell(d + 2), true) ?
				s.c_t : s.c_nil;
			popCallStack(s);
			return result;
		}

		//- functions evaluation -
		// get fx address
		var_t var = findVariable(s, fxName->s);
		if (isVariableValid(s, var)) {
			cell_t fx = getVariableAddress(s, var);
			if (fx->type == cell::typeList) {
				// [list:][list:]<arg><arg>[list:]<body><body>[list:]<body>...
				// function stack frame
				pushCallStack(s);

				// evaluate and bind args
				cell_t args = fx + 1;
				cell_t args_vals = d + 2; // skip list and fx name
				cell_t args_vals_i = args_vals;
				for (int i = 0; i < args->i; ++i) {
					auto v = eval(s, args_vals_i);
					args_vals_i = nextCell(args_vals_i);
					pushVariable(s, (args + i + 1)->s, v);
				}

				// evaluate body
				cell_t body = nextCell(args);
				return popCallStackLeaveData(s, evalreturn(s, body, endCell(fx)));
			}
			dout("not function!" << std::endl);
			return fx;
		}
		else {
			proc_t i = getProcedure(s, fxName->s);
			if (isProcedureValid(s, i)) {
				pushCallStack(s);

				// arguments list address
				cell_t r = s.stack.end();

				// evaluate arguments (leave result on stack)
				pushCell(s, cell(cell::typeList, d->i - 1)); // list elements count (not counting name)
				evalNoStack(s, d + 2, endCell(d));

				// call procedure
				return popCallStackLeaveData(s, std::get<1>(*i)(r, s.stack));
			}
			dout("procedure not found" << std::endl); // not a function?
		}
	}

	// TODO: tmp, should report some error (syntax, logic error?)
	return s.stack.end();
}

}


//- client side -
lisp::lisp() : _s(new lispState()) {}
lisp::~lisp() {}

void lisp::init(size_t memSize) {
	detail::init(*_s, memSize);
	detail::pushCallStack(*_s);
}

void lisp::close() {
	detail::popCallStack(*_s);
}

string lisp::eval(const string &s) {
	dout(std::endl << std::endl);
	auto code = detail::parse(s);
	dout(detail::toString(code) << std::endl);
	auto retAddr = detail::eval(*_s, code.begin(), true);
	string r = detail::toString(retAddr);
	dout("return addr: " << detail::getAddress(*_s, retAddr)
		 << " | " << detail::toString(retAddr) << std::endl);
	detail::printState(*_s);
	dout("sweep..." << std::endl);
	detail::sweepStack(*_s);
	detail::printState(*_s);
	return r;
}

void lisp::addProcedure(const string &name, procedure_t fx) {
	detail::addProcedure(*_s, name, fx);
}

void lisp::addVariable(const string &name, cell value) {
	*detail::pushVariable(*_s, name, 1) = value;
}

}}
