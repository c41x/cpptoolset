/*
 * granite engine 1.0 | 2006-2017 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
 * file: scheduler
 * created: 13-07-2015
 *
 * description: multithread job scheduler
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

namespace detail {

// simple C++11 semaphore
class semaphore {
    std::mutex m;
    std::condition_variable c;
    int count = 0;

public:
    void notify() {
        std::unique_lock<std::mutex> lock(m);
        ++count;
        c.notify_one();
    }

    void wait() {
        std::unique_lock<std::mutex> lock(m);
        while (count == 0)
            c.wait(lock);
        --count;
    }
};

}

class scheduler {
    struct worker {
        std::function<void()> work; // work to do
        std::atomic<bool> workToDo; // marked true when there is job to do
        std::atomic<bool> run; // global run flag - this is not scheduler member to avoid false sharing (?test this?)
        detail::semaphore semaphore; // just mutex for sleeping
        std::thread thread;
        int id; // thread identifier

        scheduler *parentScheduler; // parent context

        void initialize(scheduler *parentSchedulerInstance) {
            run = true;
            workToDo = false;
            parentScheduler = parentSchedulerInstance;
            thread = std::thread(std::bind(workerThread, std::ref(*this)));
        }

        static void workerThread(worker &context) {
            while (context.run) {
                // when there is nothing to do - thread will wait...
                while (!context.workToDo && context.run) {
                    context.semaphore.wait();
                }

                // ... otherwise just run task and mark worker thread as free
                if (context.workToDo) {
                    context.work(); // work must be done first - task scheduling is lightweight
                    context.workToDo = false;

                    // move tip position and store id in free worker list slot
                    int ind = ++context.parentScheduler->tip;
                    context.parentScheduler->freeWorkers[ind - 1] = context.id;
                }
            }
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

    explicit scheduler(size_t maxThreads) {
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

            // we need to check if tip is really >= 0 because multiple threads could
            // enter this if (tip > 0) at the same time and every each of them will
            // think that tip is correct and decrementing --tip could lead to freeWorkerIndex
            // to be negative
            if (freeWorkerIndex >= 0) {
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

}}
