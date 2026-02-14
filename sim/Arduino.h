#pragma once

#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <chrono>
#include <thread>

using std::size_t;
using byte = uint8_t;

#ifndef PROGMEM
#define PROGMEM
#endif

#ifndef pgm_read_word
#define pgm_read_word(addr) (*(addr))
#endif

inline uint32_t millis() {
    static const auto start = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count());
}

inline void delay(uint32_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

inline long random(long maxValue) {
    if (maxValue <= 0) return 0;
    return std::rand() % maxValue;
}

inline long random(long minValue, long maxValue) {
    if (maxValue <= minValue) return minValue;
    return minValue + (std::rand() % (maxValue - minValue));
}

template <typename T>
inline T constrain(T x, T a, T b) {
    if (x < a) return a;
    if (x > b) return b;
    return x;
}

template <typename T>
inline T max(T a, T b) {
    return (a > b) ? a : b;
}

template <typename T>
inline T min(T a, T b) {
    return (a < b) ? a : b;
}
