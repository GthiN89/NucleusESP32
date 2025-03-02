#include "ChamberlainCodeProtocol.h"
#include <cstdio>
#include <sstream>

ChamberlainCodeProtocol::ChamberlainCodeProtocol()
    : decodeData(0),
      decodeCountBit(0),
      te_last(0),
      validCodeFound(false),
      finalCode(0),
      finalBitCount(0),
      decoderState(DecoderReset),
      encodeData(0),
      encodeBitCount(0),
      encoderState(EncoderIdle)
{
}

void ChamberlainCodeProtocol::reset() {
    decoderState = DecoderReset;
    decodeData = 0;
    decodeCountBit = 0;
    te_last = 0;
    validCodeFound = false;
    finalCode = 0;
    finalBitCount = 0;
}

void ChamberlainCodeProtocol::feed(bool level, uint32_t duration) {
    switch(decoderState) {
        case DecoderReset:
            if (!level && durationDiff(duration, te_short * 39) < te_delta * 20) {
                // Found header
                decoderState = DecoderFoundStartBit;
            }
            break;
        case DecoderFoundStartBit:
            if (level && durationDiff(duration, te_short) < te_delta) {
                // Found start bit: initialize and add a stop nibble first.
                decodeData = 0;
                decodeCountBit = 0;
                decodeData = (decodeData << 4) | CH_CODE_BIT_STOP;
                decodeCountBit++;
                decoderState = DecoderSaveDuration;
            } else {
                decoderState = DecoderReset;
            }
            break;
        case DecoderSaveDuration:
            if (!level) {
                if (duration > te_short * 5) {
                    // End of transmission. If enough bits have been collected, check masks.
                    if (decodeCountBit >= min_count_bit) {
                        if (checkMaskAndParse()) {
                            finalCode = decodeData;
                            finalBitCount = decodeCountBit;
                            validCodeFound = true;
                        }
                    }
                    decoderState = DecoderReset;
                } else {
                    te_last = duration;
                    decoderState = DecoderCheckDuration;
                }
            } else {
                decoderState = DecoderReset;
            }
            break;
        case DecoderCheckDuration:
            if (level) {
                if (durationDiff(te_last, te_short * 3) < te_delta &&
                    durationDiff(duration, te_short) < te_delta) {
                    // Detected stop nibble.
                    decodeData = (decodeData << 4) | CH_CODE_BIT_STOP;
                    decodeCountBit++;
                    decoderState = DecoderSaveDuration;
                } else if (durationDiff(te_last, te_short * 2) < te_delta &&
                           durationDiff(duration, te_short * 2) < te_delta) {
                    // Detected bit “1”
                    decodeData = (decodeData << 4) | CH_CODE_BIT_1;
                    decodeCountBit++;
                    decoderState = DecoderSaveDuration;
                } else if (durationDiff(te_last, te_short) < te_delta &&
                           durationDiff(duration, te_short * 3) < te_delta) {
                    // Detected bit “0”
                    decodeData = (decodeData << 4) | CH_CODE_BIT_0;
                    decodeCountBit++;
                    decoderState = DecoderSaveDuration;
                } else {
                    decoderState = DecoderReset;
                }
            } else {
                decoderState = DecoderReset;
            }
            break;
    }
}

bool ChamberlainCodeProtocol::decode(const long long int* samples, size_t sampleCount) {
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

bool ChamberlainCodeProtocol::chamberlainCodeToBit(uint64_t &data, uint8_t size) const {
    uint64_t data_tmp = data;
    uint64_t data_res = 0;
    for (uint8_t i = 0; i < size; i++) {
        uint8_t nibble = static_cast<uint8_t>(data_tmp & 0xF);
        if (nibble == CH_CODE_BIT_0) {
            data_res = (data_res << 1) | 0;
        } else if (nibble == CH_CODE_BIT_1) {
            data_res = (data_res << 1) | 1;
        } else {
            return false;
        }
        data_tmp >>= 4;
    }
    data = data_res;
    return true;
}

bool ChamberlainCodeProtocol::checkMaskAndParse() {
    // Depending on the number of bits (7, 8, or 9) check the mask
    if ((decodeData & CH7_CODE_MASK) == CH7_CODE_MASK_CHECK) {
        decodeCountBit = 7;
        decodeData &= ~CH7_CODE_MASK;
        decodeData = (decodeData >> 12) | ((decodeData >> 4) & 0xF);
    } else if ((decodeData & CH8_CODE_MASK) == CH8_CODE_MASK_CHECK) {
        decodeCountBit = 8;
        decodeData &= ~CH8_CODE_MASK;
        decodeData = (decodeData >> 4); // simplified
    } else if ((decodeData & CH9_CODE_MASK) == CH9_CODE_MASK_CHECK) {
        decodeCountBit = 9;
        decodeData &= ~CH9_CODE_MASK;
        decodeData >>= 4;
    } else {
        return false;
    }
    return chamberlainCodeToBit(decodeData, decodeCountBit);
}

uint32_t ChamberlainCodeProtocol::reverseKey(uint32_t code, uint8_t bitCount) const {
    uint32_t reversed = 0;
    for (uint8_t i = 0; i < bitCount; i++) {
        reversed <<= 1;
        reversed |= (code >> i) & 1;
    }
    return reversed;
}

std::string ChamberlainCodeProtocol::getDIPString() const {
    std::ostringstream oss;
    uint32_t code = static_cast<uint32_t>(finalCode);
    if (finalBitCount == 7) {
        // 7-bit DIP: bits 0x0040, 0x0020, ... ,0x0001
        oss << ((code & 0x40) ? '1' : '0')
            << ((code & 0x20) ? '1' : '0')
            << ((code & 0x10) ? '1' : '0')
            << ((code & 0x08) ? '1' : '0')
            << ((code & 0x04) ? '1' : '0')
            << ((code & 0x02) ? '1' : '0')
            << ((code & 0x01) ? '1' : '0');
    } else if (finalBitCount == 8) {
        // 8-bit DIP (simplified)
        oss << ((code & 0x80) ? '1' : '0')
            << ((code & 0x40) ? '1' : '0')
            << ((code & 0x20) ? '1' : '0')
            << ((code & 0x10) ? '1' : '0')
            << 'x'
            << ((code & 0x02) ? '1' : '0')
            << ((code & 0x01) ? '1' : '0');
    } else if (finalBitCount == 9) {
        oss << ((code & 0x100) ? '1' : '0')
            << ((code & 0x80) ? '1' : '0')
            << ((code & 0x40) ? '1' : '0')
            << ((code & 0x20) ? '1' : '0')
            << ((code & 0x10) ? '1' : '0')
            << ((code & 0x08) ? '1' : '0')
            << ((code & 0x04) ? '1' : '0')
            << ((code & 0x02) ? '1' : '0')
            << ((code & 0x01) ? '1' : '0');
    }
    return oss.str();
}

std::string ChamberlainCodeProtocol::getCodeString() const {
    char buf[128];
    uint32_t key_found = static_cast<uint32_t>(finalCode & 0xFFFFFFFF);
    uint32_t key_reversed = reverseKey(key_found, finalBitCount);
    snprintf(buf, sizeof(buf), "Chamberlain %db\r\nKey:0x%03X\r\nYek:0x%03X\r\nDIP:%s",
             finalBitCount, key_found, key_reversed, getDIPString().c_str());
    return std::string(buf);
}

bool ChamberlainCodeProtocol::hasValidCode() const {
    return validCodeFound;
}

uint64_t ChamberlainCodeProtocol::chamberlainBitToCode(uint32_t data, uint8_t size) const {
    uint64_t res = 0;
    for (uint8_t i = 0; i < size; i++) {
        res <<= 4;
        if ((data >> (size - i - 1)) & 1)
            res |= CH_CODE_BIT_1;
        else
            res |= CH_CODE_BIT_0;
    }
    return res;
}

void ChamberlainCodeProtocol::startEncoding(uint32_t code, uint8_t bitCount) {
    samplesToSend.clear();
    // Convert the original code into a nibble–encoded value.
    uint64_t encoded = chamberlainBitToCode(code, bitCount);
    if (bitCount == 7) {
        encoded = ((encoded >> 4) << 16) | (((encoded) & 0xF) << 4) | CH7_CODE_MASK_CHECK;
    } else if (bitCount == 8) {
        encoded = ((encoded >> 12) << 16) | (((encoded) & 0xFF) << 4) | CH8_CODE_MASK_CHECK;
    } else if (bitCount == 9) {
        encoded = (encoded << 4) | CH9_CODE_MASK_CHECK;
    } else {
        return;
    }

    // Build a bit–array for upload:
    // Insert 36 guard bits (all 0).
    std::vector<int> bitArray;
    for (int i = 0; i < 36; i++) {
        bitArray.push_back(0);
    }
    // Then append the encoded data bits.
    int totalDataBits = (bitCount == 8 ? 40 : 44);
    for (int i = totalDataBits; i > 0; i--) {
        int bit = (encoded >> (i - 1)) & 1;
        bitArray.push_back(bit);
    }

    // Now convert the bit array into pulse durations.
    // Here we use a simple mapping:
    // Header: low pulse (te_short * 35) then high pulse (te_short)
    samplesToSend.push_back(-static_cast<int>(te_short * 35));
    samplesToSend.push_back(te_short);
    // For each bit: if 1 → low pulse te_short, high pulse te_long;
    // if 0 → low pulse te_long, high pulse te_short.
    for (size_t i = 0; i < bitArray.size(); i++) {
        if (bitArray[i] == 1) {
            samplesToSend.push_back(-static_cast<int>(te_short));
            samplesToSend.push_back(te_long);
        } else {
            samplesToSend.push_back(-static_cast<int>(te_long));
            samplesToSend.push_back(te_short);
        }
    }
    encoderState = EncoderReady;
}

const std::vector<long long int>& ChamberlainCodeProtocol::getEncodedSamples() const {
    return samplesToSend;
}
