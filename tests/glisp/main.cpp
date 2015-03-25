#define REPL

#include <base/base.h>

using namespace granite;

bool test(base::lisp &gl, const string &input, const string &expectedOutput) {
	string result = gl.eval(input);
	if (result == expectedOutput) {
		std::cout << "[ok] " << input << " = " << expectedOutput << std::endl;
		return true;
	}

	std::cout << "[fail] " << input << " = " << result << std::endl;
	return false;
}

// custom (user) procedures
base::cell_t c_message(base::cell_t c, base::cells_t &ret) {
	// *c - count
	// *(c + x) - element x
	std::cout << "> message: " << (c + 1)->i << std::endl;
	return c + 1;
}

base::cell_t c_mul(base::cell_t c, base::cells_t &ret) {
	int r = 1;
	for(base::cell_t i = c + 1; i != c + 1 + c->i; ++i) {
		r *= i->i;
	}
	ret.push_back(base::cell(base::cell::typeInt, r));
	return ret.begin() + ret.size() - 1;
}

int main(int argc, char **argv) {
	granite::base::lisp gl;
	gl.init();

	// add custom procedures
	gl.addProcedure("**", &c_mul);
	gl.addProcedure("message", &c_message);

	#ifdef REPL
	while (true) {
		string inp;
		std::getline(std::cin, inp);
		gl.eval(inp);
	}
	#else
	test(gl, "(defvar my-test-x 55)", "my-test-x");
	test(gl, "my-test-x", "55");
	test(gl, "(defun my-test-l () (setq my-test-x 66))", "my-test-l");
	test(gl, "(my-test-l)", "66");
	test(gl, "my-test-x", "66");
	test(gl, "(defun y () (defvar x 6))", "y");
	test(gl, "(y)", "x");
	test(gl, "(unbound 'y)", "y");
	test(gl, "1", "1");
	test(gl, "()", "nil");
	test(gl, "(= 44 66)", "nil");
	test(gl, "(= 66 66)", "t");
	test(gl, "(defvar l (lambda (a bc ) (+ a bc (** 2 bc) 1000)))", "l");
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
	test(gl, "(defvar ll '(a b c))", "ll");
	test(gl, "(push 'asdf ll)", "(a b c asdf)");
	test(gl, "ll", "(a b c asdf)");
	test(gl, "(pop ll)", "asdf");
	test(gl, "ll", "(a b c)");
	test(gl, "(append ll '(1 2 3 4))", "(a b c 1 2 3 4)");
	test(gl, "(setcar ll 666)", "666");
	test(gl, "ll", "(666 b c 1 2 3 4)");
	test(gl, "(setcar ll '(x y z))", "(x y z)");
	test(gl, "ll", "((x y z) b c 1 2 3 4)");
	test(gl, "(setcdr ll '(c d '(r r r))", "(c d (quote (r r r)))");
	test(gl, "ll", "((x y z) c d (quote (r r r)))");
	test(gl, "(defvar ll1 '(1 2 3))", "ll1");
	test(gl, "(setcdr ll1 '(b c))", "(b c)");
	test(gl, "ll1", "(1 b c)");
	test(gl, "(defvar ll2 '(1 2))", "ll2");
	test(gl, "(setcdr ll2 '(c))", "(c)");
	test(gl, "ll2", "(1 c)");
	test(gl, "(setcdr ll2 66)", "66");
	test(gl, "ll2", "(1 66)");
	test(gl, "(setcdr ll2 '(ugly parabola wolfram))", "(ugly parabola wolfram)");
	test(gl, "ll2", "(1 ugly parabola wolfram)");
	test(gl, "(setcdr ll2 '(ugly parabola))", "(ugly parabola)");
	test(gl, "ll2", "(1 ugly parabola)");
	test(gl, "(defvar ll3 '(1 2))", "ll3");
	test(gl, "(setcar ll3 '(1 2))", "(1 2)");
	test(gl, "ll3", "((1 2) 2)");
	test(gl, "(let ((i 0) (k 55)) (= i k))", "nil");
	test(gl, "(let ((i 55) (k 55)) (= i k))", "t");
	test(gl, "(let ((i 0) (k 55)) (!= i k))", "t");
	test(gl, "(let ((i 55) (k 55)) (!= i k))", "nil");
	test(gl, "(let ((i 0)) (while (!= 7 i) (setq i (+ i 1)) (message i) i))", "7");
	test(gl, "(dotimes i 5 (message i)", "4");
	test(gl, "(dolist e '(11 22 33) (message e))", "33");
	test(gl, "(cond (t 123)(nil 555))", "123");
	test(gl, "(cond (nil 123)(t 555))", "555");
	test(gl, "(cond (nil 123)(nil 555))", "nil");
	test(gl, "(cond ((= 5 (+ 2 3)) 1235)(nil 555))", "1235");
	test(gl, "(defvar trees '((pine cones) (oak acorns) (maple seeds)))", "trees");
	test(gl, "(assoc 'oak trees)", "(oak acorns)");
	test(gl, "(defun mycar (el) (car el))", "mycar");
	test(gl, "(mapcar 'car '((a b) (c d) (e f)))", "(a c e)");
	test(gl, "(mapcar 'mycar '((a b) (c d) (e f)))", "(a c e)");
	test(gl, "(eval 'trees)", "((pine cones) (oak acorns) (maple seeds))");
	test(gl, "(eval (eval ''trees))", "((pine cones) (oak acorns) (maple seeds))");
	test(gl, "(member '(maple seeds) trees)", "t");
	test(gl, "(member '(maple seeds nasty-thing) trees)", "nil");
	test(gl, "(member 'nil '(1 2 3 nil b))", "t");
	test(gl, "( member '       nil   '(1 2 3   nil- b )   )", "nil");
	test(gl, "(defvar t-str \"-some test string \\\"\")", "t-str");
	test(gl, "(defvar t-int 678)", "t-int");
	test(gl, "(strs \"-\" 666 \"-\")", "\"-666-\"");
	test(gl, "(strs \"-\" t-int t-str \"-\")", "\"-678-some test string \\\"-\"");
	test(gl, "(strf \"int test: %, %\" t-int \"string test\")", "\"int test: 678, string test\"");
	test(gl, "(/ 1.0 2.0 2.0)", "0.250000");
	test(gl, "(- 6 (* 2 3 ))", "0");
	test(gl, "|1 2 3 4|", "1.000000 2.000000 3.000000 4.000000");
	#endif

	gl.close();
	return 0;
}
