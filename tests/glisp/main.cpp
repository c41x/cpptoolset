//#define REPL

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
	test(gl, "(defvar l (lambda (a bc ) (+ a bc (* 2 bc) 1000)))", "l");
	test(gl, "(l 2 3)", "1011");
	test(gl, "(progn 1 2 3)", "3");
	test(gl, "(if t (progn 11 22) 33)", "22");
	test(gl, "(if nil (progn 11 22) 33)", "33");
	test(gl, "(let ((a 10) (b 20)) (+ a a) (+ b b))", "40");
	test(gl, "(let ((a '(a b c))) a)", "(a b c)");
	test(gl, "(let ((a 10) (b (* a 20))) b)", "200");
	test(gl, "(car '(aa bb cc))", "aa");
	test(gl, "(cdr '(aa bb cc))", "(bb cc)");
	test(gl, "(nth 1 '(aa bb cc))", "bb");
	test(gl, "(defvar xx 456)", "xx");
	test(gl, "(unbound 'l)", "l");
	test(gl, "(quote (asd sdf dfg fgh))", "(asd sdf dfg fgh)");
	test(gl, "(progn (progn 1 2 3 'asd) 44)", "44");
	test(gl, "(defvar x 123)", "x");
	test(gl, "x", "123");
	test(gl, "(setq x '(1 2 3))", "(1 2 3)");
	test(gl, "x", "(1 2 3)");
	test(gl, "(setq x 666)", "666");
	test(gl, "(setq x (progn (defvar zyx 777) zyx))", "777");
	test(gl, "(progn (defvar zyx 777) zyx)", "777");
	test(gl, "(set (progn 123 33 (defvar its-x 'x) its-x) 1234567)", "1234567");

	#endif

	gl.close();
	return 0;
}
