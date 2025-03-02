#ifndef HOLETK_HT12X_PROTOCOL_H
#define HOLETK_HT12X_PROTOCOL_H

#include <cstdint>
#include <vector>
#include <string>

class HoltekHT12xProtocol {
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

    HoltekHT12xProtocol();

    void reset();
    void feed(bool level, uint32_t duration);
    bool decode(const long long int* samples, size_t sampleCount);
    std::string getCodeString() const;
    bool hasValidCode() const;

    void startEncoding(uint32_t code, uint8_t bitCount);
    const std::vector<long long int>& getEncodedSamples() const;
    void setTE(uint32_t te);

private:
    inline uint32_t durationDiff(uint32_t a, uint32_t b) const {
        return (a > b) ? a - b : b - a;
    }
    std::string getDIPString(uint8_t cnt) const;
    std::string getEventString(uint8_t btn) const;

    static const uint32_t TE_SHORT = 320;
    static const uint32_t TE_LONG  = 640;
    static const uint32_t TE_DELTA = 200;
    static const uint8_t MIN_COUNT_BIT = 12;

    // Decoder variables
    uint32_t decodeData;
    uint8_t decodeCountBit;
    uint32_t te_last;
    bool validCodeFound;
    uint32_t finalCode;
    uint8_t finalBitCount;
    DecoderState decoderState;
    uint32_t currentTe;
    uint32_t lastData;

    // Encoder variables
    uint32_t encodeData;
    uint8_t encodeBitCount;
    EncoderState encoderState;
    std::vector<long long int> samplesToSend;
    uint32_t encodingTE; // TE used for encoding (set via setTE)

};

#endif // HOLETK_HT12X_PROTOCOL_H
