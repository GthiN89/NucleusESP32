#ifndef HOLTEK_HT12X_PROTOCOL_H
#define HOLTEK_HT12X_PROTOCOL_H

#include <Arduino.h>
#include <stdint.h>
#include <vector>
#include "GUI/ScreenManager.h"
#include "globals.h"



// Optional DIP macros for printing DIP states
#define DIP_PATTERN "%c%c%c%c%c%c%c%c"
#define CNT_TO_DIP(dip)                                                                     \
    (dip & 0x0080 ? '0' : '1'), (dip & 0x0040 ? '0' : '1'), (dip & 0x0020 ? '0' : '1'),     \
    (dip & 0x0010 ? '0' : '1'), (dip & 0x0008 ? '0' : '1'), (dip & 0x0004 ? '0' : '1'),     \
    (dip & 0x0002 ? '0' : '1'), (dip & 0x0001 ? '0' : '1')

class HoltekProtocol {
public:

    HoltekProtocol();

    void reset();

    std::bitset<12> binaryValue;

    bool decode(long long int* samples, size_t sampleCount);
    void toBits(unsigned int hexValue);
    
    CC1101_PRESET preset;

    String getCodeString() const;


    bool hasValidCode() const;

   
    void yield(unsigned int hexValue);


private:
    enum HoltekDecoderStep {
        DecoderStepReset,
        DecoderStepFoundStartBit,
        DecoderStepSaveDuration,
        DecoderStepCheckDuration,
        DecoderStepFound
    };
    HoltekDecoderStep decoderState;

    enum HoltekEncoderStep {
        EncoderStepStart,
        EncoderStepDurations,
        EncoderStepReady
    };
    HoltekEncoderStep encoderState;

    inline void addBit(uint8_t bit);


    void feed(bool level, uint32_t duration);

    // Decoder timing/config
    const uint32_t te_short;   //
    const uint32_t te_long;    
    const uint32_t te_delta;   
    const uint32_t space;     
    const uint8_t  min_count_bit; 

    // Decoder variables
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
};

#endif // HOLTEK_HT12X_PROTOCOL_H
