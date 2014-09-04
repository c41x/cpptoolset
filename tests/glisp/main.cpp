#include <base/base.h>

using namespace granite;
using namespace granite::base;

void repl() {
	initInterpreter();
	eval(parse("(defun addd (a b c) (+ a a b b c c))"));
	while(true) {
		std::cout << "> ";
		string line;
		std::getline(std::cin, line);
		std::cout << toString(eval(parse(line))) << std::endl;
	}
}

int main(int argc, char**argv){
	repl();
	return 0;
}
