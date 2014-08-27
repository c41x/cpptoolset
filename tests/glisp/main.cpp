#include <base/base.h>

using namespace granite;
using namespace granite::base;

const char *in =
	";--------------------------------------------------------------------------------------------------\n"
	"; CMake project utils																				\n"
	"(defun upward-check-file (filename startdir)														\n"
	"  \"Moves up in directory structure and checksif desired file is there\" 						\n"
	"  (let ((dirname (expand-file-name startdir))														\n"
	" 	(not-found nil)																					\n"
	" 	(top nil)																						\n"
	" 	(max-level 5)																					\n"
	" 	(prv-dirname nil))																				\n"
	" 																									\n"
	"    (while (not (or not-found top (= max-level 0)))												\n"
	"      (setq max-level (- max-level 1))																\n"
	"      (if (string= (expand-file-name dirname) \"/\")												\n"
	" 	  (setq top t))																					\n"
	"      (if (file-exists-p (expand-file-name filename dirname))										\n"
	" 	  (progn																						\n"
	" 	    (setq prv-dirname dirname)																	\n"
	" 	    (setq dirname (expand-file-name \"..\" dirname)))											\n"
	" 	(setq not-found t)))																			\n"
	" 																									\n"
	"    prv-dirname))																					\n"
	" \"\""
	;

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

void repl() {
	scope s;
	initInterpreter(&s);
	while(true)
	{
		std::cout << "> ";
		string line;
		std::getline(std::cin, line);
		std::cout << toString(eval(parse(line), &s)) << std::endl;
	}
}

int main(int argc, char**argv){
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
	const char *tokenTypeStr[] = {
		"symbol",
		"whitespace",
		"open par",
		"closing par",
		"comment",
		"string",
		"operator",
		"symbolic operator"
	};
	tokenizer tok;
	tok.addRule(tokenWhiteSpace, true, " \t\v\n\r");
	tok.addRule(tokenOpenPar, false, "(");
	tok.addRule(tokenClosePar, false, ")");
	tok.addRule(tokenComment, true, ";", "", "\n\r");
	tok.addRule(tokenString, false, "\"", "", "\"");

	std::cout << "tokenize some ELISP:" << std::endl;
	for(token t = tok.begin(in, true); t; t = tok.next()) {
		std::cout << tokenTypeStr[std::max(0, t.id + 1)] << " => " << t.value.str() << std::endl;
	}

	repl();

	return 0;
}
