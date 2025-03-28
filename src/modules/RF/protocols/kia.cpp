#include "kia.hpp"
#include "lvgl.h"
#include "GUI/ScreenManager.h"
#include "globals.h"
#include "math.h"
#include <bitset>


KiaProtocol::KiaProtocol()
     : te_short(250),
       te_long(500),
       te_delta(100),
       min_count_bit(61)
{
}



void KiaProtocol::reset() {
    state = KIADecoderStepReset;
    decodeData = 0;
    decodeCountBit = 0;
    te_last = 0;
    validCodeFound = false;
    finalCode = 0;
    finalBitCount = 0;
}

inline void KiaProtocol::addBit(uint8_t bit) {
    decodeData = decodeData << 1 | bit;
    decodeCountBit++;
}

bool KiaProtocol::decode(long long int* samples, size_t sampleCount) {
    reset();
    for(size_t i = 0; i < sampleCount; i++) {
        ////Serial.print("Sample index ");
        ////Serial.print(i);
        ////Serial.print(": ");
       ////Serial.println(samples[i]);
        
        if(samples[i] > 0) {
            feed(true, samples[i]);
        } else {
            feed(false, -samples[i]);
        }
        
        if(validCodeFound) {
           ////Serial.println(F("Valid code found, exiting decode loop"));
            return true;
        }
    }
    return false;
}


void KiaProtocol::feed(bool level, uint32_t duration) {

    switch(state) {
    case KIADecoderStepReset:
        if((level) && (DURATION_DIFF(duration, te_short) <
                       te_delta)) {
            state = KIADecoderStepCheckPreambula;
            te_last = duration;
            header_count = 0;
        }
        break;
    case KIADecoderStepCheckPreambula:
        if(level) {
            if((DURATION_DIFF(duration, te_short) <
                te_delta) ||
               (DURATION_DIFF(duration, te_long) <
                te_delta)) {
                te_last = duration;
            } else {
                state = KIADecoderStepReset;
            }
        } else if(
            (DURATION_DIFF(duration, te_short) <
             te_delta) &&
            (DURATION_DIFF(te_last, te_short) <
             te_delta)) {
            // Found header
            header_count++;
            break;
        } else if(
            (DURATION_DIFF(duration, te_long) <
             te_delta) &&
            (DURATION_DIFF(te_last, te_long) <
             te_delta)) {
            // Found start bit
            if(header_count > 15) {
                state = KIADecoderStepSaveDuration;
                decodeData = 0;
                decodeCountBit = 1;
                addBit(1);
            } else {
                state = KIADecoderStepReset;
            }
        } else {
            state = KIADecoderStepReset;
        }
        break;
    case KIADecoderStepSaveDuration:
        if(level) {
            if(duration >=
               (te_long + te_delta * 2UL)) {
                //Found stop bit
                state = KIADecoderStepReset;
                if(decodeCountBit ==
                    min_count_bit) {
                    data = decodeData;
                    data_count_bit = decodeCountBit;
                //     if(kuaCallback)
                //         callback(base, context);
                // }
                decodeData = 0;
                decodeCountBit = 0;
                break;
            } else {
                te_last = duration;
                state = KIADecoderStepCheckDuration;
            }

        } else {
            state = KIADecoderStepReset;
        }
        break;
    case KIADecoderStepCheckDuration:
        if(!level) {
            if((DURATION_DIFF(te_last, te_short) <
                te_delta) &&
               (DURATION_DIFF(duration, te_short) <
                te_delta)) {
                addBit(0);
                state = KIADecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(te_last, te_long) <
                 te_delta) &&
                (DURATION_DIFF(duration, te_long) <
                 te_delta)) {
                    addBit(1);
                state = KIADecoderStepSaveDuration;
            } else {
                state = KIADecoderStepReset;
            }
        } else {
            state = KIADecoderStepReset;
        }
        break;
    }
}
}
uint8_t KiaProtocol::crc8(uint8_t* data, size_t len) {
    uint8_t crc = 0x08;
    size_t i, j;
    for(i = 0; i < len; i++) {
        crc ^= data[i];
        for(j = 0; j < 8; j++) {
            if((crc & 0x80) != 0)
                crc = (uint8_t)((crc << 1) ^ 0x7F);
            else
                crc <<= 1;
        }
    }
    return crc;
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
void KiaProtocol::check_remote_controller() {
    /*
    *   0x0F 0112 43B04EC 1 7D
    *   0x0F 0113 43B04EC 1 DF
    *   0x0F 0114 43B04EC 1 30
    *   0x0F 0115 43B04EC 2 13
    *   0x0F 0116 43B04EC 3 F5
    *         CNT  Serial K CRC8 Kia (CRC8, poly 0x7f, start_crc 0x08)
    */

    serial = (uint32_t)((data >> 12) & 0x0FFFFFFF);
    btn = (data >> 8) & 0x0F;
    cnt = (data >> 40) & 0xFFFF;
}

uint32_t KiaProtocol::get_hash_data() {
    return getHashDataLong(
        decodeData, (decodeCountBit / 8) + 1);
}


void KiaProtocol::get_string(uint64_t shortPulse, uint64_t longPulse) {

    char buf[128];    
    check_remote_controller();
    const char* protocolName = "\nKia";
    uint32_t code_found_hi = data >> 32;
    uint32_t code_found_lo = data & 0x00000000ffffffff;

    sprintf(
        buf,
        "%s %dbit\r\n"
        "Key:%08lX%08lX\r\n"
        "Sn:%07lX Btn:%X Cnt:%04lX\r\n",
        protocolName,
        data_count_bit,
        code_found_hi,
        code_found_lo,
        serial,
        btn,
        cnt);


        ScreenManager& screenMgr = ScreenManager::getInstance();
        lv_obj_t * textarea;
if(C1101preset == CUSTOM){
   textarea = screenMgr.text_area_SubGHzCustom;        
} else {
   textarea = screenMgr.getTextArea();
}

lv_textarea_set_text(textarea, buf); 

}
