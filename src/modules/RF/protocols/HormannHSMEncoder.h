#ifndef HORMANN_HSM_ENCODER_H
#define HORMANN_HSM_ENCODER_H

#include <cstddef>
#include <cstdint>
#include <vector>
#include "HormannHSMDecoder.h"  // For TE_SHORT, TE_LONG, TE_DELTA, MIN_COUNT_BIT, HORMANN_HSM_PATTERN

class HormannHSMEncoder {
public:
    HormannHSMEncoder() = default;
    ~HormannHSMEncoder() = default;

    // key: the full 64-bit value to encode.
    // bit_count: total number of bits to encode. It must be >= HormannHSMDecoder::MIN_COUNT_BIT.
    // The least significant HormannHSMDecoder::MIN_COUNT_BIT bits should equal HORMANN_HSM_PATTERN.
    std::vector<long long int> encode(uint64_t key, uint32_t bit_count) const;
};

#endif // HORMANN_HSM_ENCODER_H
