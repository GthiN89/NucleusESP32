#ifndef NICE_FLO_DECODER_H
#define NICE_FLO_DECODER_H

#include <Arduino.h>
#include <stdint.h>
#include "math.h"
#include <bitset>

class NiceFloProtocol {
public:
    NiceFloProtocol();

    void reset();
    bool decode(long long int* samples, size_t sampleCount);
    String getCodeString() const;
    bool hasValidCode() const;
    void yield(unsigned int hexValue);


private:
    enum DecoderStep {
        StepReset,
        StepFoundStartBit,
        StepSaveDuration,
        StepCheckDuration
    };

    DecoderStep state;

      std::bitset<12> binaryValue; 

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



    inline void addBit(uint8_t bit);
    void toBits(unsigned int hexValue);
    uint32_t reverseKey(uint32_t code, uint8_t bitCount) const;
    void feed(bool level, uint32_t duration);
};

#endif // NICE_FLO_DECODER_H
