#ifndef CHAMBERLAIN_CODE_DECODER_H
#define CHAMBERLAIN_CODE_DECODER_H

#include <Arduino.h>
#include <stdint.h>
#include <String.h>
#include "math.h"

class ChamberlainCodeDecoder {
public:
    ChamberlainCodeDecoder();

    // Reset internal state.
    void reset();

    // Feed a single sample (level and duration in microseconds).
    void feed(bool level, uint32_t duration);

    // Process an array of samples; returns true if a valid code is detected.
    bool decode(long long int* samples, size_t sampleCount);

    // Returns a formatted string with the decoded key, its reversed value ("Yek"), and DIP info.
    // Also updates the GUI textarea.
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

    // Timing constants (in microseconds) and tolerances.
    static const uint32_t TE_SHORT = 1000;
    static const uint32_t TE_LONG  = 3000;
    static const uint32_t TE_DELTA = 200;
    static const uint8_t  MIN_COUNT_BIT = 10;

    // Chamberlain code bit definitions.
    static const uint8_t CH_CODE_BIT_STOP = 0b0001;
    static const uint8_t CH_CODE_BIT_1    = 0b0011;
    static const uint8_t CH_CODE_BIT_0    = 0b0111;

    // Code masks and expected check values.
    static const uint64_t CHAMBERLAIN_7_CODE_MASK       = 0xF000000FF0FULL;
    static const uint64_t CHAMBERLAIN_8_CODE_MASK       = 0xF00000F00FULL;
    static const uint64_t CHAMBERLAIN_9_CODE_MASK       = 0xF000000000FULL;
    static const uint64_t CHAMBERLAIN_7_CODE_MASK_CHECK = 0x10000001101ULL;
    static const uint64_t CHAMBERLAIN_8_CODE_MASK_CHECK = 0x1000001001ULL;
    static const uint64_t CHAMBERLAIN_9_CODE_MASK_CHECK = 0x10000000001ULL;

    // Internal state.
    DecoderStep parserStep;
    uint64_t decodeData;
    uint8_t  decodeCountBit;
    uint32_t te_last; // Last measured pulse duration

    // Final decoded key.
    uint64_t finalData;
    uint8_t  finalDataCountBit;
    bool     validCodeFound;

    // Helper functions.
    bool convertDataToBits(uint64_t &data, uint8_t size) const;
    bool checkMaskAndParse();
    uint64_t reverseKey(uint64_t code, uint8_t bitCount) const;
    String getDIPString(uint32_t code) const;
};

#endif // CHAMBERLAIN_CODE_DECODER_H
