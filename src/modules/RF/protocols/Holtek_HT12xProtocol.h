#ifndef HOLTEK_PROTOCOL_H
#define HOLTEK_PROTOCOL_H

#include <Arduino.h>
#include <stdint.h>
#include <vector>
#include "globals.h"

class HoltekProtocol {
public:
    HoltekProtocol();

    // Resets decoder state (and we can also reset encoder state if needed).
    void reset();

    // Feeds an array of samples (positive=HIGH, negative=LOW) for decoding.
    bool decode(long long int* samples, size_t sampleCount);

    // Returns a string with the decoded key, button, DIP, etc.
    String getCodeString() const;

    // Tells if decoding found a valid code.
    bool hasValidCode() const;

    // Generates pulses for a given 12-bit key (hexValue) with a yield state machine.
    void yield(unsigned int hexValue);

    // Returns the latest generated pulses for external usage.
    const std::vector<uint32_t>& getSamplesToSend() const { return samplesToSend; }

    // Expose encoder state so the brute function can watch for EncoderStepReady.

private:
    // Decoder states
    enum DecoderStep {
        DecoderStepReset = 0,
        DecoderStepFoundStartBit,
        DecoderStepSaveDuration,
        DecoderStepCheckDuration
    };

    DecoderStep decoderState;

    const uint32_t te_short;   // 320 us
    const uint32_t te_long;    // 640 us
    const uint32_t te_delta;   // 200 us tolerance
    const uint8_t  min_count_bit; // 12 bits

    // The TE actually measured (or set) for encoding.
    // If not set from decode, defaults to te_short in yield().
    uint32_t te;
    uint32_t space;

    // Decoder variables
    bool validCodeFound;
    uint8_t decodeCountBit;
    uint32_t decodedData;
    uint32_t te_last;

    // Final fields for display
    uint32_t finalBtn; // lower 4 bits
    uint32_t finalDIP; // next 8 bits

    // Internal buffer for encoding pulses
    std::vector<uint32_t> samplesToSend;

    // Helper to accumulate bits in decoding
    inline void addBit(uint8_t bit);
};

#endif // HOLTEK_PROTOCOL_H
