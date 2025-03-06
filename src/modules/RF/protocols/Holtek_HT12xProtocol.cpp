#include "Holtek_HT12xProtocol.h"
#include <stdio.h>

static inline uint32_t duration_diff(uint32_t a, uint32_t b) {
    return (a > b) ? (a - b) : (b - a);
}

HoltekProtocol::HoltekProtocol()
    : preset(AM650),
      decoderState(DecoderStepReset),
      te_short(400),
      te_long(800),
      te_delta(200),
      space(14400),
      min_count_bit(12),
      te(0),
      validCodeFound(false),
      decodeCountBit(0),
      decodedData(0),
      te_last(0),
      lastData(0),
      finalBtn(0),
      finalDIP(0),
      finalCode(0),
      finalBitCount(0) {
}

void HoltekProtocol::reset() {
    decoderState = DecoderStepReset;
    decodedData = 0;
    decodeCountBit = 0;
    te_last = 0;
    lastData = 0;
    validCodeFound = false;
    finalBtn = 0;
    finalDIP = 0;
    finalCode = 0;
    finalBitCount = 0;

    encoderState = EncoderStepStart;
    te = 0;
    samplesToSend.clear();
}

inline void HoltekProtocol::addBit(uint8_t bit) {
    decodedData = (decodedData << 1) | bit;
    decodeCountBit++;
}

void HoltekProtocol::yield(unsigned int hexValue) {
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

void HoltekProtocol::toBits(unsigned int hexValue) {
    binaryValue = std::bitset<12>(hexValue);
}


void HoltekProtocol::feed(bool level, uint32_t duration) {
    switch(decoderState) {
        case DecoderStepReset:
            if(!level && (duration_diff(duration, te_short * 36) < (te_delta * 36))) {
                decoderState = DecoderStepFoundStartBit;
            }
            break;

        case DecoderStepFoundStartBit:
            if(level && (duration_diff(duration, te_short) < te_delta)) {
                decodedData = 0;
                decodeCountBit = 0;
                te_last = duration; 
                decoderState = DecoderStepSaveDuration;
            } else {
                decoderState = DecoderStepReset;
            }
            break;

        case DecoderStepSaveDuration:
            if(!level) {
                if(duration >= (te_short * 10 + te_delta)) {
                    if(decodeCountBit == min_count_bit) {
                        if((lastData == decodedData) && (lastData != 0)) {
                            finalCode = decodedData;
                            finalBitCount = decodeCountBit;
                            validCodeFound = true;
                        }
                        lastData = decodedData;
                    }
                    decodedData = 0;
                    decodeCountBit = 0;
                    te_last = 0;
                    decoderState = DecoderStepFoundStartBit;
                } else {
                    te_last = duration;
                    decoderState = DecoderStepCheckDuration;
                }
            } else {
                decoderState = DecoderStepReset;
            }
            break;

        case DecoderStepCheckDuration:
            if(level) {
                if((duration_diff(te_last, te_long) < (te_delta * 2)) &&
                   (duration_diff(duration,  te_short) < te_delta)) {
                    addBit(1);
                    decoderState = DecoderStepSaveDuration;
                }
                else if((duration_diff(te_last, te_short) < te_delta) &&
                        (duration_diff(duration,  te_long) < (te_delta * 2))) {
                    addBit(0);
                    decoderState = DecoderStepSaveDuration;
                } else {
                    decoderState = DecoderStepReset;
                }
            } else {
                decoderState = DecoderStepReset;
            }
            break;

        case DecoderStepFound:
            break;
    }
}

bool HoltekProtocol::decode(long long int* samples, size_t sampleCount) {
    reset();
    for(size_t i = 0; i < sampleCount; i++) {
        if(samples[i] > 0) {
            feed(true,  (uint32_t)samples[i]);
        } else {
            feed(false, (uint32_t)(-samples[i]));
        }
        if(validCodeFound) {
            Serial.println(F("Holtek: valid code found"));
            return true;
        }
    }
    Serial.println(F("Holtek: no valid code detected"));
    return false;
}

String HoltekProtocol::getCodeString() const {

    ScreenManager& screenMgr = ScreenManager::getInstance();
    lv_obj_t* textarea;
    if(C1101preset == CUSTOM) {
        textarea = screenMgr.text_area_SubGHzCustom;
    } else {
        textarea = screenMgr.getTextArea();
    }


    char buf[128];

    snprintf(
        buf,
        sizeof(buf),
        "Holtek HT12X %db\r\nKey:0x%03lX\r\nBtn: ",
        finalBitCount,
        (finalCode & 0xFFF)
    );

    char btnText[32];
    snprintf(
        btnText,
        sizeof(btnText),
        "%s%s%s%s\r\n",
        (((finalBtn >> 3) & 0x1) == 0) ? "B1 " : "",
        (((finalBtn >> 2) & 0x1) == 0) ? "B2 " : "",
        (((finalBtn >> 1) & 0x1) == 0) ? "B3 " : "",
        (((finalBtn >> 0) & 0x1) == 0) ? "B4 " : ""
    );
    strlcat(buf, btnText, sizeof(buf));


    char line2[64];
    snprintf(
        line2,
        sizeof(line2),
        "DIP:" DIP_PATTERN "\r\nTe:%luus\r\n",
        (finalDIP & 0x80 ? '0' : '1'),
        (finalDIP & 0x40 ? '0' : '1'),
        (finalDIP & 0x20 ? '0' : '1'),
        (finalDIP & 0x10 ? '0' : '1'),
        (finalDIP & 0x08 ? '0' : '1'),
        (finalDIP & 0x04 ? '0' : '1'),
        (finalDIP & 0x02 ? '0' : '1'),
        (finalDIP & 0x01 ? '0' : '1'),
        (unsigned long)te
    );
    strlcat(buf, line2, sizeof(buf));

    lv_textarea_set_text(textarea, buf);


    Serial.println(F("HoltekProtocol::getCodeString() called"));
    Serial.print(F(" finalBitCount: "));
    Serial.println(finalBitCount);
    Serial.print(F(" finalCode: 0x"));
    Serial.println(finalCode, HEX);
    Serial.print(F(" finalBtn: 0x"));
    Serial.println(finalBtn, HEX);
    Serial.print(F(" finalDIP: 0x"));
    Serial.println(finalDIP, HEX);
    Serial.print(F(" te: "));
    Serial.println(te);

    return String(buf);
}


bool HoltekProtocol::hasValidCode() const {
    return validCodeFound;
}
