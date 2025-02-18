#include "HormannProtocol.h"
#include "lvgl.h"
#include "GUI/ScreenManager.h"
#include "globals.h"

HormannProtocol::HormannProtocol()
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
      finalBitCount(0),
      hormanEncoderState(HormanEncoderStepStart),
      bitCount(44)
     {
}

void HormannProtocol::reset() {
    state = StepReset;
    decodeData = 0;
    decodeCountBit = 0;
    te_last = 0;
    validCodeFound = false;
    finalCode = 0;
    finalBitCount = 0;
}


void HormannProtocol::addBit(uint8_t bit) {
    decodeData = (decodeData << 1) | bit;
    decodeCountBit++;
}

uint64_t HormannProtocol::reverseKey(uint64_t code, uint8_t bitCount) const {
    uint64_t reversed = 0;
    for(uint8_t i = 0; i < bitCount; i++) {
        reversed <<= 1;
        reversed |= (code >> i) & 1ULL;
    }
    return reversed;
}

bool HormannProtocol::checkPattern() const {
    return ((decodeData & HORMANN_HSM_PATTERN) == HORMANN_HSM_PATTERN);
}

void HormannProtocol::yield(uint64_t hexValue) {
    switch (hormanEncoderState) {
    case HormanEncoderStepStart:
        samplesToSend.clear();
        // Convert hexValue to a bitset using 'bitCount' bits (LSB first)
        for (uint8_t i = 0; i < bitCount; i++) {
            binaryValue[i] = (hexValue >> i) & 1ULL;
        }
        hormanEncoderState = HormanEncoderStepStartBit;
        break;
    case HormanEncoderStepStartBit:
        // Send start high pulse: duration = te_short * 24 (e.g., 500*24 = 12000)
        samplesToSend.push_back(te_short * 24);
        hormanEncoderState = HormanEncoderStepLowStart;
        break;
    case HormanEncoderStepLowStart:
        // Send low pulse: duration = te_short (500)
        samplesToSend.push_back(te_short);
        hormanEncoderState = HormanEncoderStepDurations;
        break;
    case HormanEncoderStepDurations:
        for (size_t i = 0; i < bitCount; i++) {
            if (binaryValue[i]) {
                samplesToSend.push_back(te_long);
                samplesToSend.push_back(te_short);
            } else {
                samplesToSend.push_back(te_short);
                samplesToSend.push_back(te_long);
            }
        }
        samplesToSend.push_back(te_short * 5);
        for (size_t i = 0; i < samplesToSend.size(); i++) {
            Serial.println(samplesToSend[i]);
        }
        hormanEncoderState = HormanEncoderStepReady;
        break;
    default:
        break;
    }
}


void HormannProtocol::feed(bool level, uint32_t duration) {
    switch(state) {
    case StepReset:
        if(level && DURATION_DIFF(duration, te_short * 24) < te_delta * 24) {
            state = StepFoundStartBit;
        }
        break;
    case StepFoundStartBit:
        if(!level && DURATION_DIFF(duration, te_short) < te_delta) {
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
            if((DURATION_DIFF(te_last, te_short) < te_delta) &&
               (DURATION_DIFF(duration, te_long) < te_delta)) {
                addBit(0);
                state = StepSaveDuration;
            } else if((DURATION_DIFF(te_last, te_long) < te_delta) &&
                      (DURATION_DIFF(duration, te_short) < te_delta)) {
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

bool HormannProtocol::decode(long long int* samples, size_t sampleCount) {
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

String HormannProtocol::getCodeString() const {
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
    //Serial.println(buf);
    lv_textarea_set_text(textarea, buf);
    return String(buf);
}

bool HormannProtocol::hasValidCode() const {
    return validCodeFound;
}
