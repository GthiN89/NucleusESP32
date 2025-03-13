#ifndef SMC5326_PROTOCOL_H
#define SMC5326_PROTOCOL_H

#include <Arduino.h>
#include <stdint.h>
#include "GUI/ScreenManager.h"
#include "globals.h"
#include "../FlipperSubFile.h"

#define DIP_PATTERN "%c%c%c%c%c%c%c%c"
#define DIP_P 0b11 
#define DIP_O 0b10  
#define DIP_N 0b00 
#define SHOW_DIP_P(dip, check_dip)                         \
    ((((dip >> 0xE) & 0x3) == check_dip) ? '*' : '_'),     \
    ((((dip >> 0xC) & 0x3) == check_dip) ? '*' : '_'),     \
    ((((dip >> 0xA) & 0x3) == check_dip) ? '*' : '_'),     \
    ((((dip >> 0x8) & 0x3) == check_dip) ? '*' : '_'),     \
    ((((dip >> 0x6) & 0x3) == check_dip) ? '*' : '_'),     \
    ((((dip >> 0x4) & 0x3) == check_dip) ? '*' : '_'),     \
    ((((dip >> 0x2) & 0x3) == check_dip) ? '*' : '_'),     \
    ((((dip >> 0x0) & 0x3) == check_dip) ? '*' : '_')

class SMC5326Protocol {
public:
    SMC5326Protocol();

    void reset();


    bool decode(long long int* samples, size_t sampleCount);

    CC1101_PRESET preset;
    String getCodeString(uint64_t shortPulse, uint64_t longPulse) const;

    bool hasValidCode() const;
    bool decodeReversed(long long int* samples, size_t sampleCount);

    void yield(unsigned int code);


private:
    enum SMC5326DecoderStep {
        DecoderStepReset,
        DecoderStepSaveDuration,
        DecoderStepCheckDuration
    };
    SMC5326DecoderStep decoderState;


    const uint32_t te_short;    
    const uint32_t te_long;     
    const uint32_t te_delta;    
    const uint8_t  min_count_bit; 

    bool     validCodeFound;
    uint8_t  decodeCountBit;
    uint32_t decodedData;
    uint32_t te_last;
    uint32_t lastData;

    uint32_t finalCode;
    uint32_t finalBitCount;
    uint32_t finalBtn;
    uint32_t finalDIP;
    uint32_t te; 

    void feed(bool level, uint32_t duration);
    inline void addBit(uint8_t bit);
};

#endif // SMC5326_PROTOCOL_H
