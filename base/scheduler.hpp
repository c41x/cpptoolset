/*
 * granite engine 1.0 | 2006-2017 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
 * file: scheduler
 * created: 13-07-2015
 *
 * description: multithread blocking job scheduler
 *
 * changelog:
 * - 17-09-2007: file created
 * - 13-07-2015: rewrite
 * - 21-06-2017: simplified wait-free implementation without dependency management
 */

#pragma once

#include "includes.hpp"
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace granite { namespace base {

// blocking notifier
template <typename T_MESSAGE> class notifier {
    std::mutex m;
    std::condition_variable c;
    T_MESSAGE msg;

public:
    // call from producer: unlocks thread and passes message to thread
    void notify(T_MESSAGE passMessage) {
        std::unique_lock<std::mutex> lock(m);
        msg = passMessage;
        c.notify_one();
    }

    // call from consumer: locks thread and waits for message
    T_MESSAGE wait() {
        std::unique_lock<std::mutex> lock(m);
        msg = nullptr;
        while (msg == nullptr) // prevent spurious wake ups
            c.wait(lock);
        return msg;
    }
};

template <typename T_WORK> class scheduler {
    struct worker {
        std::atomic<bool> run; // global run flag - this is not scheduler member to avoid false sharing (?test this?)
        notifier<T_WORK> notify; // just mutex for sleeping
        std::thread thread;
        int id; // thread identifier

        scheduler *parentScheduler; // parent context

        void initialize(scheduler *parentSchedulerInstance) {
            run = true;
            parentScheduler = parentSchedulerInstance;
            thread = std::thread(std::bind(workerThread, std::ref(*this)));
        }

        static void workerThread(worker &context) {
            while (context.run) {
                // when there is nothing to do - thread will wait...
                T_WORK work = context.notify.wait();

                // ... util there is some task to do
                // just run task and mark worker thread as free
                work(); // work must be done first - task scheduling is lightweight

                // update state
                context.parentScheduler->freeWorkersCount++;
                context.parentScheduler->workerRunning[context.id] = false;
            }
        }
    };

    // how many worker threads we have
    size_t threadsCount;

    // worker thread status + sync
    std::atomic<bool> *workerRunning;
    std::atomic<int> freeWorkersCount;

    // list of workers
    worker *workers;

public:

    void initialize(size_t maxThreads) {
        threadsCount = maxThreads;

        workerRunning = new std::atomic<bool>[maxThreads];
        freeWorkersCount = maxThreads;

        workers = new worker[maxThreads];

        for (size_t i = 0; i < maxThreads; ++i) {
            workerRunning[i] = false;
            workers[i].id = i;
            workers[i].initialize(this);
        }

        logOK(strs("initialized scheduler with ", maxThreads, " threads"));
    }

    void schedule(T_WORK work) {
        if (!trySchedule(work)) {
            // FAST PATH
            // there are no free worker threads -> run task on caller thread
            // this allows us to utilize calling thread too (blocking scheduler)
            work();
        }
    }

    bool trySchedule(T_WORK work) {
        // if count is > 0 it means that there could be free worker thread to complete the task
        if (freeWorkersCount > 0) {
            // SLOW PATH
            // find free worker
            int worker = -1;

            // first one that is available
            for (size_t i = 0; i < threadsCount; ++i) {
                // try to acquire worker, check if is not working first
                // then replace flag to true atomicaly
                bool expectingFalse = false;
                if (workerRunning[i].compare_exchange_weak(expectingFalse, true)) {
                    worker = i;
                    freeWorkersCount--;
                    break;
                }
            }

            // run task on worker thread (if found one)
            if (worker >= 0) {
                workers[worker].notify.notify(work);
                return true;
            }
        }

        return false;
    }

    void shutdown() {
        for (size_t i = 0; i < threadsCount; ++i) {
            workers[i].run = false;
            workers[i].notify.notify(T_WORK::empty(this));
            workers[i].thread.join();
        }

        delete [] workerRunning;
    }

    size_t getThreadCount() const {
        return threadsCount;
    }
};

}}
