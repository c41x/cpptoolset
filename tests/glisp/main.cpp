#include <base/base.h>

using namespace granite;
using namespace granite::base;

void repl() {
	//eval(parse("(defun addd (a b c) (+ a a b b c c))"));
	//eval(parse("(defvar addd (lambda (a b c) (+ a a b b c c)))"));

	//std::cout << toString(parse("(defvar addd (lambda (a b c) (+ a a b b c c))) (1 2 3)")) << std::endl;
	//std::cout << toString(parse("(defun addd (a b c) (+ a a b b c c))")) << std::endl;
	string inp;
	std::getline(std::cin, inp);
	auto code = parse(inp);
	std::cout << toString(code) << std::endl;
	auto retAddr = eval(code.begin());
	std::cout << "return addr: " << addrd(retAddr) << std::endl;
	std::cout << printStack() << std::endl;
}

int main(int argc, char**argv) {
	while(true) {
		repl();
	}
	return 0;
}
