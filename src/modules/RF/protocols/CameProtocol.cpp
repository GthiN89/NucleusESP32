#include "CameProtocol.h"
#include <stdio.h>
#include "lvgl.h"
#include "GUI/ScreenManager.h"
#include "globals.h"


CameProtocol::CameProtocol() 
    : te_short(320),
      te_long(640),
      te_delta(150),
      min_count_bit(12),
      binaryValue(0),
      DecoderState(DecoderStepReset),
      decodeData(0),
      decodeCountBit(0),
      te_last(0),
      validCodeFound(false),
      finalCode(0),
      finalBitCount(0) 
{
    binaryValue = std::bitset<12>(0);
    encoderState = EncoderStepIddle;
}


void CameProtocol::reset() {
    DecoderState = DecoderStepReset;
    decodeData = 0;
    decodeCountBit = 0;
    te_last = 0;
    validCodeFound = false;
    finalCode = 0;
    finalBitCount = 0;
}



inline void CameProtocol::addBit(uint8_t bit) {
    decodeData = decodeData << 1 | bit;
    decodeCountBit++;
}



void CameProtocol::toBits(unsigned int hexValue) {
    binaryValue = std::bitset<12>(hexValue);
}


void CameProtocol::yield(unsigned int hexValue) {

        samplesToSend.clear();
        toBits(hexValue);


    samplesToSend.push_back(320);
    samplesToSend.push_back(320);
    samplesToSend.push_back(640);
    samplesToSend.push_back(640);
    samplesToSend.push_back(11520);
        samplesToSend.push_back(320);
        for (size_t i = 0; i < 12; i++) 
        {
            if (binaryValue[i]) {
                samplesToSend.push_back(640);
                samplesToSend.push_back(320);
            } else {
                samplesToSend.push_back(320);
                samplesToSend.push_back(640); 
            }
        }
       
        

            delay(5);

}

void CameProtocol::feed(bool level, uint32_t duration) {    
    switch(DecoderState) {
    case DecoderStepReset:
        if(!level && DURATION_DIFF(duration, te_short * 56) < te_delta * 52) {
            DecoderState = DecoderStepFoundStartBit;
        }
        break;

    case DecoderStepFoundStartBit:
        if(!level) {
            break;
        } else if(DURATION_DIFF(duration, te_short) < te_delta) {
            DecoderState = DecoderStepSaveDuration;
            decodeData = 0;
            decodeCountBit = 0;
        } else {
            DecoderState = DecoderStepReset;
        }
        break;

    case DecoderStepSaveDuration:
        if(!level) {
            if(duration > 5000) {
                DecoderState = DecoderStepFoundStartBit;
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
            DecoderState = DecoderStepCheckDuration;
        } else {
            DecoderState = DecoderStepReset;
        }
        break;

    case DecoderStepCheckDuration:
        if(level) {
            if((DURATION_DIFF(te_last, te_short) <
                te_delta) &&
               (DURATION_DIFF(duration, te_long) <
                te_delta)) {
                addBit(0);
                DecoderState = DecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(te_last, te_long) <
                 te_delta) &&
                (DURATION_DIFF(duration, te_short) <
                 te_delta)) {
                addBit(1);
                DecoderState = DecoderStepSaveDuration;
            } else {
                DecoderState = DecoderStepReset;
            }
        } else {
            DecoderState = DecoderStepReset;
        }
        break;

    default:
        DecoderState = DecoderStepReset;
        break;
    }
}

bool CameProtocol::decode(long long int* samples, size_t sampleCount) {
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

uint32_t CameProtocol::reverseKey(uint32_t code, uint8_t bitCount) const {
    uint32_t reversed = 0;
    for(uint8_t i = 0; i < bitCount; i++) {
        reversed <<= 1;
        reversed |= (code >> i) & 1;
    }
    return reversed;
}

String CameProtocol::getCodeString(uint64_t shortPulse, uint64_t longPulse) const {
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
            protocolName, finalBitCount, codeFound, codeReversed,
            "Short pulse:",
            shortPulse,
            "\n",
            "Short pulse:",
            longPulse,
            "\n"
        );

    lv_obj_t * textarea;
    if(C1101preset == CUSTOM){
        textarea = screenMgr.text_area_SubGHzCustom;        
    } else {
        textarea = screenMgr.getTextArea();
    }

    lv_textarea_set_text(textarea, buf);        
    return String(buf);
}

bool CameProtocol::hasValidCode() const {
    return validCodeFound;
}
