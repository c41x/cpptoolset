#include <base/base.hpp>

using namespace granite;
using namespace granite::base;

int main(int argc, char **argv) {

	// std::cout << fs::getUserDirectory() << std::endl;
	// std::cout << fs::getExecutableDirectory() << std::endl;
	// fs::open(fs::getUserDirectory() + "/ge", fs::directoryTypeWorkingDirecotry);
	// for (auto &f : fs::listFiles())
	// 	std::cout << f.path << " " << f.name << " " << f.dir << std::endl;
	// std::cout << std::endl;
	// for (auto &f : fs::findFiles("CMakeLists.txt"))
	// 	std::cout << f.path << " " << f.name << " " << f.dir << std::endl;
	// std::cout << std::endl;
	// for (auto &f : fs::matchFiles(".*\\.txt"))
	// 	std::cout << f.path << " " << f.name << " " << f.dir << std::endl;

	// return 0;
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
