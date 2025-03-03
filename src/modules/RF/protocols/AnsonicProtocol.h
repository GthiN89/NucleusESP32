#ifndef ANSONIC_PROTOCOL_H
#define ANSONIC_PROTOCOL_H

#include <Arduino.h>
#include <stdint.h>
#include <bitset>
#include <vector>
#include "globals.h"

class AnsonicProtocol {
public:
    AnsonicProtocol();

    // resets internal state
    void reset();

    // feeds an array of samples; returns true if a valid code was detected.
    bool decode(long long int* samples, size_t sampleCount);

    // returns a string with the decoded key, button and DIP info.
    String getCodeString() const;

    // returns true if a valid code was detected.
    bool hasValidCode() const;

    // generates samples for transmission from a given hex key.
    void yield(unsigned int hexValue);


private:
enum DecoderStep {
    DecoderStepReset,
    DecoderStepFoundStartBit,
    DecoderStepSaveDuration,
    DecoderStepCheckDuration
};

DecoderStep DecoderState;

    const uint32_t te_short;   // 555 us
    const uint32_t te_long;    // 1111 us
    const uint32_t te_delta;   // 120 us tolerance
    const uint8_t  min_count_bit; // 12 bits

    // Decoder variables
    uint32_t decodeData;
    uint8_t  decodeCountBit;
    uint32_t te_last;
    bool     validCodeFound;
    uint32_t finalCode;
    uint8_t  finalBitCount;
    uint8_t  finalBtn;  // Derived from key: (finalCode >> 1) & 0x3
    uint16_t finalDip;  // Derived from key: finalCode & 0x0FFF

    // Encoder variables
    std::bitset<12> binaryValue;

    // Helpers
    inline void addBit(uint8_t bit);
    uint32_t reverseKey(uint32_t code, uint8_t bitCount) const;
    void toBits(unsigned int hexValue);
    void feed(bool level, uint32_t duration);
};

#endif // ANSONIC_PROTOCOL_H
