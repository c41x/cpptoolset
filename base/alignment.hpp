/*
 * granite engine 1.0 | 2006-2021 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
 * file: alignment
 * created: 10-11-2021
 *
 * description: alignment utilities
 *
 * changelog:
 * - 10-11-2021: file created
 */

#pragma once
#include "includes.hpp"
#include <atomic>

constexpr uint64_t maximum_alignment = alignof(std::max_align_t);
constexpr size_t cacheline_size = 64; // this produces warnings in clang and gcc: std::hardware_destructive_interference_size;

// required alignment for atomic operations. I prefer using raw types instead of
// std::atomic in structs. Just use this with alignas before specifying the type
template <typename T> constexpr uint64_t atomic_alignment = std::atomic_ref<T>::required_alignment;

// align up base to match specified alignment
template <typename T = uintptr_t> constexpr T align(T base, T alignment) {
    return (base + alignment - 1) & (~(alignment - 1));
}

// how much bytes must be added to base to match specified alignment
template <typename T = uintptr_t> constexpr T alignOffset(T base, T alignment) {
    return align<T>(base, alignment) - base;
}
