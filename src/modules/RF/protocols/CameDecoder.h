#ifndef CAME_DECODER_H
#define CAME_DECODER_H

#include <Arduino.h>
#include <stdint.h>

struct Sample {
    bool level;
    uint32_t duration; // in microseconds
};

class CameDecoder {
public:
    CameDecoder();

    // resets internal state
    void reset();

    // feeds an array of samples; returns true if a valid code was detected.
    bool decode(long long int* samples, size_t sampleCount);

    // returns a string with the decoded key and its reverse.
    String getCodeString() const;

    // returns true if a valid code was detected.
    bool hasValidCode() const;

private:
    enum DecoderStep {
        StepReset,
        StepFoundStartBit,
        StepSaveDuration,
        StepCheckDuration
    };

    DecoderStep state;

    const uint32_t te_short;  // 320 us
    const uint32_t te_long;   // 640 us
    const uint32_t te_delta;  // 150 us tolerance
    const uint8_t  min_count_bit; // 12 bits

    static const uint8_t CAME_12_COUNT_BIT = 12;
    static const uint8_t AIRFORCE_COUNT_BIT = 18;
    static const uint8_t CAME_24_COUNT_BIT = 24;
    static const uint8_t PRASTEL_COUNT_BIT = 25;

    uint32_t decodeData;
    uint8_t  decodeCountBit;
    uint32_t te_last;

    bool validCodeFound;
    uint32_t finalCode;
    uint8_t  finalBitCount;

    inline uint32_t durationDiff(uint32_t a, uint32_t b) const;
    inline void addBit(uint8_t bit);
    uint32_t reverseKey(uint32_t code, uint8_t bitCount) const;
    void feed(bool level, uint32_t duration);
};

#endif // CAME_DECODER_H
