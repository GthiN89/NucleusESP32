#ifndef HOLTEK_HT12X_DECODER_H
#define HOLTEK_HT12X_DECODER_H

#include <Arduino.h>
#include <stdint.h>
#include <String.h>

class HoltekHT12xDecoder {
public:
    HoltekHT12xDecoder();

    // Reset the internal state.
    void reset();

    // Feed a single sample (duration in microseconds; negative = LOW, positive = HIGH).
    void feed(bool level, uint32_t duration);

    // Process an array of samples; returns true if a valid code is detected.
    bool decode(long long int* samples, size_t sampleCount);

    // Returns a formatted string with the decoded key, button, DIP info, and TE value.
    // Also updates the GUI text area.
    String getCodeString();

    // Returns true if a valid code was detected.
    bool hasValidCode() const;

private:
    enum DecoderStep {
        StepReset = 0,
        StepFoundStartBit,
        StepSaveDuration,
        StepCheckDuration
    };

    // Timing parameters (from HT12x datasheet).
    const uint32_t te_short;   // 320 µs
    const uint32_t te_long;    // 640 µs
    const uint32_t te_delta;   // 200 µs tolerance
    const uint8_t  min_count_bit; // 12 bits expected

    // Working variables.
    DecoderStep parserStep;
    uint32_t te;             // Sum of durations (for averaging TE)
    uint64_t decodeData;     // Accumulated data bits (shifted in)
    uint8_t  decodeCountBit; // Number of bits accumulated
    uint32_t te_last;        // Last falling-edge duration
    uint32_t last_data;

    // Final decoded data.
    uint64_t finalData;
    uint8_t  finalDataCountBit;
    uint32_t te_final;       // Final averaged TE value

    // Decoded remote fields.
    uint32_t btn; // Typically lower nibble
    uint32_t cnt; // DIP info from lower 8 bits

    bool validCodeFound;
};

#endif // HOLTEK_HT12X_DECODER_H
