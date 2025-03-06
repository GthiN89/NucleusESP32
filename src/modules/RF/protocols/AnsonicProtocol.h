#ifndef ANSONIC_PROTOCOL_H
#define ANSONIC_PROTOCOL_H

#include <Arduino.h>
#include <stdint.h>
#include <bitset>
#include <vector>
#include "globals.h"
#include "math.h"
#include "GUI/ScreenManager.h"

#define DIP_PATTERN "%c%c%c%c%c%c%c%c%c%c"
#define CNT_TO_DIP(dip) \
    (dip & 0x0800 ? '1' : '0'), (dip & 0x0400 ? '1' : '0'), (dip & 0x0200 ? '1' : '0'), \
    (dip & 0x0100 ? '1' : '0'), (dip & 0x0080 ? '1' : '0'), (dip & 0x0040 ? '1' : '0'), \
    (dip & 0x0020 ? '1' : '0'), (dip & 0x0010 ? '1' : '0'), (dip & 0x0001 ? '1' : '0'), \
    (dip & 0x0008 ? '1' : '0')


class AnsonicProtocol {
public:
    AnsonicProtocol();
    void reset();
    void feed(bool level, uint32_t duration);
    bool decode(long long int* samples, size_t sampleCount);
    String getCodeString() const;
    bool hasValidCode() const;
    void yield(unsigned int hexValue);
    void checkRemoteController();
    CC1101_PRESET preset;

private:
    enum DecoderStep {
        DecoderStepReset,
        DecoderStepFoundStartBit,
        DecoderStepSaveDuration,
        DecoderStepCheckDuration,
        DecoderStepFound
    };

    // Decoder fields
    DecoderStep DecoderState;
    uint32_t decodeData;
    uint8_t  decodeCountBit;
    uint32_t te_last;
    bool     validCodeFound;
    uint32_t finalCode;
    uint8_t  finalBitCount;
    uint8_t  finalBtn;
    uint16_t finalDip;
    uint8_t btn;
    uint32_t cnt;
    uint32_t serial;

    // Protocol constants
    const uint32_t te_short;      // 555 us
    const uint32_t te_long;       // 1111 us
    const uint32_t te_delta;      // 120 us tolerance
    const uint32_t space;       // 1111 us
    const uint8_t  min_count_bit; // 12 bits

    // Encoder fields
    enum {
        EncoderStepStart,
        EncodeStepStartBit,
        EncoderStepDurations,
        EncoderStepReady
    } encoderState = EncoderStepStart;
    std::bitset<12> binaryValue;


    inline void addBit(uint8_t bit);
    void toBits(unsigned int hexValue);
    uint32_t reverseKey(uint32_t code, uint8_t bitCount) const;
};

#endif // ANSONIC_PROTOCOL_H
