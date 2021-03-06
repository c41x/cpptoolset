//#define REPL

#include <base/base.hpp>

using namespace granite;

using std::string;
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
    if (c->i > 0) {
        std::cout << "> message: " << (c + 1)->getStr() << std::endl;
        return c + 1;
    }
    return c;
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

    // using namespace granite::base;
    // std::cout << lisp::validateStr(cell::typeString, cell::typeList, cell(1), cell::anyOf(cell::typeString, cell::typeIdentifier), cell::listRange(5), cell::listRange(1, 5), cell::list(555)) << std::endl;

    // granite::base::cells_t a;
    // a.push_back(granite::base::cell::list(4));
    // a.push_back(granite::base::cell(64));
    // //a.push_back(granite::base::cell(3.1415f));
    // a.push_back(granite::base::cell(3));
    // a.push_back(granite::base::cell::quote);
    // a.push_back(granite::base::cell(32));
    // if (granite::base::lisp::validate(a.begin(), granite::base::cell::list(4), granite::base::cell::typeInt)) {
    //  std::cout << "valid!" << std::endl;
    // }
    // else std::cout << "not valid! " << std::endl;
    // return 0;

    // add custom procedures
    gl.addProcedure("**", &c_mul);
    gl.addProcedure("message", &c_message);


    // using namespace granite::base;
    // fs::open("d:/dir");
    // //cells_t program = lisp::parse(fromStream<string>(fs::load("code.gls")));
    // //fs::store("code.glsc", toStream(program));
    // //gl.eval(program);

    // gl.eval(fromStream<cells_t>(fs::load("code.glsc")));

    // fs::close();

    // gl.close();
    // return 0;

    #ifdef REPL
    while (true) {
        string inp;
        std::getline(std::cin, inp);
        std::cout << gl.eval(inp) << std::endl;
    }
    #else
    test(gl, "", "");
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
    test(gl, "(push 'asdf 'll)", "(a b c asdf)");
    test(gl, "(push '(x y) '(a b c))", "(a b c (x y))");
    test(gl, "ll", "(a b c asdf)");
    test(gl, "(append '(1 2 3 4) 'll)", "(a b c asdf 1 2 3 4)");
    test(gl, "(append '(asdf) 'll)", "(a b c asdf 1 2 3 4 asdf)");
    test(gl, "(append '(1 2 3 4) '(first list))", "(first list 1 2 3 4)");
    test(gl, "(pop 'll)", "asdf");
    test(gl, "(pop '(will pop this))", "this");
    test(gl, "(setq ll '(a b c))", "(a b c)");
    test(gl, "ll", "(a b c)");
    test(gl, "(append '(1 2 3 4) 'll)", "(a b c 1 2 3 4)");
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
    test(gl, "(defvar x '(12 223 -31415 24 100 0 3 123 33))", "x");
    test(gl, "x", "(12 223 -31415 24 100 0 3 123 33)");
    test(gl, "(sort 'x)", "(-31415 0 3 12 24 33 100 123 223)");
    test(gl, "(sort '(-31415 0 2 3))", "(-31415 0 2 3)");
    test(gl, "(reverse '(-31415 0 2 3))", "(3 2 0 -31415)");
    test(gl, "(sort '(\"cohobator\" \"reamalgamate\" \"disbarring\" \"weathertight\" \"vad\"))", "(\"cohobator\" \"disbarring\" \"reamalgamate\" \"vad\" \"weathertight\")");
    test(gl, "(> 5 4 -1)", "t");
    test(gl, "(> 5 5 4 -1)", "nil");
    test(gl, "(>= 5 5 4 -1)", "t");
    test(gl, "(> 5 4 -1 55)", "nil");
    test(gl, "(< 1 2 3)", "t");
    test(gl, "(< 1 1 2 3)", "nil");
    test(gl, "(<= 1 1 2 3)", "t");
    test(gl, "(< 1 2 3 1)", "nil");
    test(gl, "(add-to-list '(my tree) 'trees)", "((pine cones) (oak acorns) (maple seeds) (my tree))");
    test(gl, "(add-to-list '(oak acorns) 'trees)", "((pine cones) (oak acorns) (maple seeds) (my tree))");
    test(gl, "(delete '(my tree) 'trees)", "((pine cones) (oak acorns) (maple seeds))");
    test(gl, "(delete '(my tree) 'trees)", "((pine cones) (oak acorns) (maple seeds))");
    test(gl, "(assoc-delete 'pine 'trees)", "((oak acorns) (maple seeds))");
    test(gl, "(nth-delete '1 'trees)", "((oak acorns))");
    test(gl, "(nth-delete '0 'trees)", "()");
    test(gl, "(delete '5 '(8 73 3 2 5 -44))", "(8 73 3 2 -44)");
    test(gl, "(defvar ai '(-12 -1 0 1 33 44 67 77 666))", "ai");
    test(gl, "(add-to-ordered-list '2 'ai)", "(-12 -1 0 1 2 33 44 67 77 666)");
    test(gl, "(add-to-ordered-list '-2 '(1 2 4))", "(-2 1 2 4)");
    test(gl, "(add-to-ordered-list '2 '(1 2 4))", "(1 2 2 4)");
    test(gl, "(add-to-ordered-list '88 '(1 2 4))", "(1 2 4 88)");
    test(gl, "(progn (defvar x '(1 2 3)) (push '6 'x))", "(1 2 3 6)");
    test(gl, "(progn (defvar x '(1 2 3)) (push '6 'x))", "(1 2 3 6)");
    test(gl, "(progn (defvar xa '(1 2 3)) (push '6 'xa))", "(1 2 3 6)");
    test(gl, "(or (= 33 (+ 3 5 5)) nil 444 555 666)", "444");
    test(gl, "(or 4 5)", "4");
    test(gl, "(or nil nil nil)", "nil");
    test(gl, "(and 1 2 3)", "3");
    test(gl, "(and nil nil)", "nil");
    test(gl, "(and (= 1 (- 2 1)) t)", "t");
    test(gl, "(list (list))", "(nil)");
    test(gl, "((lambda (a b c) (+ a b c)) 2 3 4)", "9");
    test(gl, "(not t)", "nil");
    test(gl, "(not nil)", "t");
    test(gl, "(not (and 1 2 3))", "nil");
    test(gl, "(not (member '(maple seeds666) trees))", "t");
    std::cout << gl.eval("(print-state)");
    #endif

    gl.close();
    return 0;
}
