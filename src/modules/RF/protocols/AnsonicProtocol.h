#ifndef ANSONICDECODER_H
#define ANSONICDECODER_H

#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>
#include "math.h"

#define DIP_PATTERN "%c%c%c%c%c%c%c%c%c%c"
#define CNT_TO_DIP(dip)                                                                         \
    ((dip & 0x0800) ? '1' : '0'), ((dip & 0x0400) ? '1' : '0'), ((dip & 0x0200) ? '1' : '0'),     \
    ((dip & 0x0100) ? '1' : '0'), ((dip & 0x0080) ? '1' : '0'), ((dip & 0x0040) ? '1' : '0'),     \
    ((dip & 0x0020) ? '1' : '0'), ((dip & 0x0010) ? '1' : '0'), ((dip & 0x0001) ? '1' : '0'),     \
    ((dip & 0x0008) ? '1' : '0')

class AnsonicProtocol {
public:
    AnsonicProtocol();

    // Resets the decoder to its initial state.
    void reset();

    // Feeds a signal pulse to the decoder.
    void feed(bool level, uint32_t duration);

    // Processes an array of raw signal samples.
    bool decode(long long int* samples, size_t sampleCount);

    // Returns a formatted string of the decoded code.
    String getCodeString() const;

    // Returns true if a valid code has been decoded.
    bool hasValidCode() const;
    void toBits(unsigned int hexValue);
    // Transmits the given code using the Ansonic protocol.
    // The code is transmitted with the specified bit count.
    void transmit(uint32_t code, uint8_t bitCount);

    void yield(uint32_t hexValue);

private:
    // Adds a bit to the decoded data.
    inline void addBit(uint8_t bit);

    // Timing constants for the Ansonic protocol.
    const uint32_t te_short;
    const uint32_t te_long;
    const uint32_t te_delta;
    const uint8_t  min_count_bit;

        std::bitset<12> binaryValue; 


    // Decoder state machine steps.
    enum DecoderStep {
        StepReset = 0,
        StepFoundStartBit,
        StepSaveDuration,
        StepCheckDuration,
    };

    DecoderStep state;
    uint32_t decodeData;
    uint8_t decodeCountBit;
    uint32_t te_last;

    // Final decoded data.
    bool validCodeFound;
    uint32_t finalCode;
    uint8_t finalBitCount;
};

#endif // ANSONICDECODER_H
