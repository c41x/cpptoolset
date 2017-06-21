#include <base/base.hpp>
#include <base/scheduler.hpp>
#include <condition_variable>

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

void scheduleNextTask();

// task information graph
std::map<std::thread::id, int> data;

// (2)
std::atomic<int> jobsDone = {0};

void countWords2(const string &s, int begin, int end, int &result) {
    countWords(s, begin, end, result);
    //std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ++jobsDone;

    // // gather information data TODO: make thread safe (thread_local?)
    // auto e = data.find(std::this_thread::get_id());
    // if (e == data.end()) {
    //     data.insert(std::make_pair(std::this_thread::get_id(), 0));
    // }
    // else {
    //     e->second++;
    // }

    //std::cout << "+";

    //std::cout << jobsDone << std::endl;

    if (jobsDone >= 1000) {
        //std::cout << "DONE" << std::endl;
        //run = false;

        // // this IS thread safe ?
        // for (auto &t : TT) {
        //     t.run = false;
        //     t.sem.notify();
        // }
    }

    if (jobsDone == 10) {
        //std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    scheduleNextTask();
    scheduleNextTask();
    scheduleNextTask();
    // scheduleNextTask();
    // scheduleNextTask();
    // scheduleNextTask();
}

std::atomic<size_t> taskI = {0}, taskJ = {0};
string *taskTxt;
std::vector<int> results;
size_t taskSize;

scheduler sc(8);

void scheduleNextTask() {
    size_t i = taskI.fetch_add(taskSize);
    size_t j = taskJ.fetch_add(1);
    if (i < taskTxt->size()) {
        //std::cout << "> " << i << "/" << std::min(i + taskSize, taskTxt->size()) << std::endl;
        sc.schedule(std::bind(countWords2, std::ref(*taskTxt), i, std::min(i + taskSize, taskTxt->size()), std::ref(results[j])));
    }
}

int main(int argc, char **argv) {
    #ifdef GE_PLATFORM_WINDOWS
    fs::open("c:/tmp");
    string txt = toStr(fs::load("log-text.txt"));
    #else
    fs::open("/home/calx/tmp");
    string txt = toStr(fs::load("log-text"));
    #endif

    taskTxt = &txt;

    taskSize = txt.size() / 1000 + 1; // <= 1000 tasks
    results.resize(1001, 0);

    timer t;
    t.init();
    t.reset();
    while (jobsDone < 1000) {
        scheduleNextTask();
    }

    // for (int i = 0; i < 10; ++i) {
    //     std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    //     jobsDone = 0;
    //     taskI = 0;
    //     taskJ = 0;

    //     while (jobsDone < 1000) {
    //         scheduleNextTask();
    //     }
    // }

    std::cout << "finishing all threads" << std::endl;

    // while (run);

    //std::cout << "entering finishing while loop" << std::endl;

    sc.shutdown();

    printResult(results);
    std::cout << "\nresult: " << toStr(t.timeMs()) << " ms" << std::endl;





    fs::close();

    int result = 0;
    t.reset();
    countWords(txt, 0, txt.size(), result);
    std::cout << "should be: " << result << " in " << t.timeMs() << " ms" << std::endl;


    std::cout << std::endl << "scheduling thread info: " << std::endl;
    for (auto &t : data) {
        std::cout << t.first << " tasks done: " << t.second << std::endl;
    }

    return 0;
}
