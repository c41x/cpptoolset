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
	std::cout << "return addr: " << getAddress(retAddr) << std::endl;
	std::cout << printStack() << std::endl;
}

int main(int argc, char**argv) {
	init(100000);
	while(true) {
		repl();
	}
	return 0;
}
// (defvar l (lambda (a bc ) (+ a bc (* 2 bc) 1000)))
// (if t (progn (message 11) (message 22)) (message 33))
// (let ((a 10) (b 20)) (+ a a) (+ b b))
// let* test: (let ((a 10) (b (* a 20))) b)
