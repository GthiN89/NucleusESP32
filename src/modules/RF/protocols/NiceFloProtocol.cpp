#include "NiceFloProtocol.h"
#include "lvgl.h"
#include "GUI/ScreenManager.h"
#include "globals.h"
#include "math.h"
#include <bitset>


NiceFloProtocol::NiceFloProtocol()
    : te_short(700),
      te_long(1400),
      te_delta(200),
      min_count_bit(12),
      state(StepReset),
      decodeData(0),
      decodeCountBit(0),
      te_last(0),
      validCodeFound(false),
      finalCode(0),
      finalBitCount(0) {
}

void NiceFloProtocol::reset() {
    state = StepReset;
    decodeData = 0;
    decodeCountBit = 0;
    te_last = 0;
    validCodeFound = false;
    finalCode = 0;
    finalBitCount = 0;
}



inline void NiceFloProtocol::addBit(uint8_t bit) {
    decodeData = (decodeData << 1) | bit;
    decodeCountBit++;
    ////Serial.print("addBit: Added bit ");
    ////Serial.print(bit);
    ////Serial.print(" => decodeData: 0x");
    ////Serial.print(decodeData, HEX);
    ////Serial.print(", decodeCountBit: ");
    ////Serial.println(decodeCountBit);
}
void NiceFloProtocol::toBits(unsigned int hexValue) {
    binaryValue = std::bitset<12>(hexValue);
}

void NiceFloProtocol::yield(unsigned int hexValue) {
    switch (encoderState)
    {
    case EncoderStepStart:
        samplesToSend.clear();
        toBits(hexValue);
        encoderState = EncodeStepStartBit;
        break;
    case EncodeStepStartBit:  
        samplesToSend.push_back(700);
        encoderState = EncoderStepDurations;
        break;
    case EncoderStepDurations:
        for (size_t i = 0; i < 12; i++) 
        {
            if (binaryValue[i]) {
                samplesToSend.push_back(1400);
                samplesToSend.push_back(700);
            } else {
                samplesToSend.push_back(1400);
                samplesToSend.push_back(700); 
            }
        }
        samplesToSend.push_back(25200);
        samplesToSend.push_back(700);
                for (size_t i = 0; i < 12; i++) 
        {
            if (binaryValue[i]) {
                samplesToSend.push_back(1400);
                samplesToSend.push_back(700);
            } else {
                samplesToSend.push_back(700);
                samplesToSend.push_back(1400); 
            }
        }
        samplesToSend.push_back(25200);
        samplesToSend.push_back(700);
                for (size_t i = 0; i < 12; i++) 
        {
            if (binaryValue[i]) {
                samplesToSend.push_back(1400);
                samplesToSend.push_back(700);
            } else {
                samplesToSend.push_back(700);
                samplesToSend.push_back(1400); 
            }
        }
        samplesToSend.push_back(25200);

        // for (size_t i = 0; i < samplesToSend.size(); i++)
        // {
        //     ////Serial.println(samplesToSend[i]);
        // }
            delay(5);
            encoderState = EncoderStepReady;

        break;
    default:
            //
        break;
    }
}


void NiceFloProtocol::feed(bool level, uint32_t duration) {
    switch(state) {
    case StepReset:
        if(!level && DURATION_DIFF(duration, te_short * 36) < te_delta * 36) {
            Serial.println(F("Header detected"));
            state = StepFoundStartBit;
        } else {
           //Serial.println(F("No header condition met in StepReset"));
        }
        break;
    case StepFoundStartBit:
       //Serial.println(F("State: StepFoundStartBit"));
        if(!level) {
           //Serial.println(F("Ignoring falling edge in StepFoundStartBit"));
            break;
        } else if(DURATION_DIFF(duration, te_short) < te_delta) {
           //Serial.println(F("Start bit detected, switching to StepSaveDuration"));
            state = StepSaveDuration;
            decodeData = 0;
            decodeCountBit = 0;
        } else {
           //Serial.println(F("Start bit not matched, resetting state"));
            state = StepReset;
        }
        break;
    case StepSaveDuration:
       //Serial.println(F("State: StepSaveDuration"));
        if(!level) { // falling edge: low duration
            if(duration >= (te_short * 4)) {
               //Serial.println(F("Long low duration detected in StepSaveDuration, possible end of data"));
                state = StepFoundStartBit;
                if(decodeCountBit >= min_count_bit) {
                    //Serial.print(F("Valid code detected: decodeCountBit = "));
                    //Serial.print(decodeCountBit);
                    //Serial.print(F(", decodeData = 0x"));
                   //Serial.println(decodeData, HEX);
                    validCodeFound = true;
                    finalCode = decodeData;
                    finalBitCount = decodeCountBit;
                }
                break;
            }
            te_last = duration;
            state = StepCheckDuration;
        } else {
           //Serial.println(F("Unexpected HIGH in StepSaveDuration, resetting state"));
            state = StepReset;
        }
        break;
    case StepCheckDuration:
       //Serial.println(F("State: StepCheckDuration"));
        if(level) {
            if((DURATION_DIFF(te_last, te_short) < te_delta) &&
               (DURATION_DIFF(duration, te_long) < te_delta)) {
               //Serial.println(F("Detected bit 0 in StepCheckDuration"));
                addBit(0);
                state = StepSaveDuration;
            } else if((DURATION_DIFF(te_last, te_long) < te_delta) &&
                      (DURATION_DIFF(duration, te_short) < te_delta)) {
               //Serial.println(F("Detected bit 1 in StepCheckDuration"));
                addBit(1);
                state = StepSaveDuration;
            } else {
               //Serial.println(F("Duration mismatch in StepCheckDuration, resetting state"));
                state = StepReset;
            }
        } else {
           //Serial.println(F("Expected HIGH in StepCheckDuration but got LOW, resetting state"));
            state = StepReset;
        }
        break;
    default:
       //Serial.println(F("Unknown state, resetting"));
        state = StepReset;
        break;
    }
}



bool NiceFloProtocol::decode(long long int* samples, size_t sampleCount) {
    reset();
    for(size_t i = 0; i < sampleCount; i++) {
        //Serial.print("Sample index ");
        //Serial.print(i);
        //Serial.print(": ");
       //Serial.println(samples[i]);
        
        if(samples[i] > 0) {
            feed(true, samples[i]);
        } else {
            feed(false, -samples[i]);
        }
        
        if(validCodeFound) {
           //Serial.println(F("Valid code found, exiting decode loop"));
            return true;
        }
    }
    return false;
}

uint32_t NiceFloProtocol::reverseKey(uint32_t code, uint8_t bitCount) const {
    uint32_t reversed = 0;
    for(uint8_t i = 0; i < bitCount; i++) {
        reversed <<= 1;
        reversed |= (code >> i) & 1;
    }
    return reversed;
}

String NiceFloProtocol::getCodeString() const {
    char buf[128];
    uint32_t codeFound = finalCode;
    uint32_t codeReversed = reverseKey(finalCode, finalBitCount);
    const char* protocolName = "\nNiceFlo";
    sprintf(buf, "%s %dbit\r\nKey:0x%08lX\r\nYek:0x%08lX\r\n",
            protocolName, finalBitCount, codeFound, codeReversed);
   //Serial.println("getCodeString:");
   //Serial.println(buf);

   ScreenManager& screenMgr = ScreenManager::getInstance();
             lv_obj_t * textarea;
    if(C1101preset == CUSTOM){
        textarea = screenMgr.text_area_SubGHzCustom;        
    } else {
        textarea = screenMgr.getTextArea();
    }

    lv_textarea_set_text(textarea, buf); 

    return String(buf);
}

bool NiceFloProtocol::hasValidCode() const {
    //Serial.print("hasValidCode: ");
   //Serial.println(validCodeFound ? "true" : "false");
    return validCodeFound;
}
