#include "KeeloqDecoder.h"
#include <cstdlib>
#include <Arduino.h>
#include "lvgl.h"
#include "GUI/ScreenManager.h"
#include "globals.h"


uint32_t KeeloqDecoder::durationDiff(uint32_t a, uint32_t b) const {
    return (a > b) ? a - b : b - a;
}

void KeeloqDecoder::printDecodedData(uint64_t data, uint32_t bitCount) const {
    ScreenManager& screenMgr = ScreenManager::getInstance();

    Serial.println("Keeloq");
    Serial.print(bitCount);
    Serial.println(" bit");
    Serial.print("Key: 0x");
    Serial.println(data, HEX);

     lv_obj_t * textarea;
    if(C1101preset == CUSTOM){
        textarea = screenMgr.text_area_SubGHzCustom;        
    } else {
        textarea = screenMgr.getTextArea();
    }

    lv_textarea_add_text(textarea, "\nKeeloq\n");
    lv_textarea_add_text(textarea, String(bitCount).c_str());
    lv_textarea_add_text(textarea, " bit\n");
    lv_textarea_add_text(textarea, "Key: 0x");
    lv_textarea_add_text(textarea, String(data, HEX).c_str());
}

bool KeeloqDecoder::handlePulse(bool level, uint32_t duration) {
    switch (step) {
        case DecoderStep::Reset:
            if (level && durationDiff(duration, TE_SHORT) < TE_DELTA) {
                step = DecoderStep::CheckPreambula;
                headerCount++;
            }
            break;

        case DecoderStep::CheckPreambula:
            if (!level && durationDiff(duration, TE_SHORT) < TE_DELTA) {
                step = DecoderStep::Reset;
                break;
            }
            if (headerCount > 2 &&
                durationDiff(duration, TE_SHORT * 10) < TE_DELTA * 10) {
                step = DecoderStep::SaveDuration;
                decodeData = 0;
                decodeCountBit = 0;
            } else {
                step = DecoderStep::Reset;
                headerCount = 0;
            }
            break;

        case DecoderStep::SaveDuration:
            if (level) {
                teLast = duration;
                step = DecoderStep::CheckDuration;
            }
            break;

        case DecoderStep::CheckDuration:
            if (!level) {
                if (duration >= (TE_SHORT * 2 + TE_DELTA)) {
                    step = DecoderStep::Reset;
                    if (decodeCountBit >= MIN_COUNT_BIT &&
                        decodeCountBit <= MIN_COUNT_BIT + 2) {
                        printDecodedData(decodeData, decodeCountBit);
                        decodeData = 0;
                        decodeCountBit = 0;
                        headerCount = 0;
                        return true;
                    }
                    break;
                } else if (durationDiff(teLast, TE_SHORT) < TE_DELTA &&
                           durationDiff(duration, TE_LONG) < TE_DELTA * 2) {
                    if (decodeCountBit < MIN_COUNT_BIT) {
                        decodeData = (decodeData << 1) | 1;
                    }
                    decodeCountBit++;
                    step = DecoderStep::SaveDuration;
                } else if (durationDiff(teLast, TE_LONG) < TE_DELTA * 2 &&
                           durationDiff(duration, TE_SHORT) < TE_DELTA) {
                    if (decodeCountBit < MIN_COUNT_BIT) {
                        decodeData = (decodeData << 1) | 0;
                    }
                    decodeCountBit++;
                    step = DecoderStep::SaveDuration;
                } else {
                    step = DecoderStep::Reset;
                    headerCount = 0;
                }
            } else {
                step = DecoderStep::Reset;
                headerCount = 0;
            }
            break;
    }
    return false;
}

KeeloqDecoder::KeeloqDecoder()
    : step(DecoderStep::Reset),
      headerCount(0),
      decodeData(0),
      decodeCountBit(0),
      teLast(0) {
}

KeeloqDecoder::~KeeloqDecoder() {
}

bool KeeloqDecoder::decode(const long long int* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        bool level = (data[i] > 0);
        uint32_t duration = static_cast<uint32_t>(llabs(data[i]));
        if (handlePulse(level, duration)) {
            return true;
        }
    }
    if (decodeCountBit >= MIN_COUNT_BIT) {
        printDecodedData(decodeData, decodeCountBit);
        return true;
    } else {
        Serial.print("Not enough bits decoded: ");
        Serial.println(decodeCountBit);
        return false;
    }
}
