#include <base/base.h>

using namespace granite;
using namespace granite::base;

int main(int argc, char **argv) {
	std::cout << "\"" << cpu::toStr() << "\"" << std::endl;
	cpu::cycleTimer ct;
	ct.reset();
	uint64 e = ct.elapsed();
	std::cout << "cycles count: " << e << std::endl;
	ct.reset();
	for (volatile int i = 0; i < 9999; ++i);
	e = ct.elapsed();
	std::cout << "cycles count: " << e << std::endl;
	return 0;
}
