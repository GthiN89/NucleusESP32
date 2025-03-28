
#ifndef KIA_H
#define KIA_H

#include <stdio.h>


struct DecoderKIA;

// Decoder steps
typedef enum {
    KIADecoderStepReset = 0,
    KIADecoderStepCheckPreambula,
    KIADecoderStepSaveDuration,
    KIADecoderStepCheckDuration,
} KIADecoderStep;


class KiaProtocol {
public:

    
    KiaProtocol();

    uint64_t decodeData;
    uint32_t decodeCountBit;

    void reset();
    void feed(bool level, uint32_t duration);
    uint8_t crc8(uint8_t* data, size_t len);
    void check_remote_controller();
    uint32_t get_hash_data();
    void get_string(uint64_t shortPulse, uint64_t longPulse);
    bool decode(long long int* samples, size_t sampleCount);

private:
    // Timing parameters
    uint32_t te_short;
    uint32_t te_long;
    uint32_t te_delta;
    uint32_t min_count_bit;

    // Decoder state
    KIADecoderStep state;

    uint32_t te_last;
    bool validCodeFound;
    uint64_t finalCode;
    uint32_t finalBitCount;
    uint32_t header_count;

    // Decoded data
    uint64_t data;
    uint32_t data_count_bit;

    // Remote controller details
    uint32_t serial;
    uint8_t btn;
    uint16_t cnt;

    // Protocol identifier
    const char* protocolName;

    // External decoder and generic pointers (defined elsewhere)
    DecoderKIA* decoder;
    void addBit(uint8_t bit);

};

#endif // KIA_H

