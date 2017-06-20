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

class scheduler2 {
    struct worker {
        std::function<void()> work; // work to do
        std::atomic<bool> workToDo; // marked true when there is job to do
        std::atomic<bool> run; // global run flag - this is not scheduler member to avoid false sharing (?test this?)
        detail::semaphore semaphore; // just mutex for sleeping
        std::thread thread;
        int id; // thread identifier

        scheduler2 *parentScheduler; // parent context

        void initialize(scheduler2 *parentSchedulerInstance) {
            run = true;
            workToDo = false;
            parentScheduler = parentSchedulerInstance;
            thread = std::thread(std::bind(workerThread, std::ref(*this)));
        }

        static void workerThread(worker &context) {
            //std::cout << "thread start >\n" << std::flush;

            while (context.run) {
                while (!context.workToDo && context.run) {
                    std::cout << "sleep ...\n" << std::flush;
                    context.semaphore.wait();
                }

                if (context.workToDo) {
                    context.work(); // work must be done first - task scheduling is very lightweight
                    //std::cout << "&";
                    context.workToDo = false;

                    int ind = ++context.sc->tip;
                    context.sc->freeWorkers[ind - 1] = context.id;

                    //T[context.id] = -1;

                    //printState();
                }
            }

            std::cout << "thread done <\n" << std::flush; // TODO: remove
        }
    };

    // how many worker threads we have
    size_t threadsCount;

    // free workers list, tip is pointing to free worker
    std::atomic<int> *freeWorkers;
    std::atomic<int> tip;

    // list of workers
    worker *workers;

public:

    explicit scheduler2(size_t maxThreads) {
        threadsCount = maxThreads;

        freeWorkers = new std::atomic<int>[maxThreads];
        tip = maxThreads;

        workers = new worker[maxThreads];

        for (size_t i = 0; i < maxThreads; ++i) {
            freeWorkers[i] = i;
            workers[i].id = i;
            workers[i].initialize(this);
        }
    }

    void schedule(std::function<void()> work) {
        // if tip is > 0 it means that there could be free worker thread to complete the task
        if (tip > 0) {
            // acquire tip index and free worker id
            int freeWorkerIndex = --tip;
            int freeWorkerId = freeWorkers[freeWorkerIndex];

            // it may happen that thread is not ready yet, it does not matter
            // we go wait free
            if (freeWorkerId != -1) {
                // but if this thread is free -> acquire it and send work to do
                workers[freeWorkerId].work = work;
                workers[freeWorkerId].workToDo = true;
                workers[freeWorkerId].semaphore.notify();
                freeWorkers[freeWorkerIndex] = -1;
                return;
            }
        }

        // there are no free worker threads -> run task on caller thread
        // this allows us to utilize main thread too (I do not want a *special* threads
        // like render thread, audio thread etc.)
        work();
    }

    void shutdown() {
        for (size_t i = 0; i < threadsCount; ++i) {
            workers[i].run = false;
            workers[i].semaphore.notify();
            workers[i].thread.join();
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
