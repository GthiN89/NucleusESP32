#include "HormannDecoder.h"
#include "lvgl.h"
#include "GUI/ScreenManager.h"
#include "globals.h"

HormannDecoder::HormannDecoder()
    : te_short(500),
      te_long(1000),
      te_delta(200),
      min_count_bit(44),
      state(StepReset),
      decodeData(0),
      decodeCountBit(0),
      te_last(0),
      validCodeFound(false),
      finalCode(0),
      finalBitCount(0) {
}

void HormannDecoder::reset() {
    state = StepReset;
    decodeData = 0;
    decodeCountBit = 0;
    te_last = 0;
    validCodeFound = false;
    finalCode = 0;
    finalBitCount = 0;
}

uint32_t HormannDecoder::durationDiff(uint32_t a, uint32_t b) const {
    return (a > b) ? (a - b) : (b - a);
}

void HormannDecoder::addBit(uint8_t bit) {
    decodeData = (decodeData << 1) | bit;
    decodeCountBit++;
}

uint64_t HormannDecoder::reverseKey(uint64_t code, uint8_t bitCount) const {
    uint64_t reversed = 0;
    for(uint8_t i = 0; i < bitCount; i++) {
        reversed <<= 1;
        reversed |= (code >> i) & 1ULL;
    }
    return reversed;
}

bool HormannDecoder::checkPattern() const {
    return ((decodeData & HORMANN_HSM_PATTERN) == HORMANN_HSM_PATTERN);
}

void HormannDecoder::feed(bool level, uint32_t duration) {
    switch(state) {
    case StepReset:
        if(level && durationDiff(duration, te_short * 24) < te_delta * 24) {
            state = StepFoundStartBit;
        }
        break;
    case StepFoundStartBit:
        if(!level && durationDiff(duration, te_short) < te_delta) {
            state = StepSaveDuration;
            decodeData = 0;
            decodeCountBit = 0;
        } else {
            state = StepReset;
        }
        break;
    case StepSaveDuration:
        if(level) {
            if(duration >= (te_short * 5) && checkPattern()) {
                state = StepFoundStartBit;
                if(decodeCountBit >= min_count_bit) {
                    finalCode = decodeData;
                    finalBitCount = decodeCountBit;
                    validCodeFound = true;
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
        if(!level) {
            if((durationDiff(te_last, te_short) < te_delta) &&
               (durationDiff(duration, te_long) < te_delta)) {
                addBit(0);
                state = StepSaveDuration;
            } else if((durationDiff(te_last, te_long) < te_delta) &&
                      (durationDiff(duration, te_short) < te_delta)) {
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

bool HormannDecoder::decode(long long int* samples, size_t sampleCount) {
    reset();
    for(size_t i = 0; i < sampleCount; i++) {
        if(samples[i] > 0) {
            feed(true, samples[i]);
        } else {
            feed(false, -samples[i]);
        }
        if(validCodeFound) {
            return true;
        }
    }
    return false;
}

String HormannDecoder::getCodeString() const {
    ScreenManager& screenMgr = ScreenManager::getInstance();

    char buf[256];
    uint32_t high = (uint32_t)(finalCode >> 32);
    uint32_t low  = (uint32_t)(finalCode & 0xFFFFFFFF);
    uint8_t btn = (finalCode >> 8) & 0xF;
    uint64_t reversed = reverseKey(finalCode, finalBitCount);
    uint32_t rev_high = (uint32_t)(reversed >> 32);
    uint32_t rev_low  = (uint32_t)(reversed & 0xFFFFFFFF);
    sprintf(buf, "Hormann HSM\r\n%dbit\r\nKey:0x%03lX%08lX\r\nRev:0x%03lX%08lX\r\nBtn:0x%01X\r\n",
            finalBitCount, high, low, rev_high, rev_low, btn);

    lv_obj_t * textarea;
    if(C1101preset == CUSTOM){
        textarea = screenMgr.text_area_SubGHzCustom;        
    } else {
        textarea = screenMgr.getTextArea();
    }
    Serial.println(buf);
    lv_textarea_set_text(textarea, buf);
    return String(buf);
}

bool HormannDecoder::hasValidCode() const {
    return validCodeFound;
}
