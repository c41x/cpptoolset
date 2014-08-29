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

	return 0;
}
