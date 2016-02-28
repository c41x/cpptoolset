#include <system/system.hpp>

using namespace granite;
using namespace granite::base;
using namespace granite::system;

int main(int argc, char**argv) {
	bool run = true;
	hotkey::init();
	hotkey::add(getKey("K"), getModifier("M"), []() {
			std::cout << "hotkey ALT + B pressed!!!" << std::endl;
		});
	hotkey::add(getKey("K"), getModifier("C"), [&run]() {
			std::cout << "hotkey CTRL + B pressed!!!" << std::endl;
			run = false;
		});
	while (run) {
		hotkey::process();
	}
	hotkey::shutdown();
	return 0;
}
