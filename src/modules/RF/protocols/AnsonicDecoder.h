#ifndef ANSONIC_DECODER_H
#define ANSONIC_DECODER_H

#include <Arduino.h>
#include <stdint.h>
#include <String.h>

class AnsonicDecoder {
public:
    AnsonicDecoder();

    // Reset internal state.
    void reset();

    // Feed a single sample (level and duration in µs).
    void feed(bool level, uint32_t duration);

    // Process an array of samples; returns true if a valid code is detected.
    bool decode(long long int* samples, size_t sampleCount);

    // Return a formatted string with the decoded key, button and DIP info.
    // Also updates the GUI textarea.
    String getCodeString();

    // Returns true if a valid code has been detected.
    bool hasValidCode() const;

private:
    enum DecoderStep {
        StepReset,
        StepFoundStartBit,
        StepSaveDuration,
        StepCheckDuration
    };

    DecoderStep state;

    const uint32_t te_short;       // 555 µs
    const uint32_t te_long;        // 1111 µs
    const uint32_t te_delta;       // 120 µs tolerance
    const uint8_t  min_count_bit;  // 12 bits

    uint32_t decodeData;
    uint8_t  decodeCountBit;
    uint32_t te_last;

    // Generic protocol fields (final decoded values).
    uint32_t data;         // full key data
    uint8_t  dataCountBit; // number of bits received
    uint32_t btn;          // button field
    uint32_t cnt;          // DIP bits (extracted from key)
    String   protocolName;
    bool     validCodeFound;

    // Helper: compute absolute difference.
    uint32_t durationDiff(uint32_t a, uint32_t b) const;

    // Helper: add a bit (to the key data).
    void addBit(uint8_t bit);
};

#endif // ANSONIC_DECODER_H
