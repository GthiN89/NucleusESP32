#include "AnsonicProtocol.h"
#include <stdio.h>
#include "globals.h"

#define DIP_PATTERN "%c%c%c%c%c%c%c%c%c%c"
#define CNT_TO_DIP(dip) \
    (dip & 0x0800 ? '1' : '0'), (dip & 0x0400 ? '1' : '0'), (dip & 0x0200 ? '1' : '0'), \
    (dip & 0x0100 ? '1' : '0'), (dip & 0x0080 ? '1' : '0'), (dip & 0x0040 ? '1' : '0'), \
    (dip & 0x0020 ? '1' : '0'), (dip & 0x0010 ? '1' : '0'), (dip & 0x0001 ? '1' : '0'), \
    (dip & 0x0008 ? '1' : '0')

static inline uint32_t duration_diff(uint32_t a, uint32_t b) {
    return (a > b) ? (a - b) : (b - a);
}

AnsonicProtocol::AnsonicProtocol()
    : te_short(555),
      te_long(1111),
      te_delta(120),
      min_count_bit(12),
      DecoderState(DecoderStepReset),
      decodeData(0),
      decodeCountBit(0),
      te_last(0),
      validCodeFound(false),
      finalCode(0),
      finalBitCount(0),
      finalBtn(0),
      finalDip(0),
      binaryValue(0) {
}

void AnsonicProtocol::reset() {
    DecoderState = DecoderStepReset;
    decodeData = 0;
    decodeCountBit = 0;
    te_last = 0;
    validCodeFound = false;
    finalCode = 0;
    finalBitCount = 0;
    finalBtn = 0;
    finalDip = 0;
}

inline void AnsonicProtocol::addBit(uint8_t bit) {
    decodeData = (decodeData << 1) | bit;
    decodeCountBit++;
}

void AnsonicProtocol::toBits(unsigned int hexValue) {
    binaryValue = std::bitset<12>(hexValue);
}

void AnsonicProtocol::yield(unsigned int hexValue) {
    switch (encoderState) {
    case EncoderStepStart:
        samplesToSend.clear();
        toBits(hexValue);
        encoderState = EncodeStepStartBit;
        break;
    case EncodeStepStartBit:
        encoderState = EncoderStepDurations;
        break;
    case EncoderStepDurations:
        // Start with a high pulse
        samplesToSend.push_back(555);
        // First repetition of 12-bit key data in reverse order (MSB first)
        for (int i = 11; i >= 0; i--) {
            if (binaryValue.test(i)) {
                samplesToSend.push_back(1111); // te_long for bit '1'
                samplesToSend.push_back(555);  // te_short for bit '1'
            } else {
                samplesToSend.push_back(555);  // te_short for bit '0'
                samplesToSend.push_back(1111); // te_long for bit '0'
            }
        }
        // Fixed pause and high pulse after first repetition
        samplesToSend.push_back(19425);
        samplesToSend.push_back(555);
        // Second repetition
        for (int i = 11; i >= 0; i--) {
            if (binaryValue.test(i)) {
                samplesToSend.push_back(1111);
                samplesToSend.push_back(555);
            } else {
                samplesToSend.push_back(555);
                samplesToSend.push_back(1111);
            }
        }
        // Fixed pause and high pulse after second repetition
        samplesToSend.push_back(19425);
        samplesToSend.push_back(555);
        // Third repetition
        for (int i = 11; i >= 0; i--) {
            if (binaryValue.test(i)) {
                samplesToSend.push_back(1111);
                samplesToSend.push_back(555);
            } else {
                samplesToSend.push_back(555);
                samplesToSend.push_back(1111);
            }
        }
        // End with a fixed pause
        samplesToSend.push_back(19425);
        encoderState = EncoderStepReady;
        delay(5);
        break;
    default:
        break;
    }
}



void AnsonicProtocol::feed(bool level, uint32_t duration) {
    switch(DecoderState) {
    case DecoderStepReset:
        if(!level && duration_diff(duration, 19425) < 4200) {
            DecoderState = DecoderStepFoundStartBit;
        }
        break;
    case DecoderStepFoundStartBit:
        if(!level) {
            break;
        } else if(duration_diff(duration, te_short) < te_delta) {
            DecoderState = DecoderStepSaveDuration;
            decodeData = 0;
            decodeCountBit = 0;
        } else {
            DecoderState = DecoderStepReset;
        }
        break;
    case DecoderStepSaveDuration:
        if(!level) {
            if(duration > (te_short * 4)) {
                DecoderState = DecoderStepFoundStartBit;
                if(decodeCountBit >= min_count_bit) {
                    validCodeFound = true;
                    finalCode = decodeData;
                    finalBitCount = decodeCountBit;
                    finalBtn = (finalCode >> 1) & 0x3;
                    finalDip = finalCode & 0x0FFF;
                }
                break;
            }
            te_last = duration;
            DecoderState = DecoderStepCheckDuration;
        } else {
            DecoderState = DecoderStepReset;
        }
        break;
    case DecoderStepCheckDuration:
        if(level) {
            if(duration_diff(te_last, te_short) < te_delta &&
               duration_diff(duration, te_long) < te_delta) {
                addBit(1);
                DecoderState = DecoderStepSaveDuration;
            } else if(duration_diff(te_last, te_long) < te_delta &&
                      duration_diff(duration, te_short) < te_delta) {
                addBit(0);
                DecoderState = DecoderStepSaveDuration;
            } else {
                DecoderState = DecoderStepReset;
            }
        } else {
            DecoderState = DecoderStepReset;
        }
        break;
    }
}

bool AnsonicProtocol::decode(long long int* samples, size_t sampleCount) {
    reset();
    for(size_t i = 0; i < sampleCount; i++) {
        if(samples[i] > 0) {
            feed(true, samples[i]);
        } else {
            feed(false, (uint32_t)(-samples[i]));
        }
        if(validCodeFound) {
            return true;
        }
    }
    return false;
}

uint32_t AnsonicProtocol::reverseKey(uint32_t code, uint8_t bitCount) const {
    uint32_t reversed = 0;
    for(uint8_t i = 0; i < bitCount; i++) {
        reversed <<= 1;
        reversed |= (code >> i) & 1;
    }
    return reversed;
}

String AnsonicProtocol::getCodeString() const {
    char buf[128];
    uint32_t codeReversed = reverseKey(finalCode, finalBitCount);
    sprintf(buf, "Ansonic %dbit\r\nKey:0x%08lX\r\nYek:0x%08lX\r\nBtn:%X\r\nDIP:" DIP_PATTERN "\r\n",
            finalBitCount,
            finalCode,
            codeReversed,
            finalBtn,
            CNT_TO_DIP(finalDip));
    return String(buf);
}

bool AnsonicProtocol::hasValidCode() const {
    return validCodeFound;
}
