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
    //Serial.println("HormannProtocol: Constructor called");
}

void HormannProtocol::reset() {
    //Serial.println("HormannProtocol: Resetting state");
    state = StepReset;
    decodeData = 0;
    decodeCountBit = 0;
    te_last = 0;
    validCodeFound = false;
    finalCode = 0;
    finalBitCount = 0;
}

void HormannProtocol::addBit(uint8_t bit) {
    //Serial.print("HormannProtocol: Adding bit: ");
    //Serial.println(bit);
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
    bool patternOk = ((decodeData & HORMANN_HSM_PATTERN) == HORMANN_HSM_PATTERN);
    //Serial.print("HormannProtocol: checkPattern = ");
    //Serial.println(patternOk);
    return patternOk;
}

void HormannProtocol::yield(uint64_t hexValue) {
    //Serial.print("HormannProtocol: Yielding encoding for hex value: 0x");
    //Serial.println(hexValue, HEX);
    switch (hormanEncoderState) {
    case HormanEncoderStepStart:
        samplesToSend.clear();
        // Convert hexValue to a bitset using 'bitCount' bits (LSB first)
        for (uint8_t i = 0; i < bitCount; i++) {
            binaryValue[i] = (hexValue >> i) & 1ULL;
        }
        //Serial.println("Encoder state: HormanEncoderStepStart");
        hormanEncoderState = HormanEncoderStepStartBit;
        break;
    case HormanEncoderStepStartBit:
        samplesToSend.push_back(te_short * 24);
        //Serial.println("Encoder state: HormanEncoderStepStartBit, start high pulse added");
        hormanEncoderState = HormanEncoderStepLowStart;
        break;
    case HormanEncoderStepLowStart:
        samplesToSend.push_back(te_short);
        //Serial.println("Encoder state: HormanEncoderStepLowStart, low pulse added");
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
        //Serial.println("Encoder state: HormanEncoderStepDurations, all pulses added:");
        for (size_t i = 0; i < samplesToSend.size(); i++) {
            //Serial.println(samplesToSend[i]);
        }
        hormanEncoderState = HormanEncoderStepReady;
        break;
    default:
        //Serial.println("Encoder state: Unknown branch");
        break;
    }
}

void HormannProtocol::feed(bool level, uint32_t duration) {
    //Serial.print("Feed: level=");
    //Serial.print(level);
    //Serial.print(", duration=");
    //Serial.println(duration);
    
    switch(state) {
    case StepReset:
        //Serial.println("State: StepReset");
        if(level && DURATION_DIFF(duration, te_short * 24) < te_delta * 24) {
            //Serial.println("Start bit detected, switching to StepFoundStartBit");
            state = StepFoundStartBit;
        }
        break;
    case StepFoundStartBit:
        //Serial.println("State: StepFoundStartBit");
        if(!level && DURATION_DIFF(duration, te_short) < te_delta) {
            //Serial.println("Valid low pulse detected, switching to StepSaveDuration");
            state = StepSaveDuration;
            decodeData = 0;
            decodeCountBit = 0;
        } else {
            //Serial.println("Invalid pulse in StepFoundStartBit, resetting");
            state = StepReset;
        }
        break;
    case StepSaveDuration:
        //Serial.println("State: StepSaveDuration");
        if(level) {
            if(duration >= (te_short * 5) && checkPattern()) {
                //Serial.println("Boundary and pattern detected, finishing frame");
                state = StepFoundStartBit;
                if(decodeCountBit >= min_count_bit) {
                    finalCode = decodeData;
                    finalBitCount = decodeCountBit;
                    validCodeFound = true;
                    //Serial.print("Valid code found: 0x");
                    //Serial.println(finalCode, HEX);
                }
                break;
            }
            te_last = duration;
            //Serial.print("Recording high pulse: ");
            //Serial.println(te_last);
            state = StepCheckDuration;
        } else {
            //Serial.println("Unexpected low pulse in StepSaveDuration, resetting");
            state = StepReset;
        }
        break;
    case StepCheckDuration:
        //Serial.println("State: StepCheckDuration");
        if(!level) {
            if((DURATION_DIFF(te_last, te_short) < te_delta) &&
               (DURATION_DIFF(duration, te_long) < te_delta)) {
                //Serial.println("Bit 0 detected");
                addBit(0);
                state = StepSaveDuration;
            } else if((DURATION_DIFF(te_last, te_long) < te_delta) &&
                      (DURATION_DIFF(duration, te_short) < te_delta)) {
                //Serial.println("Bit 1 detected");
                addBit(1);
                state = StepSaveDuration;
            } else {
                //Serial.println("Invalid pulse durations in StepCheckDuration, resetting");
                state = StepReset;
            }
        } else {
            //Serial.println("Unexpected rising edge in StepCheckDuration, resetting");
            state = StepReset;
        }
        break;
    default:
        //Serial.println("Default state reached in feed, resetting");
        state = StepReset;
        break;
    }
}

bool HormannProtocol::decode(long long int* samples, size_t sampleCount) {
    //Serial.print("HormannProtocol: Decoding ");
    //Serial.print(sampleCount);
    //Serial.println(" samples");
    reset();
    for(size_t i = 0; i < sampleCount; i++) {
        if(samples[i] > 0) {
            feed(true, samples[i]);
        } else {
            feed(false, -samples[i]);
        }
        if(validCodeFound) {
            //Serial.println("HormannProtocol: Valid code found, exiting decode loop");
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
    //Serial.println("HormannProtocol: getCodeString generated:");
    //Serial.println(buf);
    lv_textarea_set_text(textarea, buf);
    return String(buf);
}

bool HormannProtocol::hasValidCode() const {
    return validCodeFound;
}
