#ifndef MATH_H
#define MATH_H

#include <vector>
#include <algorithm>
#include <cstdint>

using namespace std;

#define DURATION_DIFF(x, y) (((x) < (y)) ? ((y) - (x)) : ((x) - (y)))


struct Sample {
    bool level;
    uint32_t duration; // in microseconds
};

typedef struct {
    const uint16_t te_long;
    const uint16_t te_short;
    const uint16_t te_delta;
    const uint8_t min_count_bit_for_found;
} SubGhzBlockConst;

// Function Declarations (Prototypes)
int64_t computeMedian(vector<int64_t>* nums);
int64_t medianOfTwo(int64_t a, int64_t b);
uint32_t getHashDataLong(uint64_t &decodeData, size_t len);

#endif // MATH_H
