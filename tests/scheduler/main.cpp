#include <base/base.hpp>
#include <base/scheduler.hpp>

using namespace granite;
using namespace granite::base;

void countWords(const string &s, int begin, int end, int &result) {
	result = (int)std::count(s.begin() + begin, s.begin() + end, ' ') +
		(int)std::count(s.begin() + begin, s.begin() + end, '\t') +
		(int)std::count(s.begin() + begin, s.begin() + end, '\n');
}

void printResult(std::vector<int> &results) {
	int wordCount = std::accumulate(results.begin(), results.end(), 0);
	std::cout << "result: " << wordCount << " words" << std::endl;
}

int main(int argc, char **argv) {
	#ifdef GE_PLATFORM_WINDOWS
	fs::open("c:/tmp");
	string txt = toStr(fs::load("log-text.txt"));
	#else
	fs::open("/home/calx/tmp");
	string txt = toStr(fs::load("log-text"));
	#endif

	std::vector<int> results;

	scheduler s(1024);

	size_t taskSize = txt.size() / 1000 + 1; // <= 1000 tasks
	results.resize(1001);
	jobID wordCountFinisher = s.addJob(std::bind(printResult, std::ref(results)), jobPriorityDependent);
	for (size_t i = 0, j = 0; i < txt.size(); i += taskSize) {
		s.addJob(std::bind(countWords, std::ref(txt), i, std::min(i + taskSize, txt.size()), std::ref(results[j++])), jobPriorityHigh, wordCountFinisher);
	}

	timer t;
	t.init();
	t.reset();
	s.start(8);
	s.stop();
	std::cout << "\nresult: " << toStr(t.timeMs()) << " ms" << std::endl;

	fs::close();
	return 0;
}
