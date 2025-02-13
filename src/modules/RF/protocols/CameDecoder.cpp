#include "CameDecoder.h"
#include <stdio.h>
#include "lvgl.h"
#include "GUI/ScreenManager.h"
#include "globals.h"

CameDecoder::CameDecoder()
    : te_short(320),
      te_long(640),
      te_delta(150),
      min_count_bit(12),
      state(StepReset),
      decodeData(0),
      decodeCountBit(0),
      te_last(0),
      validCodeFound(false),
      finalCode(0),
      finalBitCount(0) {
}

void CameDecoder::reset() {
    state = StepReset;
    decodeData = 0;
    decodeCountBit = 0;
    te_last = 0;
    validCodeFound = false;
    finalCode = 0;
    finalBitCount = 0;
}

inline uint32_t CameDecoder::durationDiff(uint32_t a, uint32_t b) const {
    return (a > b) ? (a - b) : (b - a);
}

inline void CameDecoder::addBit(uint8_t bit) {
    decodeData = decodeData << 1 | bit;
    decodeCountBit++;
}

void CameDecoder::feed(bool level, uint32_t duration) {    
    switch(state) {
    case StepReset:
        if(!level && durationDiff(duration, te_short * 56) < te_delta * 52) {
            state = StepFoundStartBit;
        }
        break;

    case StepFoundStartBit:
        if(!level) {
            break;
        } else if(durationDiff(duration, te_short) < te_delta) {
            state = StepSaveDuration;
            decodeData = 0;
            decodeCountBit = 0;
        } else {
            state = StepReset;
        }
        break;

    case StepSaveDuration:
        if(!level) {
            if(duration > 5000) {
                state = StepFoundStartBit;
                if((decodeCountBit == min_count_bit) ||
                   (decodeCountBit == AIRFORCE_COUNT_BIT) ||
                   (decodeCountBit == PRASTEL_COUNT_BIT) ||
                   (decodeCountBit == CAME_24_COUNT_BIT)) {
                    validCodeFound = true;
                    finalCode = decodeData;
                    finalBitCount = decodeCountBit;
                }
                break;
            }
            te_last = duration;
            state = StepCheckDuration;
        } else {
            state = StepReset;
        }
        break;

    case StepCheckDuration:
        if(level) {
            if((durationDiff(te_last, te_short) <
                te_delta) &&
               (durationDiff(duration, te_long) <
                te_delta)) {
                addBit(0);
                state = StepSaveDuration;
            } else if(
                (durationDiff(te_last, te_long) <
                 te_delta) &&
                (durationDiff(duration, te_short) <
                 te_delta)) {
                addBit(1);
                state = StepSaveDuration;
            } else {
                state = StepReset;
            }
        } else {
            state = StepReset;
        }
        break;

    default:
        state = StepReset;
        break;
    }
}

bool CameDecoder::decode(long long int* samples, size_t sampleCount) {
    reset();
    for (size_t i = 0; i < sampleCount; i++) {
        if (samples[i] > 0) {
            feed(true, samples[i]);
        } else {
            feed(false, -samples[i]);
        }
        if (validCodeFound) {
            return true;
        }
    }
    return false;
}

uint32_t CameDecoder::reverseKey(uint32_t code, uint8_t bitCount) const {
    uint32_t reversed = 0;
    for(uint8_t i = 0; i < bitCount; i++) {
        reversed <<= 1;
        reversed |= (code >> i) & 1;
    }
    return reversed;
}

String CameDecoder::getCodeString() const {
    ScreenManager& screenMgr = ScreenManager::getInstance();

    char buf[128];
    uint32_t codeFound = finalCode;
    uint32_t codeReversed = reverseKey(finalCode, finalBitCount);
    const char* protocolName = "\nCame";
    if(finalBitCount == PRASTEL_COUNT_BIT) {
        protocolName = "\nPrastel";
    } else if(finalBitCount == AIRFORCE_COUNT_BIT) {
        protocolName = "\nAirforce";
    }
    sprintf(buf, "%s %dbit\r\nKey:0x%08lX\r\nYek:0x%08lX\r\n",
            protocolName, finalBitCount, codeFound, codeReversed);

    lv_obj_t * textarea;
    if(C1101preset == CUSTOM){
        textarea = screenMgr.text_area_SubGHzCustom;        
    } else {
        textarea = screenMgr.getTextArea();
    }

    lv_textarea_set_text(textarea, buf);        
    return String(buf);
}

bool CameDecoder::hasValidCode() const {
    return validCodeFound;
}
