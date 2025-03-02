#ifndef CHAMBERLAIN_CODE_PROTOCOL_H
#define CHAMBERLAIN_CODE_PROTOCOL_H

#include <cstdint>
#include <vector>
#include <string>

class ChamberlainCodeProtocol {
public:
    // Decoder states
    enum DecoderState {
        DecoderReset,
        DecoderFoundStartBit,
        DecoderSaveDuration,
        DecoderCheckDuration
    };

    // Encoder states
    enum EncoderState {
        EncoderIdle,
        EncoderStart,
        EncoderReady
    };

    ChamberlainCodeProtocol();

    // --- Decoder interface ---
    void reset();
    void feed(bool level, uint32_t duration);
    bool decode(const long long int* samples, size_t sampleCount);
    std::string getCodeString() const;
    bool hasValidCode() const;
    uint32_t reverseKey(uint32_t code, uint8_t bitCount) const;

    // --- Encoder interface ---
    void startEncoding(uint32_t code, uint8_t bitCount);
    const std::vector<long long int>& getEncodedSamples() const;

private:
    // Returns the absolute difference.
    inline uint32_t durationDiff(uint32_t a, uint32_t b) const {
        return (a > b) ? a - b : b - a;
    }

    // Helpers for decoding:
    uint64_t chamberlainBitToCode(uint32_t data, uint8_t size) const;
    bool chamberlainCodeToBit(uint64_t &data, uint8_t size) const;
    bool checkMaskAndParse();

    // Returns a DIP string according to the decoded bit–count.
    std::string getDIPString() const;

    // --- Protocol constants ---
    static constexpr uint32_t te_short = 1000;
    static constexpr uint32_t te_long  = 3000;
    static constexpr uint32_t te_delta = 200;
    static constexpr uint8_t  min_count_bit = 10;

    // Nibble definitions
    static constexpr uint8_t CH_CODE_BIT_STOP = 0b0001; // 1
    static constexpr uint8_t CH_CODE_BIT_1    = 0b0011; // 3
    static constexpr uint8_t CH_CODE_BIT_0    = 0b0111; // 7

    // Masks for different bit–counts
    static constexpr uint64_t CH7_CODE_MASK       = 0xF000000FF0Full;
    static constexpr uint64_t CH8_CODE_MASK       = 0xF00000F00Full;
    static constexpr uint64_t CH9_CODE_MASK       = 0xF000000000Full;

    static constexpr uint64_t CH7_CODE_MASK_CHECK = 0x10000001101ull;
    static constexpr uint64_t CH8_CODE_MASK_CHECK = 0x1000001001ull;
    static constexpr uint64_t CH9_CODE_MASK_CHECK = 0x10000000001ull;

    // --- Decoder variables ---
    uint64_t decodeData;
    uint8_t decodeCountBit;
    uint32_t te_last;
    bool validCodeFound;
    uint64_t finalCode;
    uint8_t finalBitCount;
    DecoderState decoderState;

    // --- Encoder variables ---
    uint32_t encodeData;
    uint8_t encodeBitCount;
    EncoderState encoderState;
    std::vector<long long int> samplesToSend;
};

#endif // CHAMBERLAIN_CODE_PROTOCOL_H
