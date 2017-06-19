#include <base/base.hpp>
#include <base/scheduler.hpp>
#include <condition_variable>

using namespace granite;
using namespace granite::base;
using namespace granite::base::detail;

void countWords(const string &s, int begin, int end, int &result) {
    result = (int)std::count(s.begin() + begin, s.begin() + end, ' ') +
        (int)std::count(s.begin() + begin, s.begin() + end, '\t') +
        (int)std::count(s.begin() + begin, s.begin() + end, '\n');
}

void printResult(std::vector<int> &results) {
    int wordCount = std::accumulate(results.begin(), results.end(), 0);
    std::cout << "result: " << wordCount << " words" << std::endl;
}

class scheduler2 {
    struct worker {
        std::function<void()> work;
        std::atomic<bool> workToDo;
        std::atomic<bool> run;
        semaphore sem;
        int id;
        std::thread t;

        scheduler2 *sc;

        void initialize(scheduler2 *_sc) {
            run = true;
            workToDo = false;
            sc = _sc;
            t = std::thread(std::bind(runWorker, std::ref(*this)));
        }

        static void runWorker(worker &context) {
            //std::cout << "thread start >\n" << std::flush;

            while (context.run) {
                while (!context.workToDo && context.run) {
                    std::cout << "sleep ...\n" << std::flush;
                    context.sem.wait();
                }

                if (context.workToDo) {
                    context.work(); // work must be done first - task scheduling is very lightweight
                    //std::cout << "&";
                    context.workToDo = false;

                    int ind = ++context.sc->tip;
                    context.sc->T[ind - 1] = context.id;

                    //T[context.id] = -1;

                    //printState();
                }
            }

            std::cout << "thread done <\n" << std::flush;
        }
    };

    size_t threadsCount;
    std::atomic<int> *T; // merge below
    worker *TT;
    std::atomic<int> tip;

public:

    scheduler2(size_t maxThreads) {
        threadsCount = maxThreads;

        T = new std::atomic<int>[maxThreads];
        TT = new worker[maxThreads];

        tip = maxThreads;

        for (size_t i = 0; i < maxThreads; ++i) {
            T[i] = i;
            TT[i].id = i;
            TT[i].initialize(this);
        }
    }

    void schedule(std::function<void()> fx) {
        if (tip > 0) {
            int ind = --tip;
            //for (int ind = 0; ind < MAX_T; ++ind)

            if (T[ind] != -1) {
                //std::cout << "#";
                // thread is free -> acquire
                printState();
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

    void printState() {
        for (int i = 0; i < threadsCount; ++i) {
            std::cout << T[i] << " ";
        }
        std::cout << "  | " << tip << std::endl;
    }

    void shutdown() {
        for (size_t i = 0; i < threadsCount; ++i) {
            TT[i].run = false;
            TT[i].sem.notify();
            TT[i].t.join();
        }
    }
};

void scheduleNextTask();

// task information graph
std::map<std::thread::id, int> data;

// (2)
std::atomic<int> jobsDone = {0};

void countWords2(const string &s, int begin, int end, int &result) {
    //countWords(s, begin, end, result);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
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
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
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

scheduler2 sc(8);

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

    taskSize = txt.size() / 1000 + 1; // <= 1000 tasks
    results.resize(1001, 0);

    timer t;
    t.init();
    t.reset();
    while (jobsDone < 1000) {
        scheduleNextTask();
    }

    for (int i = 0; i < 10; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        jobsDone = 0;
        taskI = 0;
        taskJ = 0;

        while (jobsDone < 1000) {
            scheduleNextTask();
        }
    }

    std::cout << "finishing all threads" << std::endl;

    sc.shutdown();

    // while (run);

    //std::cout << "entering finishing while loop" << std::endl;

    printResult(results);
    std::cout << "\nresult: " << toStr(t.timeMs()) << " ms" << std::endl;

    #endif

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
