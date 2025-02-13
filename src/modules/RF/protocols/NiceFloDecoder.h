#ifndef NICE_FLO_DECODER_H
#define NICE_FLO_DECODER_H

#include <Arduino.h>
#include <stdint.h>

class NiceFloDecoder {
public:
    NiceFloDecoder();

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

    const uint32_t te_short;  // 700 us
    const uint32_t te_long;   // 1400 us
    const uint32_t te_delta;  // 200 us tolerance
    const uint8_t  min_count_bit; // 12 bits

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

#endif // NICE_FLO_DECODER_H
