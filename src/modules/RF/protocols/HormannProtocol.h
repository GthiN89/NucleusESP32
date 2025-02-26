#ifndef HORMANN_DECODER_H
#define HORMANN_DECODER_H

#include <Arduino.h>
#include <stdint.h>
#include "math.h"
#include <bitset>
#include "globals.h"


enum HormanEncoderState {
    HormanEncoderStepStart,
    HormanEncoderStepStartBit,
    HormanEncoderStepLowStart,
    HormanEncoderStepDurations,
    HormanEncoderStepReady
};


class HormannProtocol {
public:
     HormannProtocol();

    void yield(uint64_t hexValue);

    HormanEncoderState hormanEncoderState;

    std::vector<uint32_t> samplesToSend;

    std::bitset<64> binaryValue;

    uint8_t bitCount; // We'll encode 44 bits

    // Resets internal state.
    void reset();

    // Feeds an array of samples; returns true if a valid code is detected.
    bool decode(long long int* samples, size_t sampleCount);

    // Returns a formatted string with the decoded key, its reverse, and button field.
    String getCodeString() const;

    // Returns true if a valid code was detected.
    bool hasValidCode() const;

private:
    enum DecoderStep {
        StepReset,
        StepFoundStartHeader, // (not actively used in feed)
        StepFoundHeader,      // (not actively used in feed)
        StepFoundStartBit,
        StepSaveDuration,
        StepCheckDuration
    };

    DecoderStep state;

    const uint32_t te_short;    // 500 µs
    const uint32_t te_long;     // 1000 µs
    const uint32_t te_delta;    // 200 µs tolerance
    const uint8_t  min_count_bit; // 44 bits

    static const uint64_t HORMANN_HSM_PATTERN = 0xFF000000003ULL;

    uint64_t decodeData;
    uint8_t decodeCountBit;
    uint32_t te_last;

    bool validCodeFound;
    uint64_t finalCode;
    uint8_t finalBitCount;

    void addBit(uint8_t bit);
    uint64_t reverseKey(uint64_t code, uint8_t bitCount) const;
    bool checkPattern() const;
    void feed(bool level, uint32_t duration);
};

#endif // HORMANN_DECODER_H
