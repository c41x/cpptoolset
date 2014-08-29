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
	repl();
	return 0;
}
