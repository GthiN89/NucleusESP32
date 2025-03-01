#ifndef SMC5326_DECODER_H
#define SMC5326_DECODER_H

#include <Arduino.h>
#include <stdint.h>
#include <string.h>
#include "math.h"
#include "globals.h"
#include <bitset>

class Smc5326Protocol {
public:
    Smc5326Protocol();

    std::bitset<25> binaryValue; 


    // Resets internal state.
    void reset();

    // Feed a single sample (level and duration in microseconds).
    void feed(bool level, uint32_t duration);

    // Process an array of samples; returns true if a valid code is detected.
    bool decode(long long int* samples, size_t sampleCount);

    // Returns a formatted string with the decoded key, TE value, DIP visualization, and event info.
    // Also updates the GUI textarea.
    String getCodeString();

    // Returns true if a valid code has been detected.
    bool hasValidCode() const;

    void yield(uint32_t hexValue);

private:
    enum DecoderStep {
        StepReset,
        StepSaveDuration,
        StepCheckDuration,
    };

    // Constants from the protocol.
    const uint32_t te_short;      // 300 µs
    const uint32_t te_long;       // 900 µs
    const uint32_t te_delta;      // 200 µs tolerance
    const uint8_t  min_count_bit; // 25 bits

    // Internal state.
    DecoderStep state;
    uint32_t decodeData;
    uint8_t  decodeCountBit;
    uint32_t te_last;  // Last measured duration at the transition
    uint32_t te;       // Accumulated timing value for averaging
    uint32_t lastData; // For validating repeated keys

    // Final decoded (generic) values.
    uint32_t finalData;         // The decoded key data
    uint8_t  finalDataCountBit; // Number of bits in the key
    uint32_t finalTE;           // Final TE (averaged timing)

    bool validCodeFound;


    // Helper: add a bit to the decode data.
    void addBit(uint8_t bit);



    // Helper: generate an event string from event bits.
    String getEventString(uint8_t event) const;
};

#endif // SMC5326_DECODER_H
