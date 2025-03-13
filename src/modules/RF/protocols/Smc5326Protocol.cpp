#include "Smc5326Protocol.h"
#include <stdio.h>
#

static inline uint32_t duration_diff(uint32_t a, uint32_t b) {
    return (a > b) ? (a - b) : (b - a);
}

SMC5326Protocol::SMC5326Protocol()
    : decoderState(DecoderStepReset),
      te_short(300),
      te_long(900),
      te_delta(200),
      min_count_bit(25),
      validCodeFound(false),
      decodeCountBit(0),
      decodedData(0),
      te_last(0),
      lastData(0),
      finalCode(0),
      finalBitCount(0),
      finalBtn(0),
      finalDIP(0),
      te(0),
      preset(AM650) {
}

void SMC5326Protocol::reset() {
    decoderState = DecoderStepReset;
    decodedData = 0;
    decodeCountBit = 0;
    te_last = 0;
    lastData = 0;
    validCodeFound = false;
    finalCode = 0;
    finalBitCount = 0;
    finalBtn = 0;
    finalDIP = 0;
    te = 0;
}

inline void SMC5326Protocol::addBit(uint8_t bit) {
    decodedData = (decodedData << 1) | (bit & 1);
    decodeCountBit++;
}

bool SMC5326Protocol::decodeReversed(long long int* samples, size_t sampleCount) {
    reset();
    int j = 0;
    std::vector<long long int> samplesReversed(sampleCount);
    for (int i = sampleCount ; i > 0; i--) {
        samplesReversed[j] = samples[i];
        j++;
    }
    decode(samplesReversed.data(), sampleCount);
    return false;
}

void SMC5326Protocol::feed(bool level, uint32_t duration) {
    switch(decoderState) {
        case DecoderStepReset:
            //Serial.println(F("State: DecoderStepReset"));
            if (!level && (duration_diff(duration, te_short * 24) < (te_delta * 12))) {
                //Serial.println(F("Preamble detected, switching to SaveDuration"));
                decoderState = DecoderStepSaveDuration;
                decodedData = 0;
                decodeCountBit = 0;
                te = 0;
            }
            break;

        case DecoderStepSaveDuration:
            //Serial.println(F("State: DecoderStepSaveDuration"));
            if (level) {
                te_last = duration;
                te += duration;
                decoderState = DecoderStepCheckDuration;
                //Serial.print(F("Captured HIGH duration: "));
                //Serial.println(duration);
            } else {
                //Serial.println(F("Expected HIGH but got LOW. Resetting state."));
                decoderState = DecoderStepReset;
            }
            break;

        case DecoderStepCheckDuration:
            //Serial.println(F("State: DecoderStepCheckDuration"));
            if (!level) {
                if (duration >= (te_long * 2)) {
                    //Serial.print(F("Long LOW detected: "));
                    //Serial.println(duration);
                    decoderState = DecoderStepSaveDuration;
                    if (decodeCountBit == min_count_bit) {
                        if ((lastData == decodedData) && (lastData != 0)) {
                            te = te / (decodeCountBit * 4 + 1);
                            finalCode = decodedData;
                            finalBitCount = decodeCountBit;
                            validCodeFound = true;
                            //Serial.println(F("Valid code found!"));
                        }
                        lastData = decodedData;
                    }
                    decodedData = 0;
                    decodeCountBit = 0;
                    te = 0;
                    //Serial.println(F("End of frame detected, resetting decode state."));
                    break;
                }

                te += duration;
                if ((duration_diff(te_last, te_short) < te_delta) &&
                    (duration_diff(duration, te_long) < (te_delta * 3))) {
                    addBit(0);
                    decoderState = DecoderStepSaveDuration;
                    //Serial.println(F("Decoded bit: 0"));
                } else if ((duration_diff(te_last, te_long) < (te_delta * 3)) &&
                           (duration_diff(duration, te_short) < te_delta)) {
                    addBit(1);
                    decoderState = DecoderStepSaveDuration;
                    //Serial.println(F("Decoded bit: 1"));
                } else {
                    //Serial.println(F("Timing mismatch, resetting decode state."));
                    decoderState = DecoderStepReset;
                }
            } else {
                //Serial.println(F("Expected LOW but got HIGH, resetting decode state."));
                decoderState = DecoderStepReset;
            }
            break;
    }
}


bool SMC5326Protocol::decode(long long int* samples, size_t sampleCount) {
    reset();
    for (size_t i = 0; i < sampleCount; i++) {
        if (samples[i] > 0) {
            feed(true, (uint32_t)samples[i]);
        } else {
            feed(false, (uint32_t)(-samples[i]));
        }
        if (validCodeFound) {
            //Serial.println(F("SMC5326: valid code found"));
            return true;
        }
    }
    //Serial.println(F("SMC5326: no valid code detected"));
    return false;
}

void SMC5326Protocol::yield(unsigned int code) {
    samplesToSend.clear();
    

    //Serial.print(F("Expected size: "));
    //Serial.println((min_count_bit * 2) + 2);
    
    for (uint8_t i = min_count_bit; i > 0; i--) {
        bool bitIsOne = (code & (1 << (i - 1))) != 0;
        //Serial.print(F("Bit "));
        //Serial.print(i - 1);
        //Serial.print(F(": "));
        //Serial.println(bitIsOne ? F("1") : F("0"));
        if (bitIsOne) {
            // For bit '1': send HIGH pulse for te*3, then LOW pulse for te.
            samplesToSend.push_back(te_short * 3);
            samplesToSend.push_back(te_short);
        } else {
            // For bit '0': send HIGH pulse for te, then LOW pulse for te*3.
            samplesToSend.push_back(te_short);
            samplesToSend.push_back(te_short * 3);
        }
    }
    
    // Send Stop bit: HIGH pulse for te, then PT_GUARD: LOW pulse for te*25.
    //Serial.println(F("Sending stop bit and PT_GUARD"));
    samplesToSend.push_back(te_short);
    samplesToSend.push_back(te_short * 25);
    samplesToSend.push_back(1000);
    
    //Serial.print(F("Yield complete, pulse sequence: "));
    for (size_t i = 0; i < samplesToSend.size(); i++) {
        //Serial.print(samplesToSend[i]);
        //Serial.print(F(" "));
    }
    //Serial.println();
    delay(5);
}


String SMC5326Protocol::getCodeString(uint64_t shortPulse, uint64_t longPulse) const {
    uint32_t data = (finalCode >> 9) & 0xFFFF;
    uint8_t event = (finalCode >> 1) & 0xFF;

    ScreenManager& screenMgr = ScreenManager::getInstance();
    lv_obj_t* textarea = (C1101preset == CUSTOM) ? screenMgr.text_area_SubGHzCustom : screenMgr.getTextArea();

    char buf[192];
    snprintf(
        buf,
        sizeof(buf),
        "SMC5326 %ubit\r\nKey:%07lX         Te:%luus\r\n"
        "  +:   " DIP_PATTERN "\r\n"
        "  o:   " DIP_PATTERN "    ",
        finalBitCount,
        (finalCode & 0x1FFFFFF),
        (unsigned long)te,
        SHOW_DIP_P(data, DIP_P),
        SHOW_DIP_P(data, DIP_O)
    );
    char btnLine[32];
    snprintf(
        btnLine,
        sizeof(btnLine),
        "%s%s%s%s\r\n",
        (((event >> 6) & 0x3) == 0x3 ? "B1 " : ""),
        (((event >> 4) & 0x3) == 0x3 ? "B2 " : ""),
        (((event >> 2) & 0x3) == 0x3 ? "B3 " : ""),
        (((event >> 0) & 0x3) == 0x3 ? "B4 " : "")
    );
    strlcat(buf, btnLine, sizeof(buf));
    char minusLine[48];
    snprintf(
        minusLine,
        sizeof(minusLine),
        "  -:   " DIP_PATTERN "\r\n",
        SHOW_DIP_P(data, DIP_N)
    );
    strlcat(buf, minusLine, sizeof(buf));

    lv_textarea_set_text(textarea, buf);

    //Serial.println(F("SMC5326Protocol::getCodeString(uint64_t shortPulse, uint64_t longPulse) called"));
    //Serial.print(F(" finalCode=0x"));
    //Serial.println(finalCode, HEX);
    //Serial.print(F(" finalBitCount="));
    //Serial.println(finalBitCount);
    //Serial.print(F(" te="));
    //Serial.println(te);

    return String(buf);
}

bool SMC5326Protocol::hasValidCode() const {
    return validCodeFound;
}
