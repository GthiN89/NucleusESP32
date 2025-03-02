// AnsonicProtocol.h
#ifndef ANSONIC_PROTOCOL_H
#define ANSONIC_PROTOCOL_H

#include <cstdint>
#include <vector>
#include <string>
#include <bitset>

class AnsonicProtocol {
public:
    enum DecoderState {
        DecoderStepReset,
        DecoderStepFoundStartBit,
        DecoderStepSaveDuration,
        DecoderStepCheckDuration,
    };

    enum EncoderState {
        EncoderStepIdle,
        EncoderStepStart,
        EncoderStepDurations,
        EncoderStepReady,
    };

    AnsonicProtocol();

    // Decoder interface
    void reset();
    void feed(bool level, uint32_t duration);
    bool decode(const long long int* samples, size_t sampleCount);
    std::string getCodeString();
    bool hasValidCode() const;
    uint32_t reverseKey(uint32_t code, uint8_t bitCount) const;

    // Encoder interface
    void startEncoding(uint32_t code, uint8_t bitCount);
    const std::vector<long long int>& getEncodedSamples() const;

private:
    inline uint32_t durationDiff(uint32_t a, uint32_t b) {
        return (a > b) ? (a - b) : (b - a);
    }
    std::string getDIPString(uint32_t dip);

    // Protocol parameters
    const uint32_t te_short;
    const uint32_t te_long;
    const uint32_t te_delta;
    const uint8_t min_count_bit;

    // Decoder variables
    uint32_t decodeData;
    uint8_t decodeCountBit;
    uint32_t te_last;
    bool validCodeFound;
    uint32_t finalCode;
    uint8_t finalBitCount;
    DecoderState decoderState;
    uint32_t cnt;
    uint8_t btn;

    // Encoder variables
    uint32_t encodeData;
    uint8_t encodeBitCount;
    EncoderState encoderState;
    std::vector<long long int> samplesToSend;
};

#endif // ANSONIC_PROTOCOL_H
