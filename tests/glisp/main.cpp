#include <base/base.h>

bool test(granite::base::lisp &gl, const string &input, const string &expectedOutput) {
	string result = gl.eval(input);
	if (result == expectedOutput) {
		std::cout << "[ok] " << input << " = " << expectedOutput << std::endl;
		return true;
	}

	std::cout << "[fail] " << input << " = " << result << std::endl;
	return false;
}

//#define REPL

int main(int argc, char **argv) {
	granite::base::lisp gl;
	gl.init();

	#ifdef REPL
	while (true) {
		string inp;
		std::getline(std::cin, inp);
		gl.eval(inp);
	}
	#else
	test(gl, "1", "1");
	test(gl, "()", "nil");
	test(gl, "(= 44 66)", "nil");
	test(gl, "(= 66 66)", "t");
	#endif

	gl.close();
	return 0;
}

// (defvar l (lambda (a bc ) (+ a bc (* 2 bc) 1000)))
// (if t (progn (message 11) (message 22)) (message 33))
// (let ((a 10) (b 20)) (+ a a) (+ b b))
// (let ((a '(a b c))) a)
// let* test: (let ((a 10) (b (* a 20))) b)
// (car '(aa bb cc))
