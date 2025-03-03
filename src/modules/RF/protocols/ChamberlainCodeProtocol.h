#ifndef CHAMBERLAIN_CODE_PROTOCOL_H
#define CHAMBERLAIN_CODE_PROTOCOL_H

#include <Arduino.h>
#include <stdint.h>
#include <bitset>
#include <string>
#include <vector>
#include "globals.h"

class ChamberlainCodeProtocol {
public:
    ChamberlainCodeProtocol();

    // Resets internal state (encoder and decoder if implemented)
    void reset();

    // Generates transmission samples for a given hex key and bitCount (7, 8, or 9).
    // This is meant to be called repeatedly (as in Came) until the encoder state is ready.
    void yield(uint32_t hexValue, uint8_t bitCount);



    bool isEncoderReady() const { return encoderState == EncoderStepReady; }

    // (Optional) Returns a human-readable string for a given code.
    String getCodeString(uint32_t hexValue, uint8_t bitCount) const;

private:
    // Timing constants for Chamberlain protocol
    const uint32_t te_short;  // 1000 µs
    const uint32_t te_long;   // 3000 µs
    const uint32_t te_delta;  // 200 µs tolerance
    const uint8_t  min_count_bit; // Minimal count for decoding (10)

    // Precomputed end pulses (to avoid runtime multiplication)
    // End pulse depends on the least significant bit:
    static const uint32_t END_HIGH_FOR_1 = 3000;  // te_short * 3
    static const uint32_t END_LOW_FOR_1  = 42000;  // 1000 * 42
    static const uint32_t END_HIGH_FOR_0 = 1000;    // te_short
    static const uint32_t END_LOW_FOR_0  = 44000;   // 1000 * 44

    // Internal storage for bits.
    // Since we support up to 9-bit codes, we use a 9-bit bitset.
    std::bitset<9> binaryValue;

    // Private helper: convert hexValue into binary bits (using bitCount bits)
    void toBits(uint32_t hexValue, uint8_t bitCount);
};

#endif // CHAMBERLAIN_CODE_PROTOCOL_H
