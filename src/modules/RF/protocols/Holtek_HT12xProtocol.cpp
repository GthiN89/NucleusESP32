#include "Holtek_HT12xProtocol.h"
#include <stdio.h>

#define DIP_PATTERN "%c%c%c%c%c%c%c%c"
#define CNT_TO_DIP(dip) \
    (dip & 0x80 ? '0' : '1'), (dip & 0x40 ? '0' : '1'), (dip & 0x20 ? '0' : '1'), \
    (dip & 0x10 ? '0' : '1'), (dip & 0x08 ? '0' : '1'), (dip & 0x04 ? '0' : '1'), \
    (dip & 0x02 ? '0' : '1'), (dip & 0x01 ? '0' : '1')

static inline uint32_t duration_diff(uint32_t a, uint32_t b) {
    return (a > b) ? (a - b) : (b - a);
}

HoltekProtocol::HoltekProtocol()
    : 
      decoderState(DecoderStepReset),
      te_short(320),
      te_long(640),
      te_delta(200),
      space(11520),
      min_count_bit(12),
      te(0),
      validCodeFound(false),
      decodeCountBit(0),
      decodedData(0),
      te_last(0),
      finalBtn(0),
      finalDIP(0)
{
}

void HoltekProtocol::reset() {
    // Reset decoder
    decoderState = DecoderStepReset;
    decodedData = 0;
    decodeCountBit = 0;
    te_last = 0;
    validCodeFound = false;
    finalBtn = 0;
    finalDIP = 0;
    // Reset encoder
    encoderState = EncoderStepStart;
    te = 0;
    samplesToSend.clear();
}

inline void HoltekProtocol::addBit(uint8_t bit) {
    decodedData = (decodedData << 1) | bit;
    decodeCountBit++;
}


void HoltekProtocol::yield(unsigned int hexValue) {
    switch(encoderState) {
    case EncoderStepStart:
        samplesToSend.clear();
        if(te == 0) te = te_short;
        encoderState = EncoderStepDurations;
        break;

    case EncoderStepDurations: {
        // 1) Send header: low pulse = te*36, high pulse = te
        samplesToSend.push_back(space);
        samplesToSend.push_back(te);

        for(int i = 11; i >= 0; i--) {
            bool bitIsOne = (hexValue & (1 << i)) != 0;
            if(bitIsOne) {
                // 1
                samplesToSend.push_back(te * 2);
                samplesToSend.push_back(te);
            } else {
                // 0
                samplesToSend.push_back(te);
                samplesToSend.push_back(te * 2);
            }
        }
        encoderState = EncoderStepReady;
        delay(5);
        break;
    }

    default:
        // Once we're in EncoderStepReady, do nothing.
        break;
    }
}


bool HoltekProtocol::decode(long long int* samples, size_t sampleCount) {
    reset();

    for(size_t i = 0; i < sampleCount; i++) {
        if(samples[i] > 0) {
            // 1
            uint32_t duration = (uint32_t)samples[i];
            switch(decoderState) {
            case DecoderStepFoundStartBit:
                // Expect ~te_short
                if(duration_diff(duration, te_short) < te_delta) {
                    decoderState = DecoderStepSaveDuration;
                    te = duration;         // measure actual TE from start bit
                    decodedData = 0;
                    decodeCountBit = 0;
                } else {
                    decoderState = DecoderStepReset;
                }
                break;

            case DecoderStepCheckDuration:
                // Check if bit=1 or bit=0
                if(duration_diff(te_last, te_short) < te_delta &&
                   duration_diff(duration, te_long) < te_delta) {
                    addBit(1);
                    decoderState = DecoderStepSaveDuration;
                } else if(duration_diff(te_last, te_long) < te_delta &&
                          duration_diff(duration, te_short) < te_delta) {
                    addBit(0);
                    decoderState = DecoderStepSaveDuration;
                } else {
                    decoderState = DecoderStepReset;
                }
                break;

            default:
                break;
            }
        } else {
            // LOW
            uint32_t duration = (uint32_t)(-samples[i]);
            switch(decoderState) {
            case DecoderStepReset:
                if(duration_diff(duration, space) < (space)) {
                    decoderState = DecoderStepFoundStartBit;
                }
                break;

            case DecoderStepSaveDuration:
                // If low pulse is >= (te_short*4), we consider it an end-of-group
                if(duration >= (te_short * 4)) {
                    //code is valid
                    if(decodeCountBit >= min_count_bit) {
                        validCodeFound = true;
                        finalBtn = (decodedData & 0x0F);
                        finalDIP = (decodedData >> 4) & 0xFF;
                    }
                    decodedData = 0;
                    decodeCountBit = 0;
                    decoderState = DecoderStepFoundStartBit;
                } else {
                    te_last = duration;
                    decoderState = DecoderStepCheckDuration;
                }
                break;

            default:
                // else reset
                decoderState = DecoderStepReset;
                break;
            }
        }
        if(validCodeFound) {
            return true;
        }
    }
    return false;
}

String HoltekProtocol::getCodeString() const {
    char buf[128];
    sprintf(
        buf,
        "Holtek HT12X %db\r\n"
        "Key:0x%03lX\r\n"
        "Btn:0x%X\r\n"
        "DIP:" DIP_PATTERN "\r\n"
        "Te:%luus\r\n",
        decodeCountBit,
        (decodedData & 0xFFF),
        finalBtn,
        CNT_TO_DIP(finalDIP),
        te
    );
    return String(buf);
}

bool HoltekProtocol::hasValidCode() const {
    return validCodeFound;
}
