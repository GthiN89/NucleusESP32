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

//-----------------------------------------------------------------------------
//  ENCODER: "yield" with a state machine (like the proven Came/Ansonic approach)
//-----------------------------------------------------------------------------
void HoltekProtocol::yield(unsigned int hexValue) {
    switch(encoderState) {
    case EncoderStepStart:
        // Clear old pulses
        samplesToSend.clear();
        // If TE isn't yet set, default to te_short
        if(te == 0) te = te_short;
        // Move on
        encoderState = EncodeStepStartBit;
        break;

    case EncodeStepStartBit:
        // Next step: actually create pulses
        encoderState = EncoderStepDurations;
        break;

    case EncoderStepDurations: {
        // 1) Send header: low pulse = te*36, high pulse = te
        samplesToSend.push_back(te * 36);
        samplesToSend.push_back(te);

        // 2) For each of the 12 bits (MSB first):
        for(int i = 11; i >= 0; i--) {
            bool bitIsOne = (hexValue & (1 << i)) != 0;
            if(bitIsOne) {
                // For bit '1': low = te*2, high = te
                samplesToSend.push_back(te * 2);
                samplesToSend.push_back(te);
            } else {
                // For bit '0': low = te, high = te*2
                samplesToSend.push_back(te);
                samplesToSend.push_back(te * 2);
            }
        }
        // All pulses are prepared
        encoderState = EncoderStepReady;
        // Slight delay to mirror your proven logic
        delay(5);
        break;
    }

    default:
        // Once we're in EncoderStepReady, do nothing.
        break;
    }
}

//-----------------------------------------------------------------------------
//  DECODER: same as the original Holtek logic—no changes or local replacements
//-----------------------------------------------------------------------------
bool HoltekProtocol::decode(long long int* samples, size_t sampleCount) {
    // Start fresh
    reset();

    for(size_t i = 0; i < sampleCount; i++) {
        // Positive => high level; negative => low level
        if(samples[i] > 0) {
            // HIGH
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
                // Check if previous low pulse + this high pulse => bit=1 or bit=0
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
                // do nothing in other states
                break;
            }
        } else {
            // LOW
            uint32_t duration = (uint32_t)(-samples[i]);
            switch(decoderState) {
            case DecoderStepReset:
                // Look for header: ~ te_short * 36
                if(duration_diff(duration, te_short * 36) < (te_delta * 36)) {
                    decoderState = DecoderStepFoundStartBit;
                }
                break;

            case DecoderStepSaveDuration:
                // If low pulse is >= (te_short*4), we consider it an end-of-group
                if(duration >= (te_short * 4)) {
                    // Enough bits => code is valid
                    if(decodeCountBit >= min_count_bit) {
                        validCodeFound = true;
                        finalBtn = (decodedData & 0x0F);
                        finalDIP = (decodedData >> 4) & 0xFF;
                    }
                    // Prepare for next group
                    decodedData = 0;
                    decodeCountBit = 0;
                    decoderState = DecoderStepFoundStartBit;
                } else {
                    // Store for the next check
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
    // Show the final decode, including DIP pattern
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
