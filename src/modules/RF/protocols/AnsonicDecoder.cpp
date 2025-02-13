#include "AnsonicDecoder.h"
#include <stdio.h>
#include "lvgl.h"
#include "GUI/ScreenManager.h"
#include "globals.h"

// DIP display macros.
#define DIP_PATTERN "%c%c%c%c%c%c%c%c%c%c"
#define CNT_TO_DIP(dip)                                                                     \
    ((dip & 0x0800) ? '1' : '0'), ((dip & 0x0400) ? '1' : '0'), ((dip & 0x0200) ? '1' : '0'), \
    ((dip & 0x0100) ? '1' : '0'), ((dip & 0x0080) ? '1' : '0'), ((dip & 0x0040) ? '1' : '0'), \
    ((dip & 0x0020) ? '1' : '0'), ((dip & 0x0010) ? '1' : '0'), ((dip & 0x0001) ? '1' : '0'), \
    ((dip & 0x0008) ? '1' : '0')

AnsonicDecoder::AnsonicDecoder()
    : te_short(555),
      te_long(1111),
      te_delta(120),
      min_count_bit(12),
      state(StepReset),
      decodeData(0),
      decodeCountBit(0),
      te_last(0),
      data(0),
      dataCountBit(0),
      btn(0),
      cnt(0),
      protocolName("Ansonic"),
      validCodeFound(false) {
}

void AnsonicDecoder::reset() {
    state = StepReset;
    decodeData = 0;
    decodeCountBit = 0;
    validCodeFound = false;
    data = 0;
    dataCountBit = 0;
    btn = 0;
    cnt = 0;
}

uint32_t AnsonicDecoder::durationDiff(uint32_t a, uint32_t b) const {
    return (a > b) ? a - b : b - a;
}

void AnsonicDecoder::addBit(uint8_t bit) {
    decodeData = (decodeData << 1) | bit;
    decodeCountBit++;
}

void AnsonicDecoder::feed(bool level, uint32_t duration) {
    switch(state) {
    case StepReset:
        // Look for header: a falling edge (level == false) with duration near (te_short*35)
        if((!level) && (durationDiff(duration, te_short * 35) < te_delta * 35)) {
            state = StepFoundStartBit;
        }
        break;
    case StepFoundStartBit:
        if(!level) {
            // Wait for rising edge.
            break;
        } else if(durationDiff(duration, te_short) < te_delta) {
            // Found start bit.
            state = StepSaveDuration;
            decodeData = 0;
            decodeCountBit = 0;
        } else {
            state = StepReset;
        }
        break;
    case StepSaveDuration:
        if(!level) { // On falling edge, save the duration.
            if(duration >= (te_short * 4)) {
                state = StepFoundStartBit;
                if(decodeCountBit >= min_count_bit) {
                    // Finalize the received key.
                    data = decodeData;
                    dataCountBit = decodeCountBit;
                    // In the original code, serial and btn are set to zero.
                    btn = 0;
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
        if(level) {
            if((durationDiff(te_last, te_short) < te_delta) &&
               (durationDiff(duration, te_long) < te_delta)) {
                addBit(1);
                state = StepSaveDuration;
            } else if((durationDiff(te_last, te_long) < te_delta) &&
                      (durationDiff(duration, te_short) < te_delta)) {
                addBit(0);
                state = StepSaveDuration;
            } else {
                state = StepReset;
            }
        } else {
            state = StepReset;
        }
        break;
    }
}

bool AnsonicDecoder::decode(long long int* samples, size_t sampleCount) {
    reset();
    for (size_t i = 0; i < sampleCount; i++) {
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

String AnsonicDecoder::getCodeString() {
    // Emulate remote controller analysis:
    // Set cnt = lower 12 bits of data; btn = bits [1:2] of data.
    cnt = data & 0xFFF;
    btn = ((data >> 1) & 0x3);

    char buf[256];
    sprintf(buf,
            "%s %dbit\r\n"
            "Key:%03lX\r\n"
            "Btn:%X\r\n"
            "DIP:" DIP_PATTERN "\r\n",
            protocolName.c_str(),
            dataCountBit,
            (uint32_t)(data & 0xFFFFFFFF),
            btn,
            CNT_TO_DIP(cnt));

    // Update the GUI textarea.
    ScreenManager& screenMgr = ScreenManager::getInstance();
    lv_obj_t* textarea = screenMgr.getTextArea();
    lv_textarea_set_text(textarea, buf);

    return String(buf);
}

bool AnsonicDecoder::hasValidCode() const {
    return validCodeFound;
}
