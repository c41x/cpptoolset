/*
 * granite engine 1.0 | 2006-2019 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
 * file: freelist
 * created: 13-01-2019
 *
 * description: FreeList memory allocators
 *
 * changelog:
 * - 13-01-2019: file created
 */

#pragma once
#include "includes.hpp"

namespace granite { namespace base {

// fixed size allocator
template <typename T, size_t SIZE> struct free_allocator {
    std::array<T, SIZE> mem;
    T *free;

    free_allocator() {
        static_assert(sizeof(T*) <= sizeof(T), "FreeList is not working for such small elements");
        clear();
    }

    void clear() {
        T *p = mem.data();
        free = p;

        for (int i = 0; i < SIZE - 1; ++i) {
            *((T**)p) = p + 1;
            ++p;
        }

        *((T**)p) = nullptr;
    }

    T *add() {
        T *recoveredItem = free;
        free = *((T**)free);
        return recoveredItem;
    }

    void remove(T *addr) {
        *((T**)addr) = free;
        free = addr;
    }

    template <typename... ARGS> T *construct(ARGS... args) {
        return new (add()) T(args...);
    }

    void destruct(T *addr) {
        addr->~T();
        remove(addr);
    }

    bool full() const { return free == nullptr; }
};

template <typename T, size_t PAGE_SIZE> struct paged_free_allocator {
    std::list<std::array<T, PAGE_SIZE>> pages;
    T *free;

    paged_free_allocator() : free(nullptr) {
        static_assert(sizeof(T*) <= sizeof(T), "FreeList is not working for such small elements");
    }

    void add_page() {
        pages.push_back(std::array<T, PAGE_SIZE>());

        T *p = pages.back().data();
        free = p;

        for (int i = 0; i < PAGE_SIZE - 1; ++i) {
            *((T**)p) = p + 1;
            ++p;
        }

        *((T**)p) = nullptr;
    }

    T *add() {
        if (free == nullptr) {
            add_page();
        }

        T *recoveredItem = free;
        free = *((T**)free);
        return recoveredItem;
    }

    void remove(T *addr) {
        *((T**)addr) = free;
        free = addr;
    }

    template <typename... ARGS> T *construct(ARGS... args) {
        return new (add()) T(args...);
    }

    void destruct(T *addr) {
        addr->~T();
        remove(addr);
    }

    bool full() const { return free == nullptr; }
};

}}

// TODO: lock-free versions (make T* atomic + correct acq/rel)
//~
