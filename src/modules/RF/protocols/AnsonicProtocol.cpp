// AnsonicProtocol.cpp
#include "AnsonicProtocol.h"
#include <cstdio>
#include <cstdlib>

AnsonicProtocol::AnsonicProtocol()
    : te_short(555),
      te_long(1111),
      te_delta(120),
      min_count_bit(12),
      decodeData(0),
      decodeCountBit(0),
      te_last(0),
      validCodeFound(false),
      finalCode(0),
      finalBitCount(0),
      decoderState(DecoderStepReset),
      cnt(0),
      btn(0),
      encodeData(0),
      encodeBitCount(0),
      encoderState(EncoderStepIdle)
{
}

void AnsonicProtocol::reset() {
    decoderState = DecoderStepReset;
    decodeData = 0;
    decodeCountBit = 0;
    te_last = 0;
    validCodeFound = false;
    finalCode = 0;
    finalBitCount = 0;
}

void AnsonicProtocol::feed(bool level, uint32_t duration) {
    switch(decoderState) {
    case DecoderStepReset:
        if (!level && durationDiff(duration, te_short * 35) < te_delta * 35) {
            decoderState = DecoderStepFoundStartBit;
        }
        break;
    case DecoderStepFoundStartBit:
        if (!level) {
            break;
        } else if (durationDiff(duration, te_short) < te_delta) {
            decoderState = DecoderStepSaveDuration;
            decodeData = 0;
            decodeCountBit = 0;
        } else {
            decoderState = DecoderStepReset;
        }
        break;
    case DecoderStepSaveDuration:
        if (!level) { // save interval
            if (duration >= (te_short * 4)) {
                decoderState = DecoderStepFoundStartBit;
                if (decodeCountBit >= min_count_bit) {
                    cnt = decodeData & 0xFFF;
                    btn = (decodeData >> 1) & 0x3;
                    validCodeFound = true;
                    finalCode = decodeData;
                    finalBitCount = decodeCountBit;
                }
                break;
            }
            te_last = duration;
            decoderState = DecoderStepCheckDuration;
        } else {
            decoderState = DecoderStepReset;
        }
        break;
    case DecoderStepCheckDuration:
        if (level) {
            if (durationDiff(te_last, te_short) < te_delta &&
                durationDiff(duration, te_long) < te_delta) {
                decodeData = (decodeData << 1) | 1;
                decodeCountBit++;
                decoderState = DecoderStepSaveDuration;
            } else if (durationDiff(te_last, te_long) < te_delta &&
                       durationDiff(duration, te_short) < te_delta) {
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

bool AnsonicProtocol::decode(const long long int* samples, size_t sampleCount) {
    reset();
    for (size_t i = 0; i < sampleCount; i++) {
        if (samples[i] > 0) {
            feed(true, static_cast<uint32_t>(samples[i]));
        } else {
            feed(false, static_cast<uint32_t>(-samples[i]));
        }
        if (validCodeFound)
            return true;
    }
    return false;
}

std::string AnsonicProtocol::getDIPString(uint32_t dip) {
    char buf[11];
    // Order: 0x0800, 0x0400, 0x0200, 0x0100, 0x0080, 0x0040, 0x0020, 0x0010, 0x0001, 0x0008
    snprintf(buf, sizeof(buf), "%c%c%c%c%c%c%c%c%c%c",
             (dip & 0x0800) ? '1' : '0',
             (dip & 0x0400) ? '1' : '0',
             (dip & 0x0200) ? '1' : '0',
             (dip & 0x0100) ? '1' : '0',
             (dip & 0x0080) ? '1' : '0',
             (dip & 0x0040) ? '1' : '0',
             (dip & 0x0020) ? '1' : '0',
             (dip & 0x0010) ? '1' : '0',
             (dip & 0x0001) ? '1' : '0',
             (dip & 0x0008) ? '1' : '0');
    return std::string(buf);
}

std::string AnsonicProtocol::getCodeString() {
    char buf[128];
    std::string dipStr = getDIPString(cnt);
    snprintf(buf, sizeof(buf), "Ansonic %dbit\r\nKey:0x%03lX\r\nBtn:%X\r\nDIP:%s\r\n",
             finalBitCount, finalCode, btn, dipStr.c_str());
    return std::string(buf);
}

bool AnsonicProtocol::hasValidCode() const {
    return validCodeFound;
}

uint32_t AnsonicProtocol::reverseKey(uint32_t code, uint8_t bitCount) const {
    uint32_t reversed = 0;
    for (uint8_t i = 0; i < bitCount; i++) {
        reversed <<= 1;
        reversed |= (code >> i) & 1;
    }
    return reversed;
}

void AnsonicProtocol::startEncoding(uint32_t code, uint8_t bitCount) {
    samplesToSend.clear();
    encodeData = code;
    encodeBitCount = bitCount;
    // Header: low pulse for te_short*35
    samplesToSend.push_back(-static_cast<int32_t>(te_short * 35));
    // Start bit: high pulse for te_short
    samplesToSend.push_back(te_short);
    // Encode bits (MSB first)
    for (int i = bitCount; i > 0; i--) {
        uint8_t bit = (code >> (i - 1)) & 1;
        if (bit) {
            samplesToSend.push_back(-static_cast<int32_t>(te_short));
            samplesToSend.push_back(te_long);
        } else {
            samplesToSend.push_back(-static_cast<int32_t>(te_long));
            samplesToSend.push_back(te_short);
        }
    }
    encoderState = EncoderStepReady;
}

const std::vector<long long int>& AnsonicProtocol::getEncodedSamples() const {
    return samplesToSend;
}
