#include <base/base.hpp>
#include <base/scheduler.hpp>
#include <condition_variable>

class semaphore
{
public:
    std::mutex mutex_;
    std::condition_variable condition_;
    unsigned long count_ = 0; // Initialized as locked.

public:
    void notify() {
        std::unique_lock<decltype(mutex_)> lock(mutex_);
        //std::cout << "unlock" << std::endl;
        ++count_;
        condition_.notify_one();
    }

    void wait() {
        std::unique_lock<decltype(mutex_)> lock(mutex_);
        //std::cout << "wait" << std::endl;
        while(!count_) // Handle spurious wake-ups.
            condition_.wait(lock);
        //std::cout << "wait end" << std::endl;
        --count_;
    }

    bool try_wait() {
        std::unique_lock<decltype(mutex_)> lock(mutex_);
        if(count_) {
            --count_;
            return true;
        }
        return false;
    }
};

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

const int MAX_T = 8;
std::atomic<int> T[MAX_T];
std::atomic<int> tip;
bool run = true;

struct worker;
void runWorker(worker &context);

struct worker {
    std::thread t;
    std::function<void()> work;
    std::atomic<bool> workToDo;
    //std::mutex mtx;
    //std::condition_variable cv;
    semaphore sem;
    int id;
    worker() : t(std::bind(runWorker, std::ref(*this))), workToDo(false) {}
};

void printState() {
    for (int i = 0; i < MAX_T; ++i) {
        std::cout << T[i] << " ";
    }
    std::cout << "  | " << tip << std::endl;
}

void runWorker(worker &context) {
    while (run) {
        while (!context.workToDo && run) {
            context.sem.wait();
        }

        if (context.workToDo) {
            context.work(); // work must be done first - task scheduling is very lightweight
            //std::cout << "&";
            context.workToDo = false;

            int ind = ++tip;
            T[ind - 1] = context.id;

            //T[context.id] = -1;

            //printState();
        }
    }
}

worker TT[MAX_T];

void init() {
    for (int i = 0; i < MAX_T; ++i) {
        T[i] = i;
        TT[i].id = i;
        TT[i].workToDo = false;
    }

    tip = MAX_T;
}

void scheduleNextTask();

void schedule(std::function<void()> fx) {
    if (tip > 0) {
        int ind = --tip;
        //for (int ind = 0; ind < MAX_T; ++ind)

            if (T[ind] != -1) {
                //std::cout << "#";
                // thread is free -> acquire
                //printState();
                TT[T[ind]].work = fx;
                TT[T[ind]].workToDo = true;
                TT[T[ind]].sem.notify();
                T[ind] = -1;
                // run fx on thread ind
                return;
            }
    }

    // run on this thread
    //std::cout << ".";
    //std::cout << std::this_thread::get_id() << " ";
    fx();
}

// (2)
std::atomic<int> jobsDone = {0};
void countWords2(const string &s, int begin, int end, int &result) {
    countWords(s, begin, end, result);
    jobsDone++;

    //std::cout << "+";

    if (jobsDone >= 1000) {
        run = false;
    }

    if (jobsDone == 999) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    scheduleNextTask();
    scheduleNextTask();
    scheduleNextTask();
    scheduleNextTask();
    scheduleNextTask();
    scheduleNextTask();
}

std::atomic<size_t> taskI = {0}, taskJ = {0};
string *taskTxt;
std::vector<int> results;
size_t taskSize;

void scheduleNextTask() {
    size_t i = taskI.fetch_add(taskSize);
    size_t j = taskJ.fetch_add(1);
    if (i < taskTxt->size())
        schedule(std::bind(countWords2, std::ref(*taskTxt), i, std::min(i + taskSize, taskTxt->size()), std::ref(results[j])));
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

    //#define GRANITE_SCHEDULER
    #if defined(GRANITE_SCHEDULER)

    scheduler s(1024);

    taskSize = txt.size() / 1000 + 1; // <= 1000 tasks
    results.resize(1001, 0);
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


    #else

    init();
    taskSize = txt.size() / 1000 + 1; // <= 1000 tasks
    results.resize(1001, 0);

    timer t;
    t.init();
    t.reset();
    // for (size_t i = 0, j = 0; i < txt.size(); i += taskSize) {
    //     schedule(std::bind(countWords2, std::ref(txt), i, std::min(i + taskSize, txt.size()), std::ref(results[j++])));
    // }
    scheduleNextTask();

    while (run);

    for (auto &t : TT) {
        t.sem.notify();
        t.t.join();
    }

    printResult(results);
    std::cout << "\nresult: " << toStr(t.timeMs()) << " ms" << std::endl;

    #endif

    fs::close();


    int result = 0;
    t.reset();
    countWords(txt, 0, txt.size(), result);
    std::cout << "should be: " << result << " in " << t.timeMs() << " ms" << std::endl;

    return 0;
}
