#include <base/base.hpp>
#include <csignal>

using namespace granite;
using namespace granite::base;

static bool run = true;
void sig_callback(int sig) {
	run = false;
}

int main(int argc, char **argv) {
	// break on C-c
	signal(SIGINT, sig_callback);

	// add directory watch
	#if defined(GE_PLATFORM_LINUX)
	auto wid = fs::addWatch("/home/kuba/tmp");
	#elif defined(GE_PLATFORM_WINDOWS)
	auto wid = fs::addWatch("c:/tmp");
	#endif

	// start monitoring
	while (run) {
		// pool data and print results
		auto cs = fs::pollWatch(wid);
		for (auto c : cs) {
			 switch (std::get<0>(c)) {
			 	case fs::fileMonitorAdd: std::cout << "+ " << std::get<1>(c) << std::endl; break;
			 	case fs::fileMonitorRemove: std::cout << "- " << std::get<1>(c) << std::endl; break;
			 	case fs::fileMonitorModify: std::cout << "= " << std::get<1>(c) << std::endl;
			 }
		}
	}

	// cleanup
	fs::removeWatch(wid);
	return 0;
}
