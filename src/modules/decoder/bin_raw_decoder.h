#ifndef BIN_RAW_DECODER_H
#define BIN_RAW_DECODER_H

#include <cstdint>
#include <vector>
#include <string>
#include <memory>

#define BIN_RAW_BUF_RAW_SIZE 2048
#define BIN_RAW_MAX_MARKUP_COUNT 128
#define BIN_RAW_THRESHOLD_RSSI -70

struct SubGhzProtocolBinRAW {
    uint32_t id;
    uint8_t type;
    size_t data_raw_ind;
    float data_raw;

};

struct BinRAW_Markup {
    uint32_t data;
};

struct BinRawDecoderState {
    struct {
        const SubGhzProtocolBinRAW* protocol;
    } base;
    size_t data_raw_ind;
    int32_t* data_raw;
    uint8_t* data;
    BinRAW_Markup data_markup[BIN_RAW_MAX_MARKUP_COUNT];
    int32_t adaptive_threshold_rssi;
    bool data_count_bit;
};

class BinRawDecoder {
public:
    BinRawDecoder();
    ~BinRawDecoder();

    // Prevent copying
    BinRawDecoder(const BinRawDecoder&) = delete;
    BinRawDecoder& operator=(const BinRawDecoder&) = delete;

    void setRawSamples(const std::vector<int32_t>& samples);
    bool tryDecode();
    uint32_t calculateHash();
    void getDecodedString(std::string& output);
    uint8_t readCC1101Reg(uint8_t addr);
    bool subghz_protocol_bin_raw_check_remote_controller(SubGhzProtocolBinRAW* instance);

private:
    SubGhzProtocolBinRAW subghz_protocol_bin_raw;
    BinRawDecoderState bin_raw_decoder;
    std::vector<int32_t> raw_samples;
    bool data_count_bit;
    void* spi_handle; // Placeholder for SPI handle type
};

#endif // BIN_RAW_DECODER_H