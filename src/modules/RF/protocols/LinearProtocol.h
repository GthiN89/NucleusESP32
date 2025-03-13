#ifndef LINEAR_PROTOCOL_H
#define LINEAR_PROTOCOL_H

#include <cstdint>
#include <vector>
#include <string>

class LinearProtocol {
public:
    LinearProtocol();

    // Decoder interface
    void reset();
    void feed(bool level, uint32_t duration);
    bool decode(const long long int* samples, size_t sampleCount);
    std::string getCodeString(uint64_t shortPulse, uint64_t longPulse) const;
    bool hasValidCode() const;

    // Encoder interface
    void startEncoding(uint32_t code, uint8_t bitCount);
    const std::vector<long long int>& getEncodedSamples() const;

private:
    inline uint32_t durationDiff(uint32_t a, uint32_t b) const {
        return (a > b) ? a - b : b - a;
    }
    std::string getDIPString(uint32_t data) const;

    // Protocol constants
    static constexpr uint32_t te_short = 500;
    static constexpr uint32_t te_long  = 1500;
    static constexpr uint32_t te_delta = 150;
    static constexpr uint32_t lastBit = 21000;
    static constexpr uint8_t  min_count_bit = 10;

    // Decoder state machine
    enum DecoderState {
        StateReset,
        StateSaveDuration,
        StateCheckDuration
    };

    // Decoder variables
    uint32_t decodeData;
    uint8_t decodeCountBit;
    uint32_t te_last;
    bool validCodeFound;
    uint32_t finalData;
    uint8_t finalBitCount;
    DecoderState decoderState;

    // Encoder variables
    uint32_t encodeData;
    uint8_t encodeBitCount;
    std::vector<long long int> samplesToSend;
};

#endif // LINEAR_PROTOCOL_H
