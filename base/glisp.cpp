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

const string cell::getStr() const {
	if (type == typeInt) return strs(i);
	else if (type == typeIdentifier) return s;
	return "";
}

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
cells_t stack;
vars_t variables;

// auxiculary memory
lists_t lists;

// shortcuts to constants
cell_t c_nil;
cell_t c_t;

// comparsion operator for variables (compares by address)
bool operator<(const var_key_t &a, const var_key_t &b) {
	return std::get<1>(a) < std::get<1>(b);
}

// detach variable to aux memory
cell_t detachVariable(cell_t addr, bool copySelf, cell_t val_begin, cell_t val_end, var_key_t key) {
	// move data
	auto e = lists.find(key);
	if (e == lists.end()) {
		cells_t &m = lists[key];

		// reserve space
		size_t selfSize = copySelf ? countElements(addr) : 0;
		size_t valSize = std::distance(val_begin, val_end);
		m.reserve(selfSize + valSize);

		// copy contents
		if (copySelf)
			m.insert(m.end(), addr, addr + selfSize);
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

// checks if given iterator is valid
bool isVariableValid(var_t i) {
	return i != variables.end();
}

// search for variable in index by name
var_t findVariable(const string &name) {
	auto ret = find_if_backwards(variables.begin(), variables.end(),
								 [name](const var_key_t &e) {
									 return std::get<0>(e) == name;
								 });

	// just prints info that variable is not found
	if (!isVariableValid(ret)) {
		// variable name not found
		dout("variable \"" << name << "\" not found" << std::endl);
		for(auto v : variables) {
			dout(" > " << std::get<0>(v) << " = " << std::get<1>(v) << std::endl);
		}
	}

	return ret;
}

// get variable address on stack
cell_t getVariableStackAddress(var_t v) {
	return stack.begin() + std::get<1>(*v);
}

string &getVariableName(var_t v) {
	return std::get<0>(*v);
}

size_t &getVariablePosition(var_t v) {
	return std::get<1>(*v);
}

// gets address to variable data
cell_t getVariableAddress(var_t v) {
	cell_t addr = getVariableStackAddress(v);

	if (addr->type == cell::typeDetach)
		// TODO: check?
		return lists[*v].begin();
	return addr;
}

// gets detached memory vector
cells_t &getVariableContainer(var_t v) {
	// TODO: check?
	return lists[*v];
}

// gather all variable data <is valid, is detached, data address, variable it, container>
std::tuple<bool, bool, cell_t, var_t, cells_t*> fetchVariable(cell_t addr) {
	const string &name = addr->s;
	var_t var = findVariable(name);
	bool isValid = isVariableValid(var);
	if (isValid) {
		bool isDetached = getVariableStackAddress(var)->type == cell::typeDetach;
		if (isDetached)
			return std::make_tuple(true, true,
								   getVariableContainer(var).begin(),
								   var,
								   &getVariableContainer(var));
		return std::make_tuple(true, false, getVariableStackAddress(var), var, &stack);
	}
	return std::make_tuple(false, false, stack.end(), var, &stack);
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
	dout("push variable (" << name << ") addr: " << std::distance(stack.begin(), addr)
		 << " value: " << toString(addr) << std::endl);
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

// push one element to stack (carefull with lists!)
cell_t pushCell(cell c) {
	stack.push_back(c);
	return stack.begin() + stack.size() - 1;
}

// push cdr of given list (create list without first element)
cell_t pushCdr(cell_t l) {
	cell_t whence = stack.end();
	size_t elemsCount = countElements(l);
	if (elemsCount > 0) {
		size_t elemsCountFirst = countElements(l + 1);
		size_t realElemsCount = elemsCount - elemsCountFirst;
		stack.push_back(cell(cell::typeList, l->i - 1)); // logical elements without first element
		stack.resize(stack.size() + realElemsCount - 1);
		std::copy(l + 1 + elemsCountFirst, l + elemsCount, whence + 1);
		return whence;
	}
	return c_nil;
}

// push car of given list (if it is a list -> copy all items)
cell_t pushCar(cell_t l) {
	// TODO: test
	cell_t whence = stack.end();
	size_t elems = countElements(l + 1);
	if (elems > 0) {
		stack.resize(stack.size() + elems);
		std::copy(l + 1, l + 1 + elems, whence);
		return whence;
	}
	return c_nil;
}

// get address (just size_t number - do not use in logic code)
size_t getAddress(cell_t c) {
	return std::distance(stack.begin(), c);
}

// call stack
call_stack_t callStack;

void tab() {
	for(size_t i = 0; i < callStack.size(); ++i) {
		dout("  ");
	}
}

void printVariables() {
	dout("defined variables(" << variables.size() << ")" << std::endl);
	for (auto &v : variables) {
		dout(std::get<0>(v) << " = " << stack[std::get<1>(v)].getStr() << std::endl);
	}
}

void printState() {
	// reverse call stack
	auto cs = callStack;
	call_stack_t rcs;
	while (cs.size()) {
		rcs.push(cs.top());
		cs.pop();
	}

	// print all elements
	size_t varsLeft = variables.size();
	for (size_t i = 0; i < stack.size(); ++i) {
		// put call stack bottom frame
		if (rcs.size() > 0 && i == rcs.top()) {
			dout("| ");
			rcs.pop();
		}

		// variable name
		auto v = std::find_if(variables.begin(), variables.end(), [&i](var_key_t &v) {
				return std::get<1>(v) == i;
			});
		if (v != variables.end()) {
			--varsLeft;
			dout("#" << std::get<0>(*v) << " ");
		}

		// print element
		auto &e = stack[i];
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
			dout("[detach:" << e.i << ":" << e.j << "] ");
		}
	}

	// print last call stack frame
	if (rcs.size() > 0 && rcs.top() == stack.size()) {
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

void pushCallStack() {
	callStack.push(stack.size());
}

void popVariablesAbove(size_t addr) {
	// no variables defined with address above given (warning - asserting that variables is not empty)
	if (addr <= std::get<1>(variables.back())) {
		// there are some variables above addr find last (should always delete sth)
		variables.erase(backwards_until(variables.begin(), variables.end(),
										[&addr](var_key_t var) {
											return std::get<1>(var) < addr;
										}), variables.end());
	}
}

void popVariablesInRange(size_t begin, size_t end) {
	// TODO: test
	auto lb = std::lower_bound(variables.begin(), variables.end(), std::make_tuple("", begin));
	auto ub = std::upper_bound(lb + 1, variables.end(), std::make_tuple("", end));
	variables.erase(lb, ub);
}

void eraseCell(cell_t addr) {
	// TODO: test
	cell_t e = addr + countElements(addr);
	popVariablesInRange(std::distance(stack.begin(), addr), std::distance(stack.begin(), e));
	stack.erase(addr, e);
}

void popCallStack() {
	// delete variables
	popVariablesAbove(callStack.top());

	// "free" data
	stack.resize(callStack.top());
	callStack.pop();
}

// pops call stack and leaves given cell at bottom of current stack frame
cell_t popCallStackLeaveData(cell_t addr) {
	// undefine all variables on this stack frame
	popVariablesAbove(callStack.top());

	// copy data
	size_t elemsCount = countElements(addr);
	cell_t whence = stack.begin() + callStack.top();
	std::copy(addr, addr + elemsCount, whence); // safe, not overlapping (src > dst)

	// remove unused data and pop call stack frame
	stack.resize(callStack.top() + elemsCount);
	callStack.pop();
	return whence;
}

// pops call stack and leaves stack untouched
cell_t popCallStackLeaveData() {
	cell_t ret = stack.begin() + callStack.top();
	callStack.pop();
	return ret;
}

// removes unused (unbound) data from top stack frame
void sweepStack() {
	// stack offsets - everything 'below' is valid
	size_t stackTopOffset = callStack.top();
	cell_t stackTop = stack.begin() + stackTopOffset;

	// searches for first variable in stack frame
	auto findFirstVar = [&stackTopOffset /*capture variables*/]() -> var_t {
		return std::next(find_if_backwards(variables.begin(), variables.end(),
										   [&stackTopOffset](const var_key_t &var) {
											   return std::get<1>(var) < stackTopOffset;
										   }));
	};

	// relocate all variables to beginning of stack
	auto varTop = findFirstVar();
	while (varTop != variables.end()) {
		size_t &srcAddrOffset = std::get<1>(*varTop);
		cell_t srcAddr = stack.begin() + srcAddrOffset;
		size_t elements = countElements(srcAddr);

		// erase - detach (unbound variable, 0 cells moved)
		if (srcAddr->type == cell::typeDetach && srcAddr->j == -1) {
			variables.erase(varTop);
			elements = 0;
		}
		else {
			// change variable address
			srcAddrOffset -= srcAddrOffset - stackTopOffset;

			// it's detached memory - copy only 1 element (ID)
			if (srcAddr->type == cell::typeDetach) {
				elements = 1;
				srcAddr->i = 1; // TODO: also keep count valid?
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
	stack.resize(stackTopOffset);
}

// TODO: removes all unused data (moved previously by detach) - rebuild callstack
// void sweepAll();

//- initialization consts and intrinsics -
intrinsics_t intrinsics;

cell_t c_message(cell_t c) {
	// *c - count
	// *(c + x) - element x
	dout("> message: " << (c + 1)->i << std::endl);
	return c;
}

cell_t c_mul(cell_t c) {
	int r = 1;
	for(cell_t i = c + 1; i != c + 1 + c->i; ++i) {
		r *= i->i;
	}
	dout(" > mul: " << r << std::endl);
	return pushCell(cell(cell::typeInt, r));
}

// search for intrinsic address
intrinsic_t getIntrinsic(const string &name) {
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

//- eval -
cell_t eval(cell_t d, bool temporary = false);

// helper for evaluating lists, evals all elements and returns last
cell_t evalreturn(cell_t begin, cell_t end) {
	cell_t lastResult = c_nil; // return nil when evaluating empty list

	// evaluate all cells
	for (cell_t i = begin; i != end; i = nextCell(i))
		lastResult = eval(i);

	return lastResult;
}

// evals all elements leaving results on stack (same as above but no address returned)
void evalNoStack(cell_t begin, cell_t end) {
	for (; begin != end; begin = nextCell(begin))
		eval(begin);
}

// calls [op] for all (evaluated) elements of list
template <typename T_OP>
void evalmap(cell_t begin, cell_t end, T_OP op) {
	for (cell_t i = begin; i != end; i = nextCell(i)) {
		pushCallStack();
		cell_t lastResult = eval(i);
		op(lastResult);
		popCallStack();
	}
}

cell_t eval(cell_t d, bool temporary) {
	tab(); dout("eval: " << toString(d) << std::endl);

	if (d->type == cell::typeInt) {
		// when temporary is true - return value directly (it's in input array!)
		if (temporary)
			return d;

		// leave it on stack
		stack.push_back(*d);
		return --stack.end();
	}
	else if (d->type == cell::typeIdentifier) {
		var_t var = findVariable(d->s);
		if (isVariableValid(var)) {
			auto addr = getVariableAddress(var);

			// return temporary result
			if (temporary)
				return addr;

			// leave list on stack
			if (addr->type == cell::typeList)
				return pushData(addr);

			// it's an atom
			stack.push_back(*addr);
			return --stack.end();
		}
		// variable not found
	}
	else if (d->type == cell::typeList) {
		// empty list evaluates to nil
		if (d->i == 0)
			return c_nil;

		// first argument must be identifier
		cell_t fxName = d + 1;
		if ((d + 1)->type == cell::typeList) {
			// first list element is list -> evaluating lambda
			return eval(d + 1, temporary);
		}
		else if (fxName->type != cell::typeIdentifier) {
			dout("function name must be ID" << std::endl);
			return c_nil;
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
			cell_t varValue = eval(d + 3);
			pushVariable(varName->s, varValue);

			// return identifier of created variable
			return pushCell(*varName);
		}
		else if (fxName->s == "quote") {
			// return back source, caller will only fetch data
			if (temporary)
				return d + 2;

			// just copy quote body to stack
			return pushData(d + 2);
		}
		else if (fxName->s == "lambda") {
			// copy cdr of lambda, first element is "lambda" identifier, we dont need it
			return pushCdr(d);
		}
		else if (fxName->s == "+") {
			int sum = 0;
			evalmap(firstCell(d) + 1, endCell(d), [&sum](cell_t i){ sum += i->i; });

			// leave return value on stack
			return pushCell({cell::typeInt, sum});
		}
		else if (fxName->s == "if") {
			// test, we dont need return value - discard it with call stack
			pushCallStack();
			cell_t testi = eval(d + 2, true);
			bool test = testi->s != "nil" || testi->type != cell::typeIdentifier;

			// test and eval
			if (test)
				return popCallStackLeaveData(eval(endCell(d + 2)));
			else {
				cell_t offset = endCell(endCell(d + 2)); // else statements offset
				return popCallStackLeaveData(evalreturn(offset, endCell(d)));
			}
		}
		else if (fxName->s == "=") {
			// d->i must be > 2
			pushCallStack();
			cell_t a1 = eval(d + 2);
			cell_t a2 = eval(nextCell(d + 2));
			if (a1->type == cell::typeInt && a2->type == cell::typeInt) {
				bool t = a1->i == a2->i;
				popCallStack();
				return t ? c_t : c_nil;
			}
			popCallStack();
			return c_nil;
		}
		else if (fxName->s == "progn") {
			// d->i must be > 1
			pushCallStack();
			return popCallStackLeaveData(evalreturn(d + 2, endCell(d)));
		}
		else if (fxName->s == "let") {
			// evaluate and push variables
			pushCallStack();
			cell_t args = d + 2;
			for (cell_t a = firstCell(args); a != endCell(args); a = nextCell(a)) {
				cell_t val = eval(a + 2);
				pushVariable((a + 1)->s, val);
			}

			// evaluate function body
			return popCallStackLeaveData(evalreturn(nextCell(args), endCell(d)));
		}
		else if (fxName->s == "boundp") {
			// eval(d + 2) must be ID
			return isVariableValid(findVariable(eval(d + 2, true)->s)) ? c_t : c_nil;
		}
		else if (fxName->s == "unbound") {
			// eval(d + 2) must be ID (we are not using stack frames)
			cell r = *eval(d + 2, true);

			// find and tag variable address as detached
			var_t var = findVariable(r.s);
			if (isVariableValid(var)) {
				cell_t addr = getVariableAddress(var);
				*addr = {cell::typeDetach, addr->i, -1, ""};
			}

			// return argument back
			return pushCell(r);
		}
		else if (fxName->s == "list") {
			// empty list evaluates to nil
			if (d->i < 2)
				return c_nil;

			// create list and eval its elements
			auto ret = stack.begin() + stack.size();
			stack.push_back({cell::typeList, d->i - 1});
			evalNoStack(d + 2, endCell(d));
			return ret;
		}
		else if (fxName->s == "listp") {
			// d->i > 1
			pushCallStack();
			auto ret = eval(d + 2, true)->type != cell::typeList ? c_nil : c_t;
			popCallStack();
			return ret;
		}
		else if (fxName->s == "car") {
			// d->i must be > 1
			auto arg = d + 2;

			// we could return temporary only if arg is not list
			if (temporary && arg->type != cell::typeList) {
				auto l = eval(arg, true); // guaranteed not to leave anything on stack
				if (l->type == cell::typeList) {
					if (l->i > 0)
						return l + 1;
					else return c_nil;
				}
				return l;
			}

			// leave whole list on stack
			pushCallStack();
			auto r = eval(arg);
			if (r->type == cell::typeList) {
				if (r->i > 0)
					return popCallStackLeaveData(r + 1);

				// we've got list with no elements - discard r and return nil
				popCallStack();
				return c_nil;
			}
			popCallStack();
			return r;
		}
		else if (fxName->s == "cdr") {
			// d->i > 1
			auto arg = d + 2;

			// we cant return temporary result
			pushCallStack();
			auto r = eval(arg);
			if (r->type == cell::typeList) {
				if (r->i > 1) {
					r->i--; // we're erasing one element - update list's el count
					eraseCell(r + 1);
					return popCallStackLeaveData();
				}

				// empty list
				popCallStack();
				return c_nil;
			}
			popCallStack();
			return r;
		}
		else if (fxName->s == "nth") {
			// d->i > N
			// calc N
			pushCallStack();
			int n = eval(d + 2, true)->i;

			// find address
			cell_t nth = eval(d + 3);
			if (nth->type != cell::typeList){
				popCallStack();
				return c_nil; // error?
			}

			// nth->i must be < N
			// find nth element in list and return result
			return popCallStackLeaveData(nthCell(nth, n));
		}
		else if (fxName->s == "defun") {
			// d->i > 4
			cell_t fnName = d + 2;

			// just put list (lambda) on stack
			cell_t va = pushCell({cell::typeList, d->i - 2}); // without "defun" and <fnName>
			stack.insert(stack.end(), d + 3, endCell(d)); // copy args and body
			pushVariable(fnName->s, va);

			// return function id
			return pushCell(*fnName);
		}
		else if (fxName->s == "setq" || fxName->s == "set") {
			pushCallStack();

			// gather data
			const bool isSetq = fxName->s == "setq";
			bool isValid;
			bool isDetached;
			cell_t addr;
			var_t var;
			cells_t *cont;
			std::tie(isValid, isDetached, addr, var, cont) =
				fetchVariable(isSetq ? (d + 2) : eval(d + 2, true));

			// perform variable replace
			if (isValid) {
				cell_t val = eval(nextCell(d + 2));
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
						addr = detachVariable(addr, false, val, val + countElements(val), *var);
					}
				}
			}

			// return address and pop all temporary mess
			popCallStack();
			return addr;
		}
		else if (fxName->s == "length") {
			pushCallStack();
			cell_t addr = eval(d + 2, true);
			int len = addr->type == cell::typeList ? addr->i : 1;
			popCallStack();
			return pushCell({cell::typeInt, len});
		}
		else if (fxName->s == "push") {
			// addr must be list, d + 3 must be id
			pushCallStack();
			bool isValid;
			bool isDetached;
			cell_t addr;
			var_t var;
			cells_t *cont;
			std::tie(isValid, isDetached, addr, var, cont) = fetchVariable(nextCell(d + 2));

			// push variable at list's end
			if (isValid) {
				cell_t val = eval(d + 2);

				// insert cell
				if (isDetached) {
					cont->insert(cont->end(), val, val + countElements(val));
					addr = cont->begin();
				}
				else addr = detachVariable(addr, true, val, val + countElements(val), *var);

				// update count
				addr->i += 1;
			}

			// return address and pop all temporary mess
			popCallStack();
			return addr;
		}
		else if (fxName->s == "pop") {
			// addr must be list, d + 3 must be id
			pushCallStack();
			bool isValid;
			bool isDetached;
			cell_t addr;
			var_t var;
			cells_t *cont;
			std::tie(isValid, isDetached, addr, var, cont) = fetchVariable(d + 2);

			// pop variable and return it's value
			if (isValid) {
				// find last element and leave it on the stack
				cell_t last = lastCell(addr);
				cell_t ret = pushData(last);

				// note: free space only for detached!
				if (isDetached)
					cont->erase(last, cont->end());

				// update count
				addr->i -= 1;

				// return value
				return popCallStackLeaveData(ret);
			}

			// return address and pop all temporary mess
			popCallStack();
			return addr;
		}
		else if (fxName->s == "append") {
			// addr must be list, d + 3 must be id
			pushCallStack();
			bool isValid;
			bool isDetached;
			cell_t addr;
			var_t var;
			cells_t *cont;
			std::tie(isValid, isDetached, addr, var, cont) = fetchVariable(d + 2);

			// merge 2 lists
			if (isValid) {
				// eval target value
				cell_t val = eval(nextCell(d + 2), true);
				size_t elems = countElements(val);

				// merging
				if (isDetached) {
					cont->insert(cont->end(), val + 1, val + elems);
					addr = cont->begin(); // addr may be invalid at this point
				}
				else addr = detachVariable(addr, true, val + 1, val + elems, *var);

				// update count (-1 because countElements includes list header)
				addr->i += elems - 1;
			}

			// return address and pop all temporary mess
			popCallStack();
			return addr;
		}
		else if (fxName->s == "setcar") {

		}

		/*
		 * setcar
		 * setcdr

		 * add-to-list (unique)
		 * add-to-ordered-list (sorted)
		 * reverse
		 * sort
		 * member
		 * delete
		 ** assoc **
		 */

		//- functions evaluation -
		// get fx address
		var_t var = findVariable(fxName->s);
		if (isVariableValid(var)) {
			cell_t fx = getVariableAddress(var);
			if (fx->type == cell::typeList) {
				// [list:][list:]<arg><arg>[list:]<body><body>[list:]<body>...
				// function stack frame
				pushCallStack();

				// evaluate and bind args
				cell_t args = fx + 1;
				cell_t args_vals = d + 2; // skip list and fx name
				cell_t args_vals_i = args_vals;
				for (int i = 0; i < args->i; ++i) {
					auto v = eval(args_vals_i);
					args_vals_i = nextCell(args_vals_i);
					pushVariable((args + i + 1)->s, v);
				}

				// evaluate body
				cell_t body = nextCell(args);
				return popCallStackLeaveData(evalreturn(body, endCell(fx)));
			}
			dout("not function!" << std::endl);
			return fx;
		}
		else {
			intrinsic_t i = getIntrinsic(fxName->s);
			if (isIntrinsicValid(i)) {
				pushCallStack();

				// arguments list address
				cell_t r = stack.end();

				// evaluate arguments (leave result on stack)
				pushCell(cell(cell::typeList, d->i - 1)); // list elements count (not counting name)
				evalNoStack(d + 2, endCell(d));

				// call intrinsic
				return popCallStackLeaveData(std::get<1>(*i)(r));
			}
			dout("intrinsic not found" << std::endl); // not a function?
		}
	}

	// tmp, should report some error (syntax, logic error?)
	return stack.end();
}

}


//- client side -
void lisp::init(size_t memSize) {
	detail::init(memSize);
	detail::pushCallStack();
}

void lisp::close() {
	detail::popCallStack();
}

string lisp::eval(const string &s) {
	dout(std::endl << std::endl);
	auto code = detail::parse(s);
	dout(detail::toString(code) << std::endl);
	auto retAddr = detail::eval(code.begin(), true);
	string r = detail::toString(retAddr);
	dout("return addr: " << detail::getAddress(retAddr)
		 << " | " << detail::toString(retAddr) << std::endl);
	detail::printState();
	dout("sweep..." << std::endl);
	detail::sweepStack();
	detail::printState();
	return r;
}

void lisp::addIntrinsic(const string &name, intrinsic_fx_t fx) {

}

void lisp::addVariable(const string &name, cell value) {

}

}}
