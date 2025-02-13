#ifndef CAME_ENCODER_H
#define CAME_ENCODER_H

#include <cstddef>
#include <cstdint>
#include <vector>
#include "CameDecoder.h" // For TE_SHORT, TE_LONG, TE_DELTA, MIN_COUNT_BIT, etc.

class CameEncoder {
public:
    CameEncoder() = default;
    ~CameEncoder() = default;

    // key: the full 64-bit value to encode.
    // bit_count: total number of bits to encode. It should match one of the valid lengths (e.g. MIN_COUNT_BIT, AIRFORCE_COUNT_BIT, PRASTEL_COUNT_BIT, or CAME_24_COUNT_BIT).
    // Each pulse is represented by a long long int:
    //  - A positive value indicates a high pulse.
    //  - A negative value indicates a low pulse.
    std::vector<long long int> encode(uint64_t key, uint32_t bit_count) const;
};

#endif // CAME_ENCODER_H
