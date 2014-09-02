#include <base/base.h>

using namespace granite;
using namespace granite::base;

void repl() {
	scope s;
	initInterpreter(&s);
	while(true) {
		std::cout << "> ";
		string line;
		std::getline(std::cin, line);
		std::cout << toString(eval(parse(line), &s)) << std::endl;
	}
}

int main(int argc, char**argv){
	heap<string> hp(10);
	hp.push("abc");
	hp.push("abcd");
	hp.push("123");
	hp.push("666");
	hp.push("this one is longer");
	hp.pop(2);
	hp.push("this one is even longer");
	hp.push("here we go numbers!");
	hp.push("3.14");
	hp.push("6.28");
	hp.push("2.71");
	// TODO: hp.push("---------");
	hp.print();

	//repl();
	return 0;
}
