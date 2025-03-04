#include "LinearProtocol.h"
#include <cstdio>
#include <sstream>

LinearProtocol::LinearProtocol()
    : decodeData(0),
      decodeCountBit(0),
      te_last(0),
      validCodeFound(false),
      finalData(0),
      finalBitCount(0),
      decoderState(StateReset),
      encodeData(0),
      encodeBitCount(0)
{
}

void LinearProtocol::reset() {
    decoderState = StateReset;
    decodeData = 0;
    decodeCountBit = 0;
    te_last = 0;
    validCodeFound = false;
    finalData = 0;
    finalBitCount = 0;
}

void LinearProtocol::feed(bool level, uint32_t duration) {
    switch(decoderState) {
        case StateReset:
            // Look for header: a low pulse approximately te_short*42
            if (!level && durationDiff(duration, te_short * 42) < te_delta * 20) {
                decodeData = 0;
                decodeCountBit = 0;
                decoderState = StateSaveDuration;
            }
            break;
        case StateSaveDuration:
            if (level) {
                te_last = duration;
                decoderState = StateCheckDuration;
            } else {
                decoderState = StateReset;
            }
            break;
        case StateCheckDuration:
            if (!level) {
                // If duration is long enough, it might be a guard time
                if (duration >= te_short * 5) {
                    // Check guard time matches header (te_short*42)
                    if (durationDiff(duration, te_short * 42) > te_delta * 20) {
                        decoderState = StateReset;
                        break;
                    }
                    // Decide bit value using previous pulse
                    if (durationDiff(te_last, te_short) < te_delta) {
                        decodeData = (decodeData << 1) | 0;
                        decodeCountBit++;
                    } else if (durationDiff(te_last, te_long) < te_delta) {
                        decodeData = (decodeData << 1) | 1;
                        decodeCountBit++;
                    }
                    // If enough bits have been received, mark the code as valid.
                    if (decodeCountBit == min_count_bit) {
                        finalData = decodeData;
                        finalBitCount = decodeCountBit;
                        validCodeFound = true;
                    }
                    decoderState = StateReset;
                    break;
                }
                // Otherwise, use the durations to determine a bit.
                if (durationDiff(te_last, te_short) < te_delta &&
                    durationDiff(duration, te_long) < te_delta) {
                    decodeData = (decodeData << 1) | 0;
                    decodeCountBit++;
                    decoderState = StateSaveDuration;
                } else if (durationDiff(te_last, te_long) < te_delta &&
                           durationDiff(duration, te_short) < te_delta) {
                    decodeData = (decodeData << 1) | 1;
                    decodeCountBit++;
                    decoderState = StateSaveDuration;
                } else {
                    decoderState = StateReset;
                }
            } else {
                decoderState = StateReset;
            }
            break;
    }
}

bool LinearProtocol::decode(const long long int* samples, size_t sampleCount) {
    reset();
    for (size_t i = 0; i < sampleCount; i++) {
        if (samples[i] > 0)
            feed(true, static_cast<uint32_t>(samples[i]));
        else
            feed(false, static_cast<uint32_t>(-samples[i]));
        if (validCodeFound)
            return true;
    }
    return false;
}

std::string LinearProtocol::getDIPString(uint32_t data) const {
    char buf[11];
    snprintf(buf, sizeof(buf), "%c%c%c%c%c%c%c%c%c%c",
             (data & 0x0200) ? '1' : '0',
             (data & 0x0100) ? '1' : '0',
             (data & 0x0080) ? '1' : '0',
             (data & 0x0040) ? '1' : '0',
             (data & 0x0020) ? '1' : '0',
             (data & 0x0010) ? '1' : '0',
             (data & 0x0008) ? '1' : '0',
             (data & 0x0004) ? '1' : '0',
             (data & 0x0002) ? '1' : '0',
             (data & 0x0001) ? '1' : '0');
    return std::string(buf);
}

std::string LinearProtocol::getCodeString() const {
    char buf[128];
    uint32_t key = finalData;
    uint32_t rev = 0;
    for (uint8_t i = 0; i < finalBitCount; i++) {
        rev <<= 1;
        rev |= (key >> i) & 1;
    }
    std::string dipStr = getDIPString(key);
    snprintf(buf, sizeof(buf), "Linear %dbit\r\nKey:0x%08X\r\nYek:0x%08X\r\nDIP:%s",
             finalBitCount, key, rev, dipStr.c_str());
    return std::string(buf);
}

bool LinearProtocol::hasValidCode() const {
    return validCodeFound;
}

void LinearProtocol::startEncoding(uint32_t code, uint8_t bitCount) {
    samplesToSend.clear();
    encodeData = code;
    encodeBitCount = bitCount;
    // For each data bit (except the least significant bit)
    for (int i = bitCount; i > 1; i--) {
        uint8_t bit = (code >> (i - 1)) & 1;
        if (bit) {
            samplesToSend.push_back(static_cast<long long int>(te_long));
            samplesToSend.push_back(-static_cast<long long int>(te_short));
        } else {
            samplesToSend.push_back(static_cast<long long int>(te_short));
            samplesToSend.push_back(-static_cast<long long int>(te_long));
        }
    }
    // End bit (least significant bit) with guard:
    uint8_t lastBit = code & 1;
    if (lastBit) {
        samplesToSend.push_back(static_cast<long long int>(te_short * 3));
        samplesToSend.push_back(-static_cast<long long int>(te_short * 42));
    } else {
        samplesToSend.push_back(static_cast<long long int>(te_short));
        samplesToSend.push_back(-static_cast<long long int>(te_short * 44));
    }
}

const std::vector<long long int>& LinearProtocol::getEncodedSamples() const {
    return samplesToSend;
}
