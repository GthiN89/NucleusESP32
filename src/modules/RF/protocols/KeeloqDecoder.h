#ifndef KEELOQ_DECODER_H
#define KEELOQ_DECODER_H

#include <cstdint>
#include <cstddef>

class KeeloqDecoder {
public:
    KeeloqDecoder();
    ~KeeloqDecoder();

    // data is an array of signed long long pulses (level encoded in sign)
    // returns true if a valid key is decoded
    bool decode(const long long int* data, size_t size);

private:
    enum class DecoderStep {
        Reset,
        CheckPreambula,
        SaveDuration,
        CheckDuration
    };

    DecoderStep step;
    uint32_t headerCount;
    uint64_t decodeData;
    uint32_t decodeCountBit;
    uint32_t teLast;

    static constexpr uint32_t TE_SHORT = 400;
    static constexpr uint32_t TE_LONG = 800;
    static constexpr uint32_t TE_DELTA = 140;
    static constexpr uint32_t MIN_COUNT_BIT = 64;

    uint32_t durationDiff(uint32_t a, uint32_t b) const;
    void printDecodedData(uint64_t data, uint32_t bitCount) const;
    bool handlePulse(bool level, uint32_t duration);
};

#endif // KEELOQ_DECODER_H
