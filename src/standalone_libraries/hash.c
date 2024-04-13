#include "hash.h"

// FNV HASH (1a?)
HASH_API_FUNCTION uint64_t HashBytes(void *data, uint64_t size) {
    uint8_t *data8 = (uint8_t *)data;
    uint64_t hash = (uint64_t)14695981039346656037ULL;
    for (uint64_t i = 0; i < size; i++) {
        hash = hash ^ (uint64_t)(data8[i]);
        hash = hash * (uint64_t)1099511628211ULL;
    }
    return hash;
}

HASH_API_FUNCTION RandomSeed MakeRandomSeed(uint64_t value) {
    RandomSeed result;
    result.a = value;
    return result;
}

HASH_API_FUNCTION uint64_t GetRandomU64(RandomSeed *state) {
    uint64_t x = state->a;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    return state->a = x;
}

HASH_API_FUNCTION int GetRandomRangeI(RandomSeed *seed, int first, int last_included) {
    uint64_t random = GetRandomU64(seed);
    int range = (last_included - first + 1);
    int mapped = random % range;
    int result = mapped + first;
    return result;
}

HASH_API_FUNCTION double GetRandomNormal(RandomSeed *series) {
    uint64_t rnd = GetRandomU64(series);
    double result = (double)rnd / (double)UINT64_MAX;
    return result;
}

HASH_API_FUNCTION double GetRandomNormalRange(RandomSeed *seed, double min, double max) {
    double value = GetRandomNormal(seed);
    double result = value * (max - min) + min;
    return result;
}

HASH_API_FUNCTION uint64_t HashMix(uint64_t x, uint64_t y) {
    x ^= y;
    x *= 0xff51afd7ed558ccd;
    x ^= x >> 32;
    return x;
}
