 #include "AnsonicProtocol.h"
 #include <stdio.h>
 #include "globals.h"
 #include "math.h"

 #define DURATION_DIFF(x, y) (((x) < (y)) ? ((y) - (x)) : ((x) - (y)))

 
 #define DIP_PATTERN "%c%c%c%c%c%c%c%c%c%c"
 #define CNT_TO_DIP(dip) \
     (dip & 0x0800 ? '1' : '0'), (dip & 0x0400 ? '1' : '0'), (dip & 0x0200 ? '1' : '0'), \
     (dip & 0x0100 ? '1' : '0'), (dip & 0x0080 ? '1' : '0'), (dip & 0x0040 ? '1' : '0'), \
     (dip & 0x0020 ? '1' : '0'), (dip & 0x0010 ? '1' : '0'), (dip & 0x0001 ? '1' : '0'), \
     (dip & 0x0008 ? '1' : '0')
 

 AnsonicProtocol::AnsonicProtocol()
     : te_short(555),
       te_long(1111),
       te_delta(120),
        space(19425),
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
       binaryValue(0),
       preset(AM650) {
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
     encoderState = EncoderStepStart;
     samplesToSend.clear();
 }
 
 inline void AnsonicProtocol::addBit(uint8_t bit) {
     decodeData = decodeData << 1 | bit;
     decodeCountBit++;
 }
 
 void AnsonicProtocol::toBits(unsigned int hexValue) {
     binaryValue = std::bitset<12>(hexValue);
 }
 
 void AnsonicProtocol::yield(unsigned int hexValue) {

    samplesToSend.clear();
    toBits(hexValue);

    samplesToSend.push_back(te_short);
    samplesToSend.push_back(te_short);
    samplesToSend.push_back(te_long);
    samplesToSend.push_back(te_long);

    samplesToSend.push_back(space);  
    
    samplesToSend.push_back(te_short);       
    
    for(uint8_t i = 0; i < min_count_bit; i++) {
        if(binaryValue[i] == 1) {
            // bit = 1
            samplesToSend.push_back(te_long);
            samplesToSend.push_back(te_short);
        } else {
            // bit = 0
            samplesToSend.push_back(te_short);  
            samplesToSend.push_back(te_long); 
        }
    }

        delay(5);

}

void AnsonicProtocol::feed(bool level, uint32_t duration) {
    switch(DecoderState) {
        case DecoderStepReset:
            if((!level) && (DURATION_DIFF(duration, te_short * 35) < te_delta * 35)) {
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
                if(duration >= (te_short * 4)) {
                    DecoderState = DecoderStepFoundStartBit;
                    if(decodeCountBit >= 12) {
                        serial = 0x0;
                        btn = 0x0;
                        DecoderState = DecoderStepFound;
                        validCodeFound = true;
                        finalCode = decodeData;
                        finalBitCount = decodeCountBit;
                    }
                } else {
                    te_last = duration;
                    DecoderState = DecoderStepCheckDuration;
                }
            } else {
                DecoderState = DecoderStepReset;
            }
            break;

        case DecoderStepCheckDuration:
            if(level) {
                if((DURATION_DIFF(te_last, te_short) < te_delta) &&
                   (DURATION_DIFF(duration, te_long) < te_delta)) {
                    addBit(1);
                    DecoderState = DecoderStepSaveDuration;
                } else if((DURATION_DIFF(te_last, te_long) < te_delta) &&
                          (DURATION_DIFF(duration, te_short) < te_delta)) {
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
            feed(true, (uint32_t)samples[i]);
        } else {
            feed(false, (uint32_t)(-samples[i]));
        }
        if(validCodeFound) {
            Serial.println(F("Valid code found"));
            return true;
        }
    }
    Serial.println(F("No valid code detected"));
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

void AnsonicProtocol::checkRemoteController() {

    finalDip = (finalCode & 0x0FFF);

    finalBtn = ((finalCode >> 1) & 0x3);
}


String AnsonicProtocol::getCodeString() const {
    uint16_t localDip = (finalCode & 0x0FFF);
    uint8_t  localBtn = ((finalCode >> 1) & 0x3);

    ScreenManager& screenMgr = ScreenManager::getInstance();
    lv_obj_t* textarea;
    if(C1101preset == CUSTOM) {
        textarea = screenMgr.text_area_SubGHzCustom;
    } else {
        textarea = screenMgr.getTextArea();
    }

    char buf[128];
    sprintf(
        buf,
        "Ansonic %dbit\r\n"
        "Key:%03lX\r\n"
        "Btn:%X\r\n"
        "DIP:" DIP_PATTERN "\r\n",
        finalBitCount,
        (finalCode & 0xFFF),  // 12-bit portion
        localBtn,
        (localDip & 0x0800 ? '1' : '0'),
        (localDip & 0x0400 ? '1' : '0'),
        (localDip & 0x0200 ? '1' : '0'),
        (localDip & 0x0100 ? '1' : '0'),
        (localDip & 0x0080 ? '1' : '0'),
        (localDip & 0x0040 ? '1' : '0'),
        (localDip & 0x0020 ? '1' : '0'),
        (localDip & 0x0010 ? '1' : '0'),
        (localDip & 0x0001 ? '1' : '0'),
        (localDip & 0x0008 ? '1' : '0')
    );

    lv_textarea_set_text(textarea, buf);

    // Debug prints
    Serial.println(F("AnsonicProtocol::getCodeString() called"));
    Serial.print(F(" finalBitCount: "));
    Serial.println(finalBitCount);
    Serial.print(F(" finalCode: 0x"));
    Serial.println(finalCode, HEX);
    Serial.print(F(" finalDip (local): 0x"));
    Serial.println(localDip, HEX);
    Serial.print(F(" finalBtn (local): 0x"));
    Serial.println(localBtn, HEX);

    return String(buf);
}