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
#include <atomic>

namespace granite { namespace base {

// fixed size allocator
template <typename T, size_t SIZE> struct free_allocator {
    union node {
        T data;
        node *next;
    };

    std::array<node, SIZE> mem;
    node *free;

    free_allocator() {
        clear();
    }

    void clear() {
        node *p = mem.data();
        free = p;

        for (int i = 0; i < SIZE - 1; ++i) {
            p->next = p + 1;
            ++p;
        }

        p->next = nullptr;
    }

    T *add() {
        node *recoveredItem = free;
        free = free->next;
        return &recoveredItem->data;
    }

    void remove(T *addr) {
        node *removed_node = (node*)addr;
        removed_node->next = free;
        free = removed_node;
    }

    template <typename... ARGS> T *construct(ARGS... args) {
        return new (add()) T(args...);
    }

    void destruct(T *addr) {
        addr->~T();
        remove(addr);
    }
};

// lock free multiple producer (freeing memory), single consumer
// (allocating memory) free list allocator. all functions except
// ones with _ts prefix are not thread safe
template <typename T, size_t SIZE> struct free_allocator_mpsc {
    union node {
        T data;
        node *next;
    };

    std::array<node, SIZE> mem;
    node *free;
    std::atomic<node*> bin; // separate linked list that is used for freeing memory

    free_allocator_mpsc() {
        bin = nullptr;
        clear();
    }

    void clear() {
        node *p = mem.data();
        free = p;

        for (int i = 0; i < SIZE - 1; ++i) {
            p->next = p + 1;
            ++p;
        }

        p->next = nullptr;
    }

    T *add() {
        if (free == nullptr) {
            // there is no memory left in free list
            // we just exchange free list for disposed list
            // into local one
            free = bin.exchange(nullptr, std::memory_order_acquire);
        }

        node *recoveredItem = free;
        free = free->next;
        return &recoveredItem->data;
    }

    void remove(T *addr) {
        node *removed_node = (node*)addr;
        removed_node->next = free;
        free = removed_node;
    }

    void remove_ts(T *addr) {
        node *removed_node = (node*)addr;

        // removed node now points to bin head
        // this is to avoid looping in while loop below
        // if we don't do that this loop always fails on first
        // try, entering slow path
        removed_node->next = bin.load(std::memory_order_relaxed);

        // 1) checks bin == removed_node->next
        // 2) if false -> removed_node->next = bin
        // 3) if true -> bin = removed_node
        while (bin.compare_exchange_weak(removed_node->next, removed_node,
                                         std::memory_order_release,
                                         std::memory_order_relaxed));
    }

    template <typename... ARGS> T *construct(ARGS... args) {
        return new (add()) T(args...);
    }

    void destruct(T *addr) {
        addr->~T();
        remove(addr);
    }
};

template <typename T, size_t PAGE_SIZE> struct paged_free_allocator {
    union node {
        T data;
        node *next;
    };

    std::list<std::array<node, PAGE_SIZE>> pages;
    node *free;

    paged_free_allocator() : free(nullptr) {}

    void add_page() {
        pages.emplace_back();

        node *p = pages.back().data();
        free = p;

        for (size_t i = 0; i < PAGE_SIZE - 1; ++i) {
            p->next = p + 1;
            ++p;
        }

        p->next = nullptr;
    }

    T *add() {
        if (free == nullptr) {
            add_page();
        }

        node *recoveredItem = free;
        free = free->next;
        return &recoveredItem->data;
    }

    void remove(T *addr) {
        node *removed_node = (node*)addr;
        removed_node->next = free;
        free = removed_node;
    }

    template <typename... ARGS> T *construct(ARGS... args) {
        return new (add()) T(args...);
    }

    void destruct(T *addr) {
        addr->~T();
        remove(addr);
    }
};

// thread safe removing
// no-thread safe adding
template <typename T, size_t PAGE_SIZE> struct paged_free_allocator_mpsc {
    union node {
        T data;
        node *next;
    };

    std::list<std::array<node, PAGE_SIZE>> pages;
    node *free;
    std::atomic<node*> bin; // separate linked list that is used for freeing memory

    paged_free_allocator_mpsc() : free(nullptr), bin(nullptr) {}

    void add_page() {
        pages.emplace_back();

        node *p = pages.back().data();
        free = p;

        for (size_t i = 0; i < PAGE_SIZE - 1; ++i) {
            p->next = p + 1;
            ++p;
        }

        p->next = nullptr;
    }

    T *add() {
        if (free == nullptr) {
            // there is no memory left in free list
            // we just exchange free list for disposed list
            // into local one
            free = bin.exchange(nullptr, std::memory_order_acquire);
        }

        if (free == nullptr) {
            // if there is still no memory, it allocates new page
            add_page();
        }

        node *recoveredItem = free;
        free = free->next;
        return &recoveredItem->data;
    }

    void remove(T *addr) {
        node *removed_node = (node*)addr;
        removed_node->next = free;
        free = removed_node;
    }

    void remove_ts(T *addr) {
        node *removed_node = (node*)addr;

        // removed node now points to bin head
        // this is to avoid looping in while loop below
        // if we don't do that this loop always fails on first
        // try, entering slow path
        removed_node->next = bin.load(std::memory_order_relaxed);

        // 1) checks bin == removed_node->next
        // 2) if false -> removed_node->next = bin
        // 3) if true -> bin = removed_node
        while (bin.compare_exchange_weak(removed_node->next, removed_node,
                                         std::memory_order_release,
                                         std::memory_order_relaxed));
    }

    template <typename... ARGS> T *construct(ARGS... args) {
        return new (add()) T(args...);
    }

    void destruct(T *addr) {
        addr->~T();
        remove(addr);
    }
};
}}

//~
