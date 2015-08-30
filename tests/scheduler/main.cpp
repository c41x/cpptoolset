#include <base/base.hpp>
#include <base/scheduler.hpp>

using namespace granite;
using namespace granite::base;

int results[1000];
void ta(int i) {
	results[i] = i * 2;
	std::this_thread::sleep_for(std::chrono::milliseconds(20));
}

void finisher() {
	for (int i = 0; i < 1000; ++i) {
		std::cout << results[i] << "\t";
		if (i % 10 == 0)
			std::cout << std::endl;
	}
}

void countWords(const string &s, int begin, int end, int &result) {
	result = std::count(s.begin() + begin, s.begin() + end, ' ') +
		std::count(s.begin() + begin, s.begin() + end, '\t') +
		std::count(s.begin() + begin, s.begin() + end, '\n');
}

void printResult(std::vector<int> &results) {
	int wordCount = std::accumulate(results.begin(), results.end(), 0);
	std::cout << "result: " << wordCount << " words" << std::endl;
}

int main(int argc, char **argv) {
	fs::open("/home/calx/tmp");
	string txt = toStr(fs::load("log-text"));
	std::vector<int> results;

	scheduler s(1024);

	size_t taskSize = txt.size() / 1000; // 1000 tasks
	results.resize(1001);
	jobID wordCountFinisher = s.addJob(std::bind(printResult, std::ref(results)), jobPriorityDependent);
	for (size_t i = 0, j = 0; i < txt.size(); i += taskSize) {
		s.addJob(std::bind(countWords, std::ref(txt), i, std::min(i + taskSize, txt.size()), std::ref(results[j++])), jobPriorityHigh, wordCountFinisher);
	}
	// jobID taskFinisher = s.addJob(finisher, jobPriorityDependent);
	// for (int i = 0; i < 1000; ++i) {
	// 	s.addJob(std::bind(ta, i), jobPriorityHigh, taskFinisher);
	// }

	timer t;
	t.init();
	t.reset();
	s.start(4);
	s.stop();
	std::cout << "\nresult: " << toStr(t.timeS()) << std::endl;

	fs::close();
	return 0;
}
