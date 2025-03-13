#ifndef CAME_DECODER_H
#define CAME_DECODER_H

#include <Arduino.h>
#include <stdint.h>
#include "math.h"
#include <bitset>



class CameProtocol {
public:
    CameProtocol();


    // resets internal state
    void reset();

    // feeds an array of samples; returns true if a valid code was detected.
    bool decode(long long int* samples, size_t sampleCount);


    // returns a string with the decoded key and its reverse.
    String getCodeString(uint64_t shortPulse, uint64_t longPulse) const;

    // returns true if a valid code was detected.
    bool hasValidCode() const;
    void yield(unsigned int hexValue);
 
private:
    enum DecoderStep {
        DecoderStepReset,
        DecoderStepFoundStartBit,
        DecoderStepSaveDuration,
        DecoderStepCheckDuration
    };

    DecoderStep DecoderState;
  
   

    
    

    const uint32_t te_short;  // 320 us
    const uint32_t te_long;   // 640 us
    const uint32_t te_delta;  // 150 us tolerance
    const uint8_t  min_count_bit; // 12 bits

    static const uint8_t CAME_12_COUNT_BIT = 12;
    static const uint8_t AIRFORCE_COUNT_BIT = 18;
    static const uint8_t CAME_24_COUNT_BIT = 24;
    static const uint8_t PRASTEL_COUNT_BIT = 25;

    uint32_t decodeData;
    uint8_t  decodeCountBit;
    uint32_t te_last;

    bool validCodeFound;
    uint32_t finalCode;
    uint8_t  finalBitCount;

    std::bitset<12> binaryValue; 


    void toBits(unsigned int hexValue); 
   

    inline void addBit(uint8_t bit);
    uint32_t reverseKey(uint32_t code, uint8_t bitCount) const;
    void feed(bool level, uint32_t duration);
};

#endif // CAME_DECODER_H
