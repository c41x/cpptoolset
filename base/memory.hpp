/*
 * granite engine 1.0 | 2006-2026 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
 * file: memory
 * created: 20-04-2026
 *
 * description: memory manipulation utility
 *
 * changelog:
 * - 20-04-2026: file created
 */

#pragma once
#include "includes.hpp"
#include "alignment.hpp"

// TODO: make sure that poke and templated reads are aligned (uint32_t must have alignment of 4 to not crash on arm)
void *memoryOffset(void *memory, uint32_t offset) {
    return ((char*)memory) + offset;
}

template <typename T> T *memoryOffset(void *memory, uint32_t offset) {
    return ((T*)memoryOffset(memory, offset));
}

template <typename T> T& memoryPoke(void *memory, uint32_t offset = 0) {
    return *memoryOffset<T>(memory, offset);
}

template <typename T> T& memoryPoke(void *memory, uint32_t *offset) {
    T &ret = *memoryOffset<T>(memory, *offset);
    *offset += sizeof(T);
    return ret;
}

// moves offset to match required alignment
void memoryAlign(void *memory, uint32_t *offset, size_t align) {
    *offset += alignOffset((uintptr_t)memory + *offset, (uintptr_t)align);
}
