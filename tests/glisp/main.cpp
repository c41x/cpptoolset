#include <base/base.h>

int main(int argc, char**argv) {
	granite::base::lisp gl;
	gl.init();

	while (true) {
		string inp;
		std::getline(std::cin, inp);
		gl.eval(inp);
	}

	gl.close();
	return 0;
}

// (defvar l (lambda (a bc ) (+ a bc (* 2 bc) 1000)))
// (if t (progn (message 11) (message 22)) (message 33))
// (let ((a 10) (b 20)) (+ a a) (+ b b))
// (let ((a '(a b c))) a)
// let* test: (let ((a 10) (b (* a 20))) b)
// (car '(aa bb cc))
