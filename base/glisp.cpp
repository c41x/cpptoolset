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

#include "glisp.hpp"
#include "math.string.hpp"
#include "tokenizer.hpp"
#include "gstdlib.hpp"

// error checking
#ifdef GLISP_DEBUG_ERROR
#define derr_ERR(...) gassert(false, strs(__VA_ARGS__));
#else
#define derr_ERR(...)
#endif

#ifdef GLISP_DEBUG_ERROR_ARRAY
#define derr_ERR_ARR(...) s.errors.push_back(strs(__VA_ARGS__));
#else
#define derr_ERR_ARR(...)
#endif

#ifdef GLISP_DEBUG_ERROR_STDOUT
#define derr_ERR_STD(...) std::cout << strs(__VA_ARGS__) << std::endl;
#else
#define derr_ERR_STD(...)
#endif

#if defined(GLISP_DEBUG_ERROR) || defined(GLISP_DEBUG_ERROR_ARRAY) || defined(GLISP_DEBUG_ERROR_STDOUT)
#define derr(COND, ...) if (!(COND)) { derr_ERR(__VA_ARGS__); derr_ERR_ARR(__VA_ARGS__); derr_ERR_STD(__VA_ARGS__); } else;
#define derrr(COND, RET, ...) if (!(COND)) { derr_ERR(__VA_ARGS__); derr_ERR_ARR(__VA_ARGS__); derr_ERR_STD(__VA_ARGS__); return RET; } else;
#define derrnil(COND, RET, ...) if (!(COND)) { derr_ERR(__VA_ARGS__); derr_ERR_ARR(__VA_ARGS__); derr_ERR_STD(__VA_ARGS__); return s.c_nil; } else;
#define derrpnil(COND, ...) if (!(COND)) { derr_ERR(__VA_ARGS__); derr_ERR_ARR(__VA_ARGS__); derr_ERR_STD(__VA_ARGS__); return pushCell(s, s.c_nil, temporary); } else;
#define derrppnil(COND, ...) if (!(COND)) { derr_ERR(__VA_ARGS__); derr_ERR_ARR(__VA_ARGS__); derr_ERR_STD(__VA_ARGS__); return popCallStackLeaveData(s, s.c_nil, temporary); } else;
#else
#define derr(COND, ...)
#define derrnil(COND, ...)
#define derrpnil(COND, ...)
#define derrppnil(COND, ...)
#endif

#ifdef GLISP_DEBUG_STATE
#define ddebnt(...) std::cout << strs(__VA_ARGS__) << std::endl;
#define ddeb(...) for (size_t i = 1; i < s.callStack.size(); ++i) std::cout << "  "; std::cout << strs(__VA_ARGS__) << std::endl;
#define ddebc(COND, ...) if (!(COND)) std::cout << strs(__VA_ARGS__) << std::endl; else ;
#else
#define ddeb(...)
#define ddebnt(...)
#define ddebc(COND, ...)
#endif

namespace granite { namespace base {
//- cell -
// iterating over lists: first element
cell_t firstCell(cell_t c) {
	return c + 1;
}

// iterating over lists: ++
cell_t nextCell(cell_t c) {
	cell_t delim = c;
	while (c <= delim) {
		if(c->type == cell::typeList || c->type == cell::typeDetach)
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
	else if (c->type == cell::typeString)
		return strs("\"", c->s, "\"");
	return c->getStr();
}

// convert containing variant value to string
const string cell::getStr() const {
	if (type == typeInt) return strs(i);
	else if (type == typeIdentifier || type == typeString) return s;
	else if (type == typeInt64) return strs(ii);
	else if (type == typeFloat) return strs(f);
	else if (type == typeVector) return toStr(vec4f(v4));
	return "";
}

const bool operator==(const cell &l, const cell &r) {
	if (l.type == r.type) {
		if (l.type == cell::typeInt ||
			l.type == cell::typeList ||
			l.type == cell::typeDetach) return l.i == r.i;
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

bool cellsEqual(cell_t a, cell_t b) {
	if (a->type == cell::typeList && a->type == b->type && a->i == b->i) {
		int elems = countElements(a);
		for (int i = 0; i < elems; ++i)
			if (*(a + i) != *(b + i))
				return false;
		return true;
	}
	return *a == *b;
}

// checks if all cells in range are type t
bool cellsType(cell_t begin, cell_t end, cell::type_t t) {
	for (; begin != end; begin = nextCell(begin))
		if (begin->type != t)
			return false;
	return true;
}

bool isNil(cell_t c) {
	return c->type == cell::typeIdentifier && c->s == "nil";
}

// operator generator for all types
std::function<void(cell_t)> getOperator(cell &acc, auto op) {
	switch (acc.type) {
		case cell::typeInt: return [&acc,&op](cell_t c) { op(acc.i, c->i); }; break;
		case cell::typeInt64: return [&acc,&op](cell_t c) { op(acc.ii, c->ii); }; break;
		case cell::typeFloat: return [&acc, &op](cell_t c) { op(acc.f, c->f); }; break;
		default: return [](cell_t){};
	}
}

// search for element in list, return element or lst if not found
cell_t findCell(cell_t lst, cell_t e) {
	cell_t end = endCell(lst);
	for (cell_t i = firstCell(lst); i != end; i = nextCell(i))
		if (cellsEqual(e, i))
			return i;
	return lst;
}

// search for element in list, return t / nil if found / not found
bool cellFound(cell_t lst, cell_t e) {
	return lst != findCell(lst, e);
}

// search for first found key in list
cell_t findValue(cell_t lst, cell_t key) {
	for (cell_t it = firstCell(lst); it != endCell(lst); it = nextCell(it)) {
		if (cellsEqual(key, it + 1))
			return it;
	}
	return lst;
}

// same as above but returns true/false
bool valueFound(cell_t lst, cell_t key) {
	return lst != findValue(lst, key);
}

// some static constants
const cell cell::nil = cell(cell::typeIdentifier, "nil");
const cell cell::t = cell(cell::typeIdentifier, "t");

//- state -
typedef std::tuple<string, size_t> var_key_t; // name, stack position
typedef std::vector<var_key_t> vars_t;
typedef std::set<var_key_t> list_pool_t;
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
	list_pool_t listsPool;

	// shortcuts to constants
	cell_t c_nil;
	cell_t c_t;

	// stack containing frames begin
	call_stack_t callStack;

	// procedures list (user fx)
	procedures_t procedures;

	// error list
	#ifdef GLISP_DEBUG_ERROR_ARRAY
	std::vector<string> errors;
	#endif
};

namespace detail {
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
		tokenQuote,
		tokenVec
	};

	// define rules
	tokenizer tok;
	tok.addRule(tokenWhiteSpace, true, " \t\v\n\r");
	tok.addRule(tokenOpenPar, false, "(");
	tok.addRule(tokenClosePar, false, ")");
	tok.addRule(tokenComment, true, ";", "", "\n\r");
	tok.addRule(tokenString, false, "\"", "\\", "\"");
	tok.addRule(tokenQuote, false, "\'");
	tok.addRule(tokenVec, false, "|", "", "|");

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
		else if (t.id == tokenSymbol || t.id == tokenString || t.id == tokenVec) {
			// adds new element
			onNewElement();

			// determine token type and add atom to cells
			if (t.id == tokenString)
				cells.push_back(cell(t.value));
			else if (t.id == tokenVec)
				cells.push_back(cell(fromStr<vec4f>(t.value)));
			else if (isInteger(t.value))
				cells.push_back(cell(fromStr<int>(t.value)));
			else if (isFloat(t.value))
				cells.push_back(cell(fromStr<float>(t.value)));
			else cells.push_back(cell(cell::typeIdentifier, t.value));

			// after element is added - check if we need to close quote
			checkQuoteDelim();
		}
	}

	return cells;
}

//- dynamic scoping / stack / variable memory -
// comparsion operator for variables (compares by address)
bool operator<(const var_key_t &a, const var_key_t &b) {
	return std::get<1>(a) < std::get<1>(b);
}

// detach variable to aux memory
cells_t &detachVariable(lispState &s, cell_t addr, var_key_t key) {
	// initialize storage
	auto e = s.lists.find(key);
	if (e == s.lists.end()) {
		// if we have something in pool -> use it (move semantics)
		if (s.listsPool.size() > 0) {
			auto pe = s.lists.find(*s.listsPool.begin());
			e = s.lists.insert({key, std::move(pe->second)}).first;
			s.lists.erase(pe);
			s.listsPool.erase(s.listsPool.begin());
		}
		else
			// or create new one
			e = s.lists.insert({key, cells_t()}).first;
	}

	// reserve space
	size_t selfSize = countElements(addr);
	// it's ok - shrinking vector does not reallocate memory
	// its neccessary because there may be some garbage in vector
	cells_t &m = e->second;
	m.resize(0);
	m.reserve(selfSize);

	// copy contents
	m.insert(m.end(), addr, addr + selfSize);

	// leave id on stack
	*addr = {cell::typeDetach,
			 addr->type == cell::typeList ? addr->i : 1,
			 (int)std::get<1>(key), std::get<0>(key)};

	// return container
	return m;
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
	ddebc(isVariableValid(s, ret), "variable \"", name, "\" not found");

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

	if (addr->type == cell::typeDetach) {
		derrnil(s.lists.find(*v) != s.lists.end(), "detach variable \"", getVariableName(v), "\" empty");
		return s.lists[*v].begin();
	}
	return addr;
}

// gets detached memory vector
cells_t &getVariableContainer(lispState &s, var_t v) {
	derrr(s.lists.find(*v) != s.lists.end(), s.stack, "detach variable \"", getVariableName(v), "\" empty");
	return s.lists[*v];
}

// template to list operations, call order:
// - peekOp - access to list itself
// - detachOp (optional) - if yields true -> variable is detached
// - processOp - operation, access to list pointer and container
cell_t listOp(lispState &s, cell_t addr,
			  std::function<void(cells_t&, cell_t)> processOp,
			  std::function<bool()> detachOp = []() { return true; },
			  std::function<void(cell_t)> peekOp = [](cell_t) {}) {
	// if list is placed on stack -> allow to modify - just be careful,
	// list must be on stack top, otherwise cells may have invalid iterators after processOp call
	if (addr->type == cell::typeList) {
		peekOp(addr);
		processOp(s.stack, addr);
		return addr;
	}

	// if this is not ID -> signal error
	derrnil(addr->type == cell::typeIdentifier, "list operation failed: expected identifier: ", toString(addr));

	// search for variable
	var_t var = findVariable(s, addr->s);
	if (isVariableValid(s, var)) {
		cell_t stackAddr = getVariableStackAddress(s, var);
		cells_t *con = nullptr;
		if (stackAddr->type != cell::typeDetach) {
			// maybe detach
			peekOp(stackAddr);
			if (detachOp()) {
				con = &detachVariable(s, stackAddr, *var);
			}
			else {
				// or allow to modify stack directly
				processOp(s.stack, stackAddr);
				return stackAddr;
			}
		}

		// already detached
		if (!con) {
			con = &getVariableContainer(s, var);
			peekOp(con->begin());
		}

		// continue with detached
		processOp(*con, con->begin());
		return con->begin();
	}

	derr(false, "list operation failed: variable \"", addr->s, "\" not found");
	return s.c_nil;
}

// push variable and allocate memory
cell_t pushVariable(lispState &s, const string &name, size_t count) {
	ddeb("pushing variable \"", name, "\" on stack top, count: ", count);

	// add new variable
	s.variables.push_back(std::make_tuple(name, s.stack.size()));

	// resize stack and return iterator to first allocated element
	s.stack.resize(s.stack.size() + count);
	return s.stack.begin() + std::get<1>(s.variables.back());
}

// assign address to memory
void pushVariable(lispState &s, const string &name, cell_t addr) {
	ddeb("pushing variable \"", name, "\" addr: ", std::distance(s.stack.begin(), addr), " value: ", toString(addr));

	// find address offset
	size_t dst = std::distance(s.stack.begin(), addr);
	bool inOrder = std::get<1>(s.variables.back()) < dst;
	derr(inOrder, "inserting variable \"", name, "\" in wrong order, data corrupted");

	// must insert variables in order
	if (inOrder)
		s.variables.push_back(std::make_tuple(name, dst));
}

// push data on top of stack (copy all addr cell) return data address on stack
cell_t pushList(lispState &s, cell_t addr) {
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

// same as above but with temporary support
cell_t pushCell(lispState &s, cell_t c, bool temporary) {
	if (temporary)
		return c;
	return pushCell(s, *c);
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

string getState(lispState &s) {
	// reverse call stack
	auto cs = s.callStack;
	call_stack_t rcs;
	while (cs.size()) {
		rcs.push(cs.top());
		cs.pop();
	}

	// list of unlisted detach variables
	std::set<var_key_t> keysLeft;
	for (const auto &e : s.lists)
		keysLeft.insert(e.first);

	// print all elements
	string out;
	size_t varsLeft = s.variables.size();
	for (size_t i = 0; i < s.stack.size(); ++i) {
		// put call stack bottom frame
		while (rcs.size() > 0 && i == rcs.top()) {
			out += "| ";
			rcs.pop();
		}

		// variable name
		auto v = std::find_if(s.variables.begin(), s.variables.end(), [&i](var_key_t &v) {
				return std::get<1>(v) == i;
			});
		if (v != s.variables.end()) {
			--varsLeft;
			out += strs("#", getVariableName(v), " ");
		}

		// print element
		auto &e = s.stack[i];
		if (e.type == cell::typeList) {
			out += strs("[list:",e.i, "] ");
		}
		else if (e.type == cell::typeIdentifier) {
			out += strs(e.s, " ");
		}
		else if (e.type == cell::typeFloat) {
			out += strs(e.f, " ");
		}
		else if (e.type == cell::typeString) {
			out += strs("\"", e.s, "\" ");
		}
		else if (e.type == cell::typeInt) {
			out += strs(e.i, " ");
		}
		else if (e.type == cell::typeVector) {
			out += strs("/", vec4f(e.v4), "/ ");
		}
		else if (e.type == cell::typeDetach) {
			out += strs("[detach:", e.i, ":", e.j, ":",
						(e.j != -1 ? toString(getVariableAddress(s, v)) : "?"), "] ");
			keysLeft.erase(*v);
		}
	}

	if (rcs.size() > 0) {
		bool firstFrame = rcs.size() == 1;
		if (!firstFrame)
			out += strs("\ncall stack corrupted: ", rcs.size(), "frames");
	}

	if (varsLeft > 0) {
		out += strs("\nvariables corrupted: ", varsLeft, " variables");
	}

	if (keysLeft.size() > 0) {
		out += "\nhanging detached variables: ";
		for (const auto &e : keysLeft)
			out += strs("<", std::get<0>(e), "> ");
	}

	if (s.listsPool.size() > 0) {
		out += "\ndetached variables pool: ";
		for (const auto &e : s.listsPool)
			out += strs("<", std::get<0>(e), "> ");
	}

	return out;
}

void pushCallStack(lispState &s) {
	s.callStack.push(s.stack.size());
}

void popVariablesAbove(lispState &s, size_t addr) {
	// no variables defined with address above given (warning - asserting that variables is not empty)
	if (addr <= std::get<1>(s.variables.back())) {
		// there are some variables above addr find last (should always delete sth)
		s.variables.erase(backwards_until(s.variables.begin(), s.variables.end(),
										  [&s, &addr](var_key_t var) {
											  // stop
											  if (std::get<1>(var) < addr)
												  return true;

											  // continue iterating
											  // we have variable to delete -> throw to pool if needed
											  if (s.lists.end() != s.lists.find(var))
												  s.listsPool.insert(var);
											  return false;
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
cell_t popCallStackLeaveData(lispState &s, cell_t addr, bool temporary = false) {
	// undefine all variables on this stack frame
	popVariablesAbove(s, s.callStack.top());

	// return address
	cell_t whence;

	// copy data or just return passed value
	if (!temporary) {
		int elemsCount = countElements(addr);
		whence = s.stack.begin() + s.callStack.top();

		// we need more space
		if (s.callStack.top() + elemsCount > s.stack.size())
			s.stack.resize(s.callStack.top() + elemsCount);

		std::copy(addr, addr + elemsCount, whence); // safe, not overlapping (src > dst)

		// remove unused data
		s.stack.resize(s.callStack.top() + elemsCount);
	}
	else {
		// unused data will be sweept out
		whence = addr;
	}

	// pop call stack frame
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

	// free hanging detached (leave 5 buffer)
	for (auto &e : s.listsPool) {
		if (s.listsPool.size() <= 5)
			break;
		s.lists.erase(e);
	}

	// discard rest
	s.stack.resize(stackTopOffset);
}

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

//- eval / utils -
cell_t eval(lispState &s, cell_t d, bool temporary = false);

// helper for evaluating lists, evals all elements and returns last
cell_t evalreturn(lispState &s, cell_t begin, cell_t end, bool temporary = false) {
	cell_t lastResult = s.c_nil; // return nil when evaluating empty list

	// evaluate all cells
	for (cell_t i = begin; i != end; i = nextCell(i))
		lastResult = eval(s, i, temporary);

	return lastResult;
}

// evals all elements leaving results on stack (same as above but no address returned)
void evalNoStack(lispState &s, cell_t begin, cell_t end, bool temporary = false) {
	for (; begin != end; begin = nextCell(begin))
		eval(s, begin, temporary);
}

// calls [op] for all (evaluated) elements of list
template <typename T_OP>
void evalmap(lispState &s, cell_t begin, cell_t end, T_OP op, bool temporary = false) {
	for (cell_t i = begin; i != end; i = nextCell(i))
		op(eval(s, i, temporary));
}

// evals all list elements until [op] returns false
template <typename T_OP>
bool evalUntilUnary(lispState &s, cell_t begin, cell_t end, T_OP op, bool temporary = false) {
	for (cell_t i = begin; i != end; i = nextCell(i))
		if (!op(eval(s, i, temporary)))
			return false;
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

// evaluates op on each cell accumulating result
template <typename T>
cell_t evalAccumulate(lispState &s, cell_t first, cell_t last, T op) {
	pushCallStack(s);
	cell acc = *eval(s, first);
	evalmap(s, nextCell(first), last, getOperator(acc, op), true);
	return popCallStackLeaveData(s, pushCell(s, acc));
}

cell_t boolToCell(lispState &s, bool v) {
	return v ? s.c_t : s.c_nil;
}

// binds arguments as variables, names - list with variable id's
void bindArguments(lispState &s, cell_t names, cell_t values) {
	for (int i = 0; i < names->i; ++i) {
		auto v = eval(s, values);
		values = nextCell(values);
		pushVariable(s, (names + i + 1)->s, v);
	}
}

//- eval -
cell_t eval(lispState &s, cell_t d, bool temporary) {
	ddeb("eval: ", toString(d));

	if (d->type == cell::typeInt ||
		d->type == cell::typeString ||
		d->type == cell::typeFloat ||
		d->type == cell::typeVector) {
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
				return pushList(s, addr);

			// it's an atom
			s.stack.push_back(*addr);
			return --s.stack.end();
		}
		derr(false, "variable \"", d->s, "\" not found");
	}
	else if (d->type == cell::typeList) {
		// empty list evaluates to nil
		if (d->i == 0) {
			return pushCell(s, s.c_nil, temporary);
		}

		// first argument must be identifier or list
		cell_t fxNameCell = d + 1;
		const string &fxName = fxNameCell->s;
		if ((d + 1)->type == cell::typeList) {
			// first list element is list -> evaluating lambda
			// function stack frame
			pushCallStack(s);
			cell_t fx = eval(s, d + 1, temporary);

			// error check
			derrppnil(fx->type == cell::typeList, "evaluating function \"", fxName, "\": not a function: ", toString(fx));
			derrppnil(fx->i > 0, "evaluating empty function, data corrupted: ", toString(fx));

			// evaluate and bind args
			bindArguments(s, fx + 1, nextCell(d + 1));

			// evaluate body
			cell_t body = nextCell(fx + 1);
			return popCallStackLeaveData(s, evalreturn(s, body, endCell(fx), temporary), temporary);
		}
		else if (fxNameCell->type != cell::typeIdentifier) {
			derr(false, "syntax error: function name must be ID or list - return nil");
			return pushCell(s, s.c_nil, temporary);
		}

		//- builtins -
		// builtins rules:
		// 1) a) returns address to result
		//    b) can return temporary result (in source / stack memory)
		//    c) can use temporary memory in read only
		// 2) a) leaves stack frame at the same point as before execution
		//    b) unbounds all unused variables that ran out of scope
		if (fxName == "defvar") {
			derrpnil(d->i == 3, "defvar: expecting 2 arguments, passed: ", d->i - 1);
			derrpnil((d + 2)->type == cell::typeIdentifier, "defvar: variable name (1) must be ID");

			// check name and eval value
			cell_t varName = d + 2;
			cell_t varValue = eval(s, d + 3);
			pushVariable(s, varName->s, varValue);

			// return identifier of created variable
			return pushCell(s, varName, temporary);
		}
		else if (fxName == "quote") {
			derrpnil(d->i == 2, "quote: expecting 1 argument, passed: ", d->i - 1);

			// return back source, caller will only fetch data
			if (temporary)
				return d + 2;

			// just copy quote body to stack
			return pushList(s, d + 2);
		}
		else if (fxName == "lambda") {
			derrpnil(d->i > 2, "lambda: invalid syntax, expecting > 2 arguments, passed: ", d->i - 1);

			// copy cdr of lambda, first element is "lambda" identifier, we dont need it
			return pushCdr(s, d);
		}
		else if (fxName == "+") {
			derrpnil(d->i > 2, "+: expecting > 1 arguments, passed: ", d->i - 1);
			return evalAccumulate(s, d + 2, endCell(d), [](auto &acc, auto v) { acc += v; });
		}
		else if (fxName == "-") {
			derrpnil(d->i > 2, "-: expecting > 1 arguments, passed: ", d->i - 1);
			return evalAccumulate(s, d + 2, endCell(d), [](auto &acc, auto v) { acc -= v; });
		}
		else if (fxName == "/") {
			derrpnil(d->i > 2, "/: expecting > 1 arguments, passed: ", d->i - 1);
			return evalAccumulate(s, d + 2, endCell(d), [](auto &acc, auto v) { acc /= v; });
		}
		else if (fxName == "*") {
			derrpnil(d->i > 2, "*: expecting > 1 arguments, passed: ", d->i - 1);
			return evalAccumulate(s, d + 2, endCell(d), [](auto &acc, auto v) { acc *= v; });
		}
		else if (fxName == "if") {
			derrpnil(d->i > 2, "if: expecting > 1 arguments, passed: ", d->i - 1);

			// test, we dont need return value - discard it with call stack
			pushCallStack(s);
			cell_t testi = eval(s, d + 2, true);
			bool test = !isNil(testi);

			// test and eval
			if (test)
				return popCallStackLeaveData(s, eval(s, nextCell(d + 2), temporary), temporary);
			else if (d->i > 3) {
				cell_t offset = nextCell(nextCell(d + 2)); // else statements offset
				return popCallStackLeaveData(s, evalreturn(s, offset, endCell(d), temporary), temporary);
			}

			// return nil if there are no else statements
			return popCallStackLeaveData(s, s.c_nil, temporary);
		}
		else if (fxName == "progn") {
			derrpnil(d->i > 1, "progn: expecting > 0 arguments, passed: ", d->i - 1);
			pushCallStack(s);
			return popCallStackLeaveData(s, evalreturn(s, d + 2, endCell(d), temporary), temporary);
		}
		else if (fxName == "let") {
			derrpnil(d->i > 2, "let: expecting > 1 arguments, passed: ", d->i - 1);
			derrpnil((d + 2)->type == cell::typeList, "let: variable list invalid type (should be list)");

			// evaluate and push variables
			pushCallStack(s);
			cell_t args = d + 2;
			for (cell_t a = firstCell(args); a != endCell(args); a = nextCell(a)) {
				cell_t val = eval(s, a + 2);
				pushVariable(s, (a + 1)->s, val);
			}

			// evaluate function body
			return popCallStackLeaveData(s, evalreturn(s, nextCell(args), endCell(d), temporary),
										 temporary);
		}
		else if (fxName == "boundp") {
			derrpnil(d->i == 2, "boundp: expecting 1 argument, passed: ", d->i - 1);
			cell_t id = eval(s, d + 2, true);
			derrpnil(id->type == cell::typeIdentifier, "boundp: argument 1 is not identifier");
			return pushCell(s, boolToCell(s, isVariableValid(s, findVariable(s, id->s))), temporary);
		}
		else if (fxName == "unbound") {
			derrpnil(d->i == 2, "unbound: expecting 1 argument, passed: ", d->i - 1);

			// eval(d + 2) must be ID
			pushCallStack(s);
			cell r = *eval(s, d + 2, true);
			derrppnil(r.type == cell::typeIdentifier, "unbound: argument 1 is not identifier");

			// find and tag variable address as detached
			var_t var = findVariable(s, r.s);
			if (isVariableValid(s, var)) {
				cell_t addr = getVariableAddress(s, var);
				*addr = {cell::typeDetach, addr->i, -1, ""};
			}

			// return argument back
			popCallStack(s);
			return pushCell(s, r);
		}
		else if (fxName == "list") {
			// empty list evaluates to nil
			if (d->i < 2)
				return pushCell(s, s.c_nil, temporary);

			// create list and eval its elements
			auto ret = s.stack.begin() + s.stack.size();
			s.stack.push_back({cell::typeList, d->i - 1});
			evalNoStack(s, d + 2, endCell(d));
			return ret;
		}
		else if (fxName == "listp") {
			derrpnil(d->i == 2, "listp: expecting 1 argument, passed: ", d->i - 1);
			pushCallStack(s);
			auto ret = boolToCell(s, eval(s, d + 2, true)->type == cell::typeList);
			return popCallStackLeaveData(s, ret, temporary);
		}
		else if (fxName == "car") {
			derrpnil(d->i == 2, "car: expecting 1 argument, passed: ", d->i - 1);

			// list
			auto arg = d + 2;

			// we could return temporary only if arg is not list
			if (temporary && arg->type != cell::typeList) {
				auto l = eval(s, arg, true); // guaranteed not to leave anything on stack
				if (l->type == cell::typeList) {
					if (l->i > 0)
						return l + 1;
					else return pushCell(s, s.c_nil, temporary);
				}
				return l;
			}

			// leave whole list on stack
			pushCallStack(s);
			auto r = eval(s, arg);
			if (r->type == cell::typeList) {
				if (r->i > 0)
					return popCallStackLeaveData(s, r + 1, temporary);

				// we've got list with no elements - discard r and return nil
				return popCallStackLeaveData(s, s.c_nil, temporary);
			}

			// return nil
			return popCallStackLeaveData(s, s.c_nil, temporary);
		}
		else if (fxName == "cdr") {
			derrpnil(d->i == 2, "cdr: expecting 1 argument, passed: ", d->i - 1);

			// list
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
				return popCallStackLeaveData(s, s.c_nil, temporary);
			}
			return popCallStackLeaveData(s, s.c_nil, temporary);
		}
		else if (fxName == "nth") {
			derrpnil(d->i == 3, "nth: expecting 2 argument, passed: ", d->i - 1);

			// calc N
			pushCallStack(s);
			cell_t nc = eval(s, d + 2, true);
			derrppnil(nc->type == cell::typeInt, "nth: argument 1 must be int");
			int n = nc->i;

			// find address
			cell_t nth = eval(s, d + 3);
			derrppnil(nth->type == cell::typeList, "nth: argument 2 must be list");
			derrppnil(n < nth->i, "nth: index out of list bounds");

			// find nth element in list and return result
			return popCallStackLeaveData(s, nthCell(nth, n), temporary);
		}
		else if (fxName == "defun") {
			derrpnil(d->i > 3, "defun: expecting > 2 arguments, passed: ", d->i - 1);
			derrpnil((d + 2)->type == cell::typeIdentifier, "defun: argument 1 must be ID");
			cell_t fnName = d + 2;

			// just put list (lambda) on stack
			cell_t va = pushCell(s, {cell::typeList, d->i - 2}); // without "defun" and <fnName>
			s.stack.insert(s.stack.end(), d + 3, endCell(d)); // copy args and body
			pushVariable(s, fnName->s, va);

			// return function id
			return pushCell(s, *fnName);
		}
		else if (fxName == "length") {
			derrpnil(d->i == 2, "length: expecting 1 argument, passed: ", d->i - 1);
			pushCallStack(s);
			cell_t addr = eval(s, d + 2, true);
			int len = addr->type == cell::typeList ? addr->i : 1;
			popCallStack(s);
			return pushCell(s, {cell::typeInt, len});
		}
		else if (fxName == "push") {
			derrpnil(d->i == 3, "push: expecting 2 arguments, passed: ", d->i - 1);
			pushCallStack(s);

			// eval value and list itself
			cell_t val = eval(s, d + 2, true);
			cell_t lst = eval(s, nextCell(d + 2));

			// insert value at list end and update list count
			auto actionOp = [val](cells_t &c, cell_t lst) {
				cell_t end = endCell(lst);
				lst->i += 1;
				c.insert(end, val, val + countElements(val));
			};
			lst = listOp(s, lst, actionOp);
			return popCallStackLeaveData(s, lst, temporary);
		}
		else if (fxName == "append") {
			derrpnil(d->i == 3, "append: expecting 2 arguments, passed: ", d->i - 1);
			pushCallStack(s);

			// eval source and target lists
			cell_t val = eval(s, d + 2, true);
			cell_t lst = eval(s, nextCell(d + 2));
			size_t elems = countElements(val);

			// merge lists and update count
			auto actionOp = [val, elems](cells_t &c, cell_t lst) {
				cell_t end = endCell(lst);
				lst->i += val->i;
				c.insert(end, val + 1, val + elems);
			};
			lst = listOp(s, lst, actionOp);
			return popCallStackLeaveData(s, lst, temporary);
		}
		else if (fxName == "add-to-list") {
			derrpnil(d->i == 3, "add-to-list: expecting 2 arguments, passed: ", d->i - 1);
			pushCallStack(s);

			// eval list and element to insert
			cell_t val = eval(s, d + 2, true);
			cell_t lst = eval(s, nextCell(d + 2));
			bool doInsert;

			// add element to list (set)
			auto peekOp = [&doInsert, val](cell_t lst) { doInsert = !cellFound(lst, val); };
			auto detachOp = [&doInsert](){ return doInsert; };
			auto actionOp = [&doInsert, val](cells_t &c, cell_t lst) {
				if (doInsert) {
					cell_t end = endCell(lst);
					lst->i += 1;
					c.insert(end, val, val + countElements(val));
				}
			};
			lst = listOp(s, lst, actionOp, detachOp, peekOp);
			return popCallStackLeaveData(s, lst, temporary);
		}
		else if (fxName == "add-to-ordered-list") {
			derrpnil(d->i == 3, "add-to-ordered-list: expecting 2 arguments, passed: ", d->i - 1);
			pushCallStack(s);

			// eval list and element to insert
			cell_t val = eval(s, d + 2, true);
			cell_t lst = eval(s, nextCell(d + 2));
			size_t offset;

			// add element to list (set)
			auto peekOp = [&offset, val](cell_t lst) {
				for (cell_t e = firstCell(lst);; e = nextCell(e)) {
					offset = std::distance(lst, e);
					if (e == endCell(lst) || *e >= *val)
						return;
				}
			};
			auto detachOp = [](){ return true; };
			auto actionOp = [&offset, val](cells_t &c, cell_t lst) {
				cell_t whence = lst + offset;
				lst->i += 1;
				c.insert(whence, val, val + countElements(val));
			};
			lst = listOp(s, lst, actionOp, detachOp, peekOp);
			return popCallStackLeaveData(s, lst, temporary);
		}
		else if (fxName == "setq" || fxName == "set") {
			derrpnil(d->i == 3, fxName, ": expecting 2 arguments, passed: ", d->i - 1);
			pushCallStack(s);

			// gather data
			const bool isSetq = fxName == "setq";
			cell_t var = isSetq ? (d + 2) : eval(s, d + 2, true);
			derrppnil(var->type == cell::typeIdentifier, fxName, ": argument 1 must be ID");
			cell_t val = eval(s, nextCell(d + 2), true);
			size_t targetSize = countElements(val);
			size_t srcSize;

			// define ops & process
			auto peekOp = [&srcSize](cell_t lst) { srcSize = countElements(lst); };
			auto detachOp = [&srcSize, targetSize]() { return targetSize != srcSize; };
			auto actionOp = [&srcSize, targetSize, val](cells_t &c, cell_t lst) {
				remove_copy(c, lst, lst + srcSize, val, val + targetSize);
			};
			var = listOp(s, var, actionOp, detachOp, peekOp);

			// return address and pop all temporary mess
			return popCallStackLeaveData(s, var, temporary);
		}
		else if (fxName == "setcar") {
			derrpnil(d->i == 3, "setcar: expecting 2 arguments, passed: ", d->i - 1);
			derrpnil((d + 2)->type == cell::typeIdentifier, "setcar: argument 1 must be ID");
			pushCallStack(s);

			// gather data
			cell_t var = d + 2;
			cell_t val = eval(s, nextCell(d + 2), true);
			size_t targetSize = countElements(val);
			size_t srcCarSize;

			// define ops & process
			auto peekOp = [&srcCarSize](cell_t lst) { srcCarSize = countElements(lst + 1); };
			auto detachOp = [&srcCarSize, targetSize]() { return targetSize != srcCarSize; };
			auto actionOp = [&srcCarSize, targetSize, val](cells_t &c, cell_t lst) {
				remove_copy(c, lst + 1, lst + 1 + srcCarSize, val, val + targetSize);
			};
			listOp(s, var, actionOp, detachOp, peekOp);

			// return address and pop all temporary mess
			return popCallStackLeaveData(s, val, temporary);
		}
		else if (fxName == "setcdr") {
			derrpnil(d->i == 3, "setcdr: expecting 2 arguments, passed: ", d->i - 1);
			derrpnil((d + 2)->type == cell::typeIdentifier, "setcdr: argument 1 must be ID");
			pushCallStack(s);

			// gather data
			cell_t var = d + 2;
			cell_t val = eval(s, nextCell(d + 2), true);
			size_t targetOffset = val->type == cell::typeList ? 1 : 0;
			size_t targetSize = countElements(val) - targetOffset;
			size_t srcOffset; // list head + car size
			size_t srcSize; // car size

			// define ops & process
			auto peekOp = [&srcOffset, &srcSize](cell_t lst) {
				srcOffset = countElements(lst + 1) + 1;
				srcSize = countElements(lst);
			};
			auto detachOp = [&srcOffset, &srcSize, targetSize]() {
				return targetSize != (srcSize - srcOffset);
			};
			auto actionOp = [&srcOffset, &srcSize, targetSize, targetOffset, val](cells_t &c, cell_t lst) {
				lst->i = 1 + (val->type == cell::typeList ? val->i : 1);
				remove_copy(c, lst + srcOffset, lst + srcSize,
							val + targetOffset, val + targetOffset + targetSize);
			};
			listOp(s, var, actionOp, detachOp, peekOp);

			// return address and pop all temporary mess
			return popCallStackLeaveData(s, val, temporary);
		}
		else if (fxName == "pop") {
			derrpnil(d->i == 2, "pop: expecting 1 argument, passed: ", d->i - 1);
			pushCallStack(s);

			// gather data
			cell_t var = eval(s, d + 2);
			size_t sourceSize = countElements(var);
			cell last;

			// define ops & process
			auto actionOp = [sourceSize, &last](cells_t &c, cell_t lst) {
				cell_t end = lastCell(lst);
				last = *end;
				lst->i = std::max(0, lst->i - 1);
				c.erase(end, end + countElements(end));
			};
			listOp(s, var, actionOp);

			// return address and pop all temporary mess
			return popCallStackLeaveData(s, pushCell(s, last), temporary);
		}
		else if (fxName == "delete" ||
				 fxName == "assoc-delete" ||
				 fxName == "nth-delete") {
			derrpnil(d->i == 3, fxName, ": expecting 2 arguments, passed: ", d->i - 1);
			pushCallStack(s);

			// gather data
			cell_t var = eval(s, nextCell(d + 2));
			cell_t item = eval(s, d + 2, true);
			size_t delIndex;

			// peek op will search for index to delete
			auto peekOp = [&delIndex, item, fxName](cell_t lst) {
				if (fxName == "delete")
					delIndex = std::distance(lst, findCell(lst, item));
				else if (fxName == "assoc-delete")
					delIndex = std::distance(lst, findValue(lst, item));
				else delIndex = std::distance(lst, nthCell(lst, item->i));
			};

			// define ops & process
			auto detachOp = [&delIndex]() { return delIndex > 0; };
			auto actionOp = [&delIndex](cells_t &c, cell_t lst) {
				if (delIndex > 0) {
					cell_t whence = lst + delIndex;
					lst->i = std::max(0, lst->i - 1);
					c.erase(whence, whence + countElements(whence));
				}
			};
			var = listOp(s, var, actionOp, detachOp, peekOp);

			// return address and pop all temporary mess
			return popCallStackLeaveData(s, var, temporary);
		}
		else if (fxName == "while") {
			derrpnil(d->i > 2, "while: expecting > 1 arguments, passed: ", d->i - 1);
			cell_t result = s.c_nil;
			while (true) {
				pushCallStack(s);

				// compute test
				cell_t test = eval(s, d + 2, true);

				// if test is not nil -> eval body, otherwise return last result
				if (!isNil(test))
					result = evalreturn(s, nextCell(d + 2), endCell(d), temporary);
				else return popCallStackLeaveData(s, result, temporary);
				popCallStack(s);
			}

			// will never reach here
			return s.c_nil;
		}
		else if (fxName == "dotimes") {
			derrpnil(d->i > 3, "dotimes: expecting > 2 arguments, passed: ", d->i - 1);
			derrpnil((d + 2)->type == cell::typeIdentifier, "dotimes: argument 1 must be identifier");
			pushCallStack(s);

			// extract name and loop iteration count
			string &name = (d + 2)->s;
			cell_t cntimes = eval(s, d + 3, true);
			derrppnil(cntimes->type == cell::typeInt, "dotimes: argument 2 must be int");
			int ntimes = cntimes->i;

			// create iterator variable
			cell_t it = pushVariable(s, name, 1);
			cell_t result = s.c_nil;

			// update variable value and eval body
			for (int i = 0; i < ntimes; ++i) {
				*it = { cell::typeInt, i };
				result = evalreturn(s, d + 4, endCell(d), temporary);
			}

			// return last eval result or nil
			return popCallStackLeaveData(s, result, temporary);
		}
		else if (fxName == "dolist") {
			derrpnil(d->i > 3, "dolist: expecting > 2 arguments, passed: ", d->i - 1);
			derrpnil((d + 2)->type == cell::typeIdentifier, "dolist: argument 1 must be ID");
			pushCallStack(s);

			// extract iterator name, and eval list
			string &name = (d + 2)->s;
			cell_t lst = eval(s, d + 3); // must copy to stack, we are binding variable to that addr
			derrppnil(lst->type == cell::typeList, "dolist: argument 2 must be list");

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
				result = evalreturn(s, nextCell(d + 3), endBody, temporary);
			}

			// return last eval or nil
			return popCallStackLeaveData(s, result, temporary);
		}
		else if (fxName == "cond") {
			derrpnil(d->i > 1, "cond: expecting > 0 arguments, passed: ", d->i - 1);
			pushCallStack(s);

			// iterate through all clauses
			for (cell_t c = d + 2; c != endCell(d); c = nextCell(c)) {
				derrppnil(c->type == cell::typeList && c->i > 1, "cond: invalid condition clause, should be <cond><body...>");

				// check condition - if true return evaluated result
				if (!isNil(eval(s, c + 1, true))) {
					return popCallStackLeaveData(
						s, evalreturn(s, nextCell(c + 1), endCell(c)));
				}
			}

			// no condition yielded true -> return nil
			return popCallStackLeaveData(s, s.c_nil, temporary);
		}
		else if (fxName == "or") {
			derrpnil(d->i > 2, "or: expecting > 2 arguments, passed", d->i - 1);

			// eval all until non nil, then return
			pushCallStack(s);
			for (cell_t c = d + 2; c != endCell(d); c = nextCell(c)) {
				cell_t r = eval(s, c, true);
				if (!isNil(r))
					return popCallStackLeaveData(s, r, temporary);
			}

			// all nil - return nil
			return popCallStackLeaveData(s, s.c_nil, temporary);
		}
		else if (fxName == "and") {
			derrpnil(d->i > 2, "and: expecting > 2 arguments, passed", d->i - 1);

			// eval all until nil, then return
			pushCallStack(s);
			cell_t r = s.c_nil;
			for (cell_t c = d + 2; c != endCell(d); c = nextCell(c)) {
				r = eval(s, c, true);
				if (isNil(r))
					return popCallStackLeaveData(s, r, temporary);
			}

			// all t - return last
			return popCallStackLeaveData(s, r, temporary);
		}
		else if (fxName == "!=") {
			derrpnil(d->i == 3, "!=: expecting 2 arguments, passed: ", d->i - 1);
			pushCallStack(s);

			// checks if all evaluated cells are not equal
			// does not support multiple values
			cell_t result = cellsEqual(eval(s, d + 2, true),
									   eval(s, nextCell(d + 2), true)) ?
				s.c_nil : s.c_t;
			return popCallStackLeaveData(s, result, temporary);
		}
		else if (fxName == "=" ||
				 fxName == "<" ||
				 fxName == ">" ||
				 fxName == "<=" ||
				 fxName == ">=") {
			derrpnil(d->i > 2, fxName, ": expecting > 1 arguments, passed: ", d->i - 1);

			// determine comparing function
			std::function<bool(cell_t, cell_t)> fx;
			if (fxName == "=") fx = [](cell_t n, cell_t n1) { return cellsEqual(n, n1); };
			else if (fxName == "<") fx = [](cell_t n, cell_t n1) { return *n < *n1; };
			else if (fxName == ">") fx = [](cell_t n, cell_t n1) { return *n > *n1; };
			else if (fxName == "<=") fx = [](cell_t n, cell_t n1) { return *n <= *n1; };
			else fx = [](cell_t n, cell_t n1) { return *n >= *n1; };

			// perform comparsion and return result
			pushCallStack(s);
			cell_t result = evalUntilBinary(s, d + 2, endCell(d), fx, true) ? s.c_t : s.c_nil;
			return popCallStackLeaveData(s, result, temporary);
		}
		else if (fxName == "assoc") {
			derrpnil(d->i == 3, "assoc: expecting 2 arguments, passed: ", d->i - 1);
			pushCallStack(s);

			// get key and list address
			cell_t key = eval(s, d + 2, true);
			cell_t lst = eval(s, nextCell(d + 2), true);
			derrppnil(lst->type == cell::typeList, "assoc: argument 2 must be list");

			// search for first found key in list
			cell_t val = findValue(lst, key);
			if (lst != val)
				return popCallStackLeaveData(s, val, temporary);

			// or return nil if not found
			return popCallStackLeaveData(s, s.c_nil, temporary);
		}
		else if (fxName == "mapcar") {
			derrpnil(d->i == 3, "mapcar: expecting 2 arguments, passed: ", d->i - 1);
			pushCallStack(s);

			// evaluate map function and list
			cell_t fx = eval(s, d + 2, true);
			cell_t lst = eval(s, nextCell(d + 2), true);
			derrppnil(lst->type == cell::typeList, "mapcar: argument 2 must be list");

			// initialize return list (size is equal to lst size)
			cell_t res = pushCell(s, cell(cell::typeList, lst->i));

			// build code to run map function
			cell_t ev = pushCell(s, cell(cell::typeList, 2));
			pushList(s, fx);
			pushCell(s, cell(cell::typeList, 2));
			pushCell(s, cell(cell::typeIdentifier, "quote"));

			// mapping each cell
			for (cell_t el = lst + 1; el != endCell(lst); el = nextCell(el)) {
				// push cell
				cell_t argBegin = pushList(s, el);

				// eval result
				cell_t resultBegin = eval(s, ev);

				// move data to beginning and (rotate with swap function)
				// TODO: note that GCC's version of std::rotate is not C++11 compilant -
				// it does not return offset pointer :| will be fixed in GCC 5.0
				size_t resultSize = std::distance(resultBegin, s.stack.end());
				s.stack.erase(argBegin, resultBegin);
				std::rotate(ev, argBegin, s.stack.end());
				ev += resultSize;
			}
			return popCallStackLeaveData(s, res);
		}
		else if (fxName == "eval") {
			derrpnil(d->i == 2, "eval: expecting 1 argument, passed: ", d->i - 1);
			return eval(s, eval(s, d + 2, temporary), temporary);
		}
		else if (fxName == "member") {
			derrpnil(d->i == 3, "member: expecting 2 arguments, passed: ", d->i - 1);
			pushCallStack(s);

			// eval object to find and list itself
			cell_t obj = eval(s, d + 2, true);
			cell_t lst = eval(s, nextCell(d + 2), true);
			derrppnil(lst->type == cell::typeList, "member: argument 2 must be list");

			// iterate through list and return t if object found
			return popCallStackLeaveData(s, boolToCell(s, cellFound(lst, obj)), temporary);
		}
		else if (fxName == "strs") {
			derrpnil(d->i > 1, "strs: expecting > 0 arguments, passed: ", d->i - 1);

			// result string
			string res;

			// convert to strings and concatenate elements
			pushCallStack(s);
			evalmap(s, d + 2, endCell(d),
					[&res](cell_t el) {
						res += el->getStr();
					}, true);

			// push return on stack
			return popCallStackLeaveData(s, pushCell(s, cell(cell::typeString, res)));
		}
		else if (fxName == "strf") {
			derrpnil(d->i > 2, "strf: expected > 1 arguments, passed: ", d->i - 1);

			// number of arguments
			int nArgs = d->i - 2;

			// evaluate arguments (leave them on stack)
			pushCallStack(s);
			const string &format = eval(s, d + 2, true)->s;
			cell_t args = s.stack.end();
			evalNoStack(s, nextCell(d + 2), endCell(d));

			// pass arguemnts to strf
			#define FA(NUM) (args + NUM)->getStr()
			string res;
			switch (nArgs) {
				case 1: res = strf(format, FA(0)); break;
				case 2: res = strf(format, FA(0), FA(1)); break;
				case 3: res = strf(format, FA(0), FA(1), FA(2)); break;
				case 4: res = strf(format, FA(0), FA(1), FA(2), FA(3)); break;
				case 5: res = strf(format, FA(0), FA(1), FA(2), FA(3), FA(4)); break;
				case 6: res = strf(format, FA(0), FA(1), FA(2), FA(3), FA(4), FA(5)); break;
				case 7: res = strf(format, FA(0), FA(1), FA(2), FA(3), FA(4), FA(5), FA(6)); break;
				default: res = "";
			}
			#undef FA

			// return result on stack
			return popCallStackLeaveData(s, pushCell(s, {cell::typeString, res}));
		}
		else if (fxName == "sort" ||
				 fxName == "reverse") {
			derrpnil(d->i == 2, fxName, ": expected 1 argument, passed: ", d->i - 1);

			// all elements must be atoms
			pushCallStack(s);

			// eval list
			cell_t lst = eval(s, d + 2);

			// list is ID - we must fetch variable
			if (lst->type == cell::typeIdentifier)
				lst = eval(s, lst, true);
			else if (lst->type != cell::typeList)
				// not supported type
				return popCallStackLeaveData(s, pushCell(s, s.c_nil, temporary), temporary);

			// perform sort and return sorted / reversed list
			derrppnil(cellsType(firstCell(lst), endCell(lst), firstCell(lst)->type), fxName, ": all list elements must have the same type");
			if (fxName == "sort")
				std::sort(lst + 1, endCell(lst));
			else std::reverse(lst + 1, endCell(lst));
			return popCallStackLeaveData(s, lst, temporary);
		}
		else if (fxName == "print-state") {
			return pushCell(s, {getState(s)});
		}

		//- functions evaluation -
		// get fx address
		var_t var = findVariable(s, fxName);
		if (isVariableValid(s, var)) {
			cell_t fx = getVariableAddress(s, var);

			// error check
			derrpnil(fx->type == cell::typeList, "evaluating function \"", fxName, "\": not a function: ", toString(fx));
			derrpnil(fx->i > 0, "evaluating empty function, data corrupted: ", toString(fx));

			// [list:][list:]<arg><arg>[list:]<body><body>[list:]<body>...
			// function stack frame
			pushCallStack(s);

			// evaluate and bind args
			bindArguments(s, fx + 1, d + 2);

			// evaluate body
			cell_t body = nextCell(fx + 1);
			return popCallStackLeaveData(s, evalreturn(s, body, endCell(fx), temporary), temporary);
		}
		else {
			proc_t i = getProcedure(s, fxName);
			if (isProcedureValid(s, i)) {
				pushCallStack(s);

				// arguments list address
				cell_t r = s.stack.end();

				// evaluate arguments, if any (leave result on stack)
				pushCell(s, cell(cell::typeList, d->i - 1));// list elements count (not counting name)
				if (d->i > 1)
					evalNoStack(s, d + 2, endCell(d));

				// call procedure
				return popCallStackLeaveData(s, std::get<1>(*i)(r, s.stack), temporary);
			}
			derr(false, "procedure / function \"", fxName, "\" not found");
		}
	}

	derr(false, "logic error: return nil");
	return pushCell(s, s.c_nil, temporary);
}

void signalError(lispState &s, const string &description) {
	derr(false, description);
}

} // detail end


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

cells_t lisp::parse(const string &s) {
	ddebnt("");
	auto code = detail::parse(s);
	ddebnt("");
	return code;
}

string lisp::eval(cells_t &code) {
	if (code.size() > 0) {
		string r = "";
		for (cell_t statement = code.begin(); statement < code.end(); statement = nextCell(statement)) {
			#ifdef GLISP_DEBUG_ERROR_ARRAY
			_s->errors.clear();
			#endif

			auto retAddr = detail::eval(*_s, statement, true);

			#ifdef GLISP_DEBUG_ERROR_ARRAY
			for (const auto &e : getError())
				std::cout << e << std::endl;
			#endif

			if (r != "")
				r += "\n";
			r += toString(retAddr);
			ddebnt("return addr: ", detail::getAddress(*_s, retAddr),
				   " | ", toString(retAddr));
			ddebnt(detail::getState(*_s));
			ddebnt("sweep...");
			detail::sweepStack(*_s);
			ddebnt(detail::getState(*_s));
		}
		return r;
	}
	return "";
}

string lisp::eval(const string &s) {
	cells_t code = parse(s);
	return eval(code);
}

string lisp::eval(cells_t &&code) {
	return eval(code);
}

void lisp::signalError(const string &description) {
	detail::signalError(*_s, description);
}

const std::vector<string> &lisp::getError() {
	#ifdef GLISP_DEBUG_ERROR_ARRAY
	return _s->errors;
	#else
	static std::vector<string> t;
	return t;
	#endif
}

void lisp::addProcedure(const string &name, procedure_t fx) {
	detail::addProcedure(*_s, name, fx);
}

void lisp::addVariable(const string &name, cell value) {
	*detail::pushVariable(*_s, name, 1) = value;
}

}}
