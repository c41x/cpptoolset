#include <base/base.h>

const char *in =
	";--------------------------------------------------------------------------------------------------\n" 
	"; CMake project utils																				\n" 
	"(defun upward-check-file (filename startdir)														\n" 
	"  \"Moves up in directory structure and checks \\\"if desired file is there\" 						\n" 
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

const char *in2 =
	"// extract tokens                                                                      \n" 
	"bool inToken = false;																	\n" 
	"string::const_iterator tokenBegin = in.cbegin(), next;									\n" 
	"std::vector<rule>::const_iterator currentRule;											\n" 
	"for(auto it = in.cbegin(); it != in.cend(); ++it) {									\n" 
	" 	next = it + 1;																		\n" 
	" 	// find rule if we are not in token													\n" 
	" 	if(!inToken) {																		\n" 
	" 		currentRule = findRule(it);														\n" 
	" 																						\n" 
	" 		if(currentRule != _rules.cend() || (it + 1) == in.cend()) {						\n" 
	" 			if(tokenBegin != it) {														\n" 
	" 				tokens.push_back({-1, stringRange(tokenBegin, it)});					\n" 
	" 			}																			\n" 
	" 																						\n" 
	" 			tokenBegin = it;															\n" 
	" 			inToken = true;																\n" 
	" 			++it;																		\n" 
	" 		}																				\n" 
	" 	}																					\n"
	;

class cell {
public:
	enum type_t {
		typeIdentifier = 0,
		typeString = 1,
		typeInt = 1 << 1,
		typeFloat = 1 << 2,
		typeFunction = 1 << 3,
		typeList = 1 << 4
	};

	typedef struct {
		size_t car;
		size_t cdr;
	} cons_t;
	
	cons_t cons;
	type_t type;
	std::vector<cell> cdr;
	
	typedef union {
		string str;
		string id;
		float f;
		int i;
	} data;
};

class scope {
	std::vector<cell> cells;
public:
	scope(const std::vector<cell> &args, const std::vector<cell> &body, scope *iparent) {
		parent = iparent;
	}
};

int main(int argc, char**argv){
	using namespace granite;
	using namespace granite::base;

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
	tok.addRule(tokenString, false, "\"", "\\", "\"");
	tok.tokenize(in, true);

	std::cout << "tokenize some ELISP:" << std::endl;
	for(auto t : tok.tokens) {
		std::cout << tokenTypeStr[t.id + 1] << " => " << t.value.str() << std::endl;
	}

	tok.clear();
	tok.addRule(tokenWhiteSpace, true, " \t\v\n\r");
	tok.addRule(tokenOperator, false, "|&+-,=");
	tok.addRule(tokenSymbolicOperator, false, "(){};<>");
	tok.addRule(tokenComment, false, "//", "", "\n\r");
	tok.addRule(tokenString, false, "\"", "\\", "\"");
	tok.tokenize(in2, true);
	
	std::cout << std::endl << "tokenize some C++:" << std::endl;
	for(auto t : tok.tokens) {
		std::cout << tokenTypeStr[t.id + 1] << " => " << t.value.str() << std::endl;
	}

	return 0;
}
