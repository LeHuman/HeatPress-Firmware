#pragma once
#include <cstdint>

// Define LCG parameters
#define RAND_A 1103515245
#define RAND_C 12345
#define RAND_M 0x80000000 // 2^31

// Initialize the seed value (can be any non-zero value)
static uint32_t rand_seed = 1;

// Generate a random integer within the given range [min, max]
int32_t rndInt(int32_t min, int32_t max) {
    // Ensure min <= max
    if (min > max) {
        int32_t temp = min;
        min = max;
        max = temp;
    }

    // Generate a pseudo-random number using LCG algorithm
    rand_seed = (RAND_A * rand_seed + RAND_C) % RAND_M;

    // Scale and shift the random number to fit within the range [min, max]
    return (int32_t)(((uint64_t)rand_seed * (max - min + 1)) >> 31) + min;
}