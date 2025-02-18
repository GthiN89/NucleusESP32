#include "Holtek_HT12xDecoder.h"
#include "GUI/ScreenManager.h" // Assumes a ScreenManager singleton for GUI updates.
#include "math.h"
#include <stdio.h>
#include <string.h>

// Macros for DIP formatting.
#define DIP_PATTERN "%c%c%c%c%c%c%c%c"
#define CNT_TO_DIP(dip)                                                                     \
    ((dip & 0x0080) ? '0' : '1'), ((dip & 0x0040) ? '0' : '1'), ((dip & 0x0020) ? '0' : '1'), \
    ((dip & 0x0010) ? '0' : '1'), ((dip & 0x0008) ? '0' : '1'), ((dip & 0x0004) ? '0' : '1'), \
    ((dip & 0x0002) ? '0' : '1'), ((dip & 0x0001) ? '0' : '1')




HoltekHT12xDecoder::HoltekHT12xDecoder()
    : parserStep(StepReset),
      te_short(320),
      te_long(640),
      te_delta(200),
      min_count_bit(12),
      te(0),
      last_data(0),
      decodeData(0),
      decodeCountBit(0),
      te_last(0),
      finalData(0),
      finalDataCountBit(0),
      btn(0),
      cnt(0),
      te_final(0),
      validCodeFound(false) {
    ////Serial.println("[HoltekHT12x] Decoder initialized");
}

void HoltekHT12xDecoder::reset() {
    ////Serial.println("[HoltekHT12x] Resetting decoder state");
    parserStep = StepReset;
    decodeData = 0;
    decodeCountBit = 0;
    validCodeFound = false;
    finalData = 0;
    finalDataCountBit = 0;
    te = 0;
}

void HoltekHT12xDecoder::feed(bool level, uint32_t duration) {
    Serial.print("[HoltekHT12x] Feed: level=");
    Serial.print(level);
    Serial.print(" duration=");
    ////Serial.println(duration);
    Serial.print("//////////////////////////////////");
    ////Serial.println(decodeCountBit);
    switch (parserStep) {
        case StepReset:
            ////Serial.println("[HoltekHT12x] State: Reset");
            if (!level && (DURATION_DIFF(duration, te_short * 36) < te_delta * 36)) {
                ////Serial.println("[HoltekHT12x] Header detected; switching to FoundStartBit");
                parserStep = StepFoundStartBit;
            }
            break;
        case StepFoundStartBit:
            ////Serial.println("[HoltekHT12x] State: FoundStartBit");
            if (level && (DURATION_DIFF(duration, te_short) < te_delta)) {
                ////Serial.println("[HoltekHT12x] Start bit detected; switching to SaveDuration");
                parserStep = StepSaveDuration;
                decodeData = 0;
                decodeCountBit = 0;
                te = duration;
            } else {
                ////Serial.println("[HoltekHT12x] Start bit mismatch; resetting state");
                parserStep = StepReset;
            }
            break;
        case StepSaveDuration:
            ////Serial.println("[HoltekHT12x] State: SaveDuration");
            if (!level) {
                if (duration >= ((uint32_t)te_short * 10 + te_delta)) {
                    // End of key sequence; if repeated key, then validate.
                    if (decodeCountBit == min_count_bit) {
                        // Average TE calculation and key finalization would occur here.
                        finalData = decodeData;
                        finalDataCountBit = decodeCountBit;
                        validCodeFound = true;
                        Serial.print("[HoltekHT12x] Valid key found: 0x");
                        ////Serial.println(finalData, HEX);
                    }
                    last_data = decodeData;
                    decodeData = 0;
                    decodeCountBit = 0;
                    te = 0;
                    parserStep = StepFoundStartBit;
                    break;
                } else {
                    te_last = duration;
                    te += duration;
                    parserStep = StepCheckDuration;
                }
            } else {
                ////Serial.println("[HoltekHT12x] Unexpected HIGH in SaveDuration; resetting state");
                parserStep = StepReset;
            }
            break;
        case StepCheckDuration:
            ////Serial.println("[HoltekHT12x] State: CheckDuration");
            if (level) {
                te += duration;
                if ((DURATION_DIFF(te_last, te_long) < te_delta * 2) &&
                    (DURATION_DIFF(duration, te_short) < te_delta)) {
                    ////Serial.println("[HoltekHT12x] Detected bit 1");
                    // In a complete implementation, you’d add a bit here.
                    // For demonstration, assume we accumulate the bit in decodeData.
                    decodeData = (decodeData << 1) | 1;
                    decodeCountBit++;
                    parserStep = StepSaveDuration;
                } else if ((DURATION_DIFF(te_last, te_short) < te_delta) &&
                           (DURATION_DIFF(duration, te_long) < te_delta * 2)) {
                    ////Serial.println("[HoltekHT12x] Detected bit 0");
                    decodeData = (decodeData << 1) | 0;
                    decodeCountBit++;
                    parserStep = StepSaveDuration;
                } else {
                    ////Serial.println("[HoltekHT12x] Duration mismatch; resetting state");
                    parserStep = StepReset;
                }
            } else {
                ////Serial.println("[HoltekHT12x] Expected HIGH in CheckDuration but got LOW; resetting state");
                parserStep = StepReset;
            }
            break;
        default:
            ////Serial.println("[HoltekHT12x] Unknown state; resetting");
            parserStep = StepReset;
            break;
    }
}

bool HoltekHT12xDecoder::decode(long long int* samples, size_t sampleCount) {
    ////Serial.println("[HoltekHT12x] Starting decode process");
    reset();
    for (size_t i = 0; i < sampleCount; i++) {
        Serial.print("[HoltekHT12x] Sample[");
        Serial.print(i);
        Serial.print("]: ");
        ////Serial.println(samples[i]);
        if (samples[i] > 0)
            feed(true, samples[i]);
        else
            feed(false, -samples[i]);
        if (validCodeFound) {
            ////Serial.println("[HoltekHT12x] Valid key detected; terminating decode loop");
            return true;
        }
    }
    ////Serial.println("[HoltekHT12x] Decode process finished; no valid key found");
    return false;
}

String HoltekHT12xDecoder::getCodeString() {
    char buf[256];
    sprintf(buf,
            "%s %db\r\n"
            "Key:0x%03lX\r\n"
            "Btn:0x%X\r\n"
            "DIP:" DIP_PATTERN "\r\n"
            "Te:%luus\r\n",
            "Holtek_HT12x",
            min_count_bit,  // assuming 12-bit key
            (uint32_t)(finalData & 0xFFF),
            (uint32_t)(finalData & 0xF),
            CNT_TO_DIP(finalData & 0xFF),  // using lower 8 bits for DIP
            te_final);
    ////Serial.println("[Holtek_HT12x] Formatted key string:");
    ////Serial.println(buf);
    // Update GUI textarea.
    ScreenManager& screenMgr = ScreenManager::getInstance();
    lv_obj_t* textarea = screenMgr.getTextArea();
    lv_textarea_set_text(textarea, buf);
    return String(buf);
}

bool HoltekHT12xDecoder::hasValidCode() const {
    return validCodeFound;
}
