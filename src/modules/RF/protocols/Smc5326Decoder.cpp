#include "Smc5326Decoder.h"
#include "GUI/ScreenManager.h"  // Assumes a singleton ScreenManager for GUI updates.
#include <stdio.h>
#include <string.h>

// DIP display macros from the original protocol.
#define DIP_P 0b11 // (+)
#define DIP_O 0b10 // (0)
#define DIP_N 0b00 // (-)

// The DIP pattern is eight characters.
#define DIP_PATTERN "%c%c%c%c%c%c%c%c"

// In the original code the macro extracts 2-bit fields at positions 0xE,0xC,...,0x0.

Smc5326Decoder::Smc5326Decoder()
    : te_short(300),
      te_long(900),
      te_delta(200),
      min_count_bit(25),
      state(StepReset),
      decodeData(0),
      decodeCountBit(0),
      te_last(0),
      te(0),
      lastData(0),
      finalData(0),
      finalDataCountBit(0),
      finalTE(0),
      validCodeFound(false)
{
}

void Smc5326Decoder::reset() {
    state = StepReset;
    decodeData = 0;
    decodeCountBit = 0;
    te_last = 0;
    te = 0;
    validCodeFound = false;
    finalData = 0;
    finalDataCountBit = 0;
    finalTE = 0;
    lastData = 0;
}

void Smc5326Decoder::addBit(uint8_t bit) {
    decodeData = (decodeData << 1) | bit;
    decodeCountBit++;
}

String Smc5326Decoder::getEventString(uint8_t event) const {
    char buf[64] = "";
    if (((event >> 6) & 0x3) == 0x3) strcat(buf, "B1 ");
    if (((event >> 4) & 0x3) == 0x3) strcat(buf, "B2 ");
    if (((event >> 2) & 0x3) == 0x3) strcat(buf, "B3 ");
    if (((event >> 0) & 0x3) == 0x3) strcat(buf, "B4 ");
    return String(buf);
}

void Smc5326Decoder::feed(bool level, uint32_t duration) {
    switch(state) {
    case StepReset:
        // Look for preamble: a falling edge (level==false) with duration near (te_short * 24)
        // using a tolerance of te_delta * 12.
        if((!level) && (DURATION_DIFF(duration, te_short * 24) < te_delta * 12)) {
            state = StepSaveDuration;
            decodeData = 0;
            decodeCountBit = 0;
            te = 0;
        }
        break;
    case StepSaveDuration:
        // On rising edge: save the duration and add to accumulated TE.
        if(level) {
            te_last = duration;
            te += duration;
            state = StepCheckDuration;
        }
        break;
    case StepCheckDuration:
        if(!level) {
            // If the falling duration is very long, treat it as a boundary.
            if(duration >= (te_long * 2)) {
                state = StepSaveDuration;
                if(decodeCountBit == min_count_bit) {
                    if((lastData == decodeData) && (lastData != 0)) {
                        // Average the TE value.
                        finalTE = te / (decodeCountBit * 4 + 1);
                        finalData = decodeData;
                        finalDataCountBit = decodeCountBit;
                        validCodeFound = true;
                    }
                    lastData = decodeData;
                }
                decodeData = 0;
                decodeCountBit = 0;
                te = 0;
                break;
            }
            te += duration;
            // Compare the last high duration (te_last) and this low duration to decide on a bit.
            if((DURATION_DIFF(te_last, te_short) < te_delta) &&
               (DURATION_DIFF(duration, te_long) < te_delta * 3)) {
                addBit(0);
                state = StepSaveDuration;
            }
            else if((DURATION_DIFF(te_last, te_long) < te_delta * 3) &&
                    (DURATION_DIFF(duration, te_short) < te_delta)) {
                addBit(1);
                state = StepSaveDuration;
            }
            else {
                state = StepReset;
            }
        }
        else {
            state = StepReset;
        }
        break;
    default:
        state = StepReset;
        break;
    }
}

bool Smc5326Decoder::decode(long long int* samples, size_t sampleCount) {
    reset();
    for(size_t i = 0; i < sampleCount; i++) {
        if(samples[i] > 0)
            feed(true, samples[i]);
        else
            feed(false, -samples[i]);
        if(validCodeFound)
            return true;
    }
    return false;
}

String Smc5326Decoder::getCodeString() {
    // Extract a 16‐bit field from the decoded key as in the original:
    uint32_t data_field = (finalData >> 9) & 0xFFFF;

    // Prepare DIP visualization for three channels.
    auto showDip = [data_field](uint8_t check_dip) -> String {
        char dipBuf[9];
        char c0 = (((data_field >> 14) & 0x3) == check_dip) ? '*' : '_';
        char c1 = (((data_field >> 12) & 0x3) == check_dip) ? '*' : '_';
        char c2 = (((data_field >> 10) & 0x3) == check_dip) ? '*' : '_';
        char c3 = (((data_field >> 8)  & 0x3) == check_dip) ? '*' : '_';
        char c4 = (((data_field >> 6)  & 0x3) == check_dip) ? '*' : '_';
        char c5 = (((data_field >> 4)  & 0x3) == check_dip) ? '*' : '_';
        char c6 = (((data_field >> 2)  & 0x3) == check_dip) ? '*' : '_';
        char c7 = (((data_field >> 0)  & 0x3) == check_dip) ? '*' : '_';
        sprintf(dipBuf, "%c%c%c%c%c%c%c%c", c0, c1, c2, c3, c4, c5, c6, c7);
        return String(dipBuf);
    };

    // Prepare event string from (finalData >> 1) & 0xFF.
    uint8_t event = (finalData >> 1) & 0xFF;
    String eventStr = getEventString(event);

    char buf[256];
    sprintf(buf,
            "%s %ubit\r\nKey:%07lX         \nTe:%luus\r\n"
            // "  +:   " DIP_PATTERN "\r\n"
            // "  o:   " DIP_PATTERN "    ",
            "SMC5326",
            finalDataCountBit,
            (uint32_t)(finalData & 0x1FFFFFF),
            finalTE);
            // Use the original order of DIP fields:
            // showDip(DIP_P).c_str(), showDip(DIP_P).c_str(), showDip(DIP_P).c_str(), showDip(DIP_P).c_str(),
            // showDip(DIP_P).c_str(), showDip(DIP_P).c_str(), showDip(DIP_P).c_str(), showDip(DIP_P).c_str());
    // Append event string and DIP for DIP_N.
    char buf2[64];
    sprintf(buf2, "  -:   " DIP_PATTERN "\r\n", 
            showDip(DIP_N).c_str(), showDip(DIP_N).c_str(), showDip(DIP_N).c_str(), showDip(DIP_N).c_str(),
            showDip(DIP_N).c_str(), showDip(DIP_N).c_str(), showDip(DIP_N).c_str(), showDip(DIP_N).c_str());
    String finalStr = String(buf) + "\r\n" + eventStr+ "\r\n";// + String(buf2);

    // Update the GUI textarea.
    ScreenManager& screenMgr = ScreenManager::getInstance();
    lv_obj_t* textarea = screenMgr.getTextArea();
    lv_textarea_set_text(textarea, finalStr.c_str());

    return String(finalStr);
}

bool Smc5326Decoder::hasValidCode() const {
    return validCodeFound;
}
