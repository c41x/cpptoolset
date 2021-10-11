#include <base/base.hpp>
#include <base/scheduler.hpp>
#include <condition_variable>

using namespace granite;
using namespace granite::base;

int countWords(const string &s, int begin, int end, int &result) {
    result = (int)std::count(s.begin() + begin, s.begin() + end, ' ') +
        (int)std::count(s.begin() + begin, s.begin() + end, '\t') +
        (int)std::count(s.begin() + begin, s.begin() + end, '\n');
    return result;
}

int printResult(std::vector<int> &results) {
    int wordCount = std::accumulate(results.begin(), results.end(), 0);
    std::cout << "[info] scheduler result: " << wordCount << " words" << std::endl;
    return wordCount;
}

void scheduleNextTask();

std::atomic<int> jobsDone = {0};

void countWordsTask(const string &s, int begin, int end, int &result) {
    countWords(s, begin, end, result);
    ++jobsDone;

    scheduleNextTask();
    scheduleNextTask();
    scheduleNextTask();
}

std::atomic<size_t> taskI = {0}, taskJ = {0};
string *taskTxt;
std::vector<int> results;
size_t taskSize;

scheduler<std::function<void()>> sc(8);

void scheduleNextTask() {
    size_t i = taskI.fetch_add(taskSize);
    size_t j = taskJ.fetch_add(1);
    if (i < taskTxt->size()) {
        sc.schedule(std::bind(countWordsTask, std::ref(*taskTxt), i, std::min(i + taskSize, taskTxt->size()), std::ref(results[j])));
    }
}

int main(int argc, char **argv) {
    timer::init();

    // generate random string
    rng<> rn;
    const size_t txtSize = 1024 * 1024 * 256; // 128MB
    const string characters = " \n\tqwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM1234567890.,;'/[]{}";
    string txt;
    txt.reserve(txtSize);
    for (size_t i = 0; i < txtSize; ++i) {
        txt.push_back(*rn.pickOne(characters.begin(), characters.end()));
    }

    // prepare scheduler
    taskTxt = &txt;
    taskSize = txt.size() / 1000 + 1; // <= 1000 tasks
    results.resize(1001, 0);

    std::cout << "[info] data generated" << std::endl;

    timer t;
    t.reset();

    while (jobsDone < 1000) {
        scheduleNextTask();
    }

    sc.shutdown();

    int words = printResult(results);
    float time = t.timeMs();
    std::cout << "[info] scheduler result: " << toStr(time) << " ms" << std::endl;

    // cleanup
    fs::close();

    // print result from main thread
    int result = 0;
    t.reset();
    int sWords = countWords(txt, 0, txt.size(), result);
    float sTime = t.timeMs();
    std::cout << "[info] should be: " << result << " completed in " << sTime << " ms" << std::endl;

    std::cout << (time < sTime ? "[ok] scheduler faster" : "[fail] scheduler slower") << std::endl;
    std::cout << (words == sWords ? "[ok] results match" : "[fail] results incorrect") << std::endl;

    return 0;
}
