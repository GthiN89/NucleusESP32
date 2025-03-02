#include "Holtek_HT12xProtocol.h"
#include <cstdio>
#include <cstring>

HoltekHT12xProtocol::HoltekHT12xProtocol()
    : decodeData(0),
      decodeCountBit(0),
      te_last(0),
      validCodeFound(false),
      finalCode(0),
      finalBitCount(0),
      decoderState(DecoderStepReset),
      currentTe(0),
      lastData(0),
      encodeData(0),
      encodeBitCount(0),
      encoderState(EncoderStepIdle),
      encodingTE(TE_SHORT) // default TE for encoding
{
}

void HoltekHT12xProtocol::reset() {
    decoderState = DecoderStepReset;
    decodeData = 0;
    decodeCountBit = 0;
    te_last = 0;
    validCodeFound = false;
    finalCode = 0;
    finalBitCount = 0;
    currentTe = 0;
    lastData = 0;
}

void HoltekHT12xProtocol::feed(bool level, uint32_t duration) {
    switch(decoderState) {
    case DecoderStepReset:
        if(!level && durationDiff(duration, TE_SHORT * 36) < TE_DELTA * 36)
            decoderState = DecoderStepFoundStartBit;
        break;
    case DecoderStepFoundStartBit:
        if(level && durationDiff(duration, TE_SHORT) < TE_DELTA) {
            decoderState = DecoderStepSaveDuration;
            decodeData = 0;
            decodeCountBit = 0;
            currentTe = duration;
        } else {
            decoderState = DecoderStepReset;
        }
        break;
    case DecoderStepSaveDuration:
        if(!level) {
            if(duration >= (TE_SHORT * 10 + TE_DELTA)) {
                if(decodeCountBit == MIN_COUNT_BIT) {
                    if((lastData == decodeData) && lastData != 0) {
                        currentTe /= ((decodeCountBit * 3) + 1);
                        finalCode = decodeData;
                        finalBitCount = decodeCountBit;
                        validCodeFound = true;
                    }
                    lastData = decodeData;
                }
                decodeData = 0;
                decodeCountBit = 0;
                currentTe = 0;
                decoderState = DecoderStepFoundStartBit;
                break;
            } else {
                te_last = duration;
                currentTe += duration;
                decoderState = DecoderStepCheckDuration;
            }
        } else {
            decoderState = DecoderStepReset;
        }
        break;
    case DecoderStepCheckDuration:
        if(level) {
            currentTe += duration;
            if(durationDiff(te_last, TE_LONG) < TE_DELTA * 2 &&
               durationDiff(duration, TE_SHORT) < TE_DELTA) {
                decodeData = (decodeData << 1) | 1;
                decodeCountBit++;
                decoderState = DecoderStepSaveDuration;
            } else if(durationDiff(te_last, TE_SHORT) < TE_DELTA &&
                      durationDiff(duration, TE_LONG) < TE_DELTA * 2) {
                decodeData = (decodeData << 1);
                decodeCountBit++;
                decoderState = DecoderStepSaveDuration;
            } else {
                decoderState = DecoderStepReset;
            }
        } else {
            decoderState = DecoderStepReset;
        }
        break;
    }
}

bool HoltekHT12xProtocol::decode(const long long int* samples, size_t sampleCount) {
    reset();
    for(size_t i = 0; i < sampleCount; i++) {
        if(samples[i] > 0)
            feed(true, static_cast<uint32_t>(samples[i]));
        else
            feed(false, static_cast<uint32_t>(-samples[i]));
        if(validCodeFound)
            return true;
    }
    return false;
}

std::string HoltekHT12xProtocol::getEventString(uint8_t btn) const {
    char buf[32] = {0};
    std::string eventStr;
    if(((btn >> 3) & 0x1) == 0) eventStr += "B1 ";
    if(((btn >> 2) & 0x1) == 0) eventStr += "B2 ";
    if(((btn >> 1) & 0x1) == 0) eventStr += "B3 ";
    if(((btn >> 0) & 0x1) == 0) eventStr += "B4 ";
    return eventStr;
}

std::string HoltekHT12xProtocol::getDIPString(uint8_t cnt) const {
    char buf[9];
    snprintf(buf, sizeof(buf), "%c%c%c%c%c%c%c%c",
             (cnt & 0x80) ? '0' : '1',
             (cnt & 0x40) ? '0' : '1',
             (cnt & 0x20) ? '0' : '1',
             (cnt & 0x10) ? '0' : '1',
             (cnt & 0x08) ? '0' : '1',
             (cnt & 0x04) ? '0' : '1',
             (cnt & 0x02) ? '0' : '1',
             (cnt & 0x01) ? '0' : '1');
    return std::string(buf);
}

std::string HoltekHT12xProtocol::getCodeString() const {
    char buf[128];
    uint32_t key = finalCode & 0x0FFF;
    uint8_t btn = finalCode & 0x0F;
    uint8_t cnt = (finalCode >> 4) & 0xFF;
    std::string eventStr = getEventString(btn);
    std::string dipStr = getDIPString(cnt);
    snprintf(buf, sizeof(buf), "Holtek HT12x %dbit\r\nKey:0x%03X\r\nBtn:%s\r\nDIP:%s\r\nTe:%lums\r\n",
             finalBitCount, key, eventStr.c_str(), dipStr.c_str(), static_cast<unsigned long>(currentTe));
    return std::string(buf);
}

bool HoltekHT12xProtocol::hasValidCode() const {
    return validCodeFound;
}

void HoltekHT12xProtocol::startEncoding(uint32_t code, uint8_t bitCount) {
    samplesToSend.clear();
    encodeData = code;
    encodeBitCount = bitCount;
    // Header: low pulse for te * 36
    samplesToSend.push_back(-static_cast<int32_t>(encodingTE * 36));
    // Start bit: high pulse for te
    samplesToSend.push_back(encodingTE);
    // Encode bits (MSB first)
    for (int i = bitCount; i > 0; i--) {
        uint8_t bit = (code >> (i - 1)) & 1;
        if(bit) {
            samplesToSend.push_back(-static_cast<int32_t>(encodingTE * 2));
            samplesToSend.push_back(encodingTE);
        } else {
            samplesToSend.push_back(-static_cast<int32_t>(encodingTE));
            samplesToSend.push_back(encodingTE * 2);
        }
    }
    encoderState = EncoderStepReady;
}

const std::vector<long long int>& HoltekHT12xProtocol::getEncodedSamples() const {
    return samplesToSend;
}

void HoltekHT12xProtocol::setTE(uint32_t te) {
    encodingTE = te;
}
