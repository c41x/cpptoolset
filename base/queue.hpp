/*
 * granite engine 1.0 | 2006-2026 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
 * file: queue
 * created: 19-06-2026
 *
 * description: multithreaded queues based on Vyukov mpmc queue
 *
 * changelog:
 * - 19-06-2026: file created
 */

#pragma once
#include "includes.hpp"
#include "alignment.hpp"
#include <atomic>

namespace granite { namespace base {

template <typename T>
class queue_mpmc {
    struct cell_t {
        std::atomic<size_t> sequence;
        T data;
    };

    cell_t* cells;
    size_t mask;
    alignas(cacheline_size) std::atomic<size_t> enqueue_pos;
    alignas(cacheline_size) std::atomic<size_t> dequeue_pos;
    queue_mpmc() = delete;

public:
    queue_mpmc(size_t cellssize) {
        assert((cellssize >= 2) && ((cellssize & (cellssize - 1)) == 0));

        cells = new cell_t[cellssize];
        mask = cellssize - 1;

        for (size_t i = 0; i != cellssize; i += 1)
            cells[i].sequence.store(i, std::memory_order_relaxed);

        enqueue_pos.store(0, std::memory_order_relaxed);
        dequeue_pos.store(0, std::memory_order_relaxed);
    }

    ~queue_mpmc() {
        delete [] cells;
    }

    bool push_ts(T const& data) {
        cell_t* cell;
        size_t pos = enqueue_pos.load(std::memory_order_relaxed);

        while (true) {
            cell = &cells[pos & mask];
            size_t seq = cell->sequence.load(std::memory_order_acquire);
            intptr_t dif = (intptr_t)seq - (intptr_t)pos;

            if (dif == 0) {
                if (enqueue_pos.compare_exchange_weak(
                        pos, pos + 1,
                        std::memory_order_relaxed)) {
                    break;
                }
            }
            else if (dif < 0) {
                return false;
            }
            else {
                pos = enqueue_pos.load(std::memory_order_relaxed);
            }
        }

        cell->data = data;
        cell->sequence.store(pos + 1, std::memory_order_release);
        return true;
    }

    bool pop_ts(T& data) {
        cell_t* cell;
        size_t pos = dequeue_pos.load(std::memory_order_relaxed);

        while(true) {
            cell = &cells[pos & mask];
            size_t seq = cell->sequence.load(std::memory_order_acquire);
            intptr_t dif = (intptr_t)seq - (intptr_t)(pos + 1);

            if (dif == 0) {
                if (dequeue_pos.compare_exchange_weak(
                        pos, pos + 1,
                        std::memory_order_relaxed)) {
                    break;
                }
            }
            else if (dif < 0) {
                return false;
            }
            else {
                pos = dequeue_pos.load(std::memory_order_relaxed);
            }
        }

        data = cell->data;
        cell->sequence.store(pos + mask + 1, std::memory_order_release);
        return true;
    }
};

}}
