/*
 * granite engine 1.0 | 2006-2015 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
 * file: scheduler
 * created: 13-07-2015
 *
 * description: multithread job scheduler
 *
 * changelog:
 * - 17-09-2007: file created
 * - 13-07-2015: rewrite
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

enum jobPriority {
    jobPriorityLow,
    jobPriorityMedium,
    jobPriorityHigh,
    jobPriorityNow,
    jobPriorityDependent
};

class scheduler {
    struct thread {
        std::thread *t;
    };

    struct job {
        std::function<void()> fn;
        jobPriority priority;
        int count;
        job *parent;
        job *nextFree;
    };

    std::vector<thread> _threads;
    std::vector<job> _jobsList; // free list containing all jobs
    std::vector<job*> _jobs; // jobs about to execute
    job *_freeJobs; // head of free list jobs
    std::mutex _jobMutex;

    std::vector<job*>::iterator findJob() {
        return std::find_if(_jobs.begin(), _jobs.end(),
                            [](job *j) {
                                return j->priority != jobPriorityDependent;
                            });
    }

    void fthread(size_t ti) {
        while (_jobs.size() > 0) {
            _jobMutex.lock();
            if (_jobs.size() > 0) {
                // find next job
                auto ji = findJob();
                auto j = *ji;
                auto fn = j->fn;
                _jobs.erase(ji);

                // release free list element
                j->nextFree = _freeJobs;
                _freeJobs = j;

                // update dependency (if all subtasks are done, just set priority to 'now')
                if (j->parent && --j->parent->count == 0)
                    j->parent->priority = jobPriorityNow;

                _jobMutex.unlock();

                // execute job
                fn();
            }
            else {
                _jobMutex.unlock();
            }
        }
    }

public:
    typedef job *jobID;

    explicit scheduler(size_t maxJobs) {
        // allocate and initialize free list containing jobs
        _jobsList.resize(maxJobs);
        job *prv = nullptr;
        for (size_t i = 0; i < maxJobs; ++i) {
            _jobsList[i].nextFree = prv;
            prv = &_jobsList[i];
        }
        _freeJobs = prv;
    }

    ~scheduler() {
        stop();
    }

    void stop() {
        // signal all threads to stop, then join them and clear threads list
        for (auto &t : _threads) t.t->join();
        _threads.clear();
    }

    void start(size_t numThreads) {
        // create and run threads
        gassert(_threads.size() == 0, "threads already initialized");
        for (size_t i = 0; i < numThreads; ++i)
            _threads.push_back({ new std::thread(&scheduler::fthread, this, i) });
    }

    jobID addJob(std::function<void()> fn, jobPriority priority, jobID parent = nullptr) {
        gassert(_freeJobs != nullptr, "max jobs limit exceeded");

        _jobMutex.lock();

        // find free block and update free list head
        job *j = _freeJobs;
        j->fn = fn;
        j->priority = priority;
        j->count = 0;
        j->parent = parent;
        _freeJobs = j->nextFree;

        // keep track of all allocated jobs
        _jobs.push_back(j);

        // if there is dependency - update count
        if (parent)
            ++parent->count;

        _jobMutex.unlock();

        // return ID
        return j;
    }
};

// shortcut
typedef scheduler::jobID jobID;

}}

// TODO: condition variable to wake up / kill threads
// TODO: relax max jobs limit - create temporary buffer with stored rest of jobs
// TODO: message priority selection (test this)
// TODO: someday - lock free version
