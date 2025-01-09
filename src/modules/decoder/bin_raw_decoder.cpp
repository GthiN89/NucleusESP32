#include "bin_raw_decoder.h"
#include <cstring>

BinRawDecoder::BinRawDecoder() : subghz_protocol_bin_raw{0, 0}, spi_handle(nullptr) {
    bin_raw_decoder.base.protocol = &subghz_protocol_bin_raw;
    bin_raw_decoder.data_raw_ind = 0;
    bin_raw_decoder.data_raw = new int32_t[BIN_RAW_BUF_RAW_SIZE];
    bin_raw_decoder.data = new uint8_t[BIN_RAW_BUF_RAW_SIZE];
    memset(bin_raw_decoder.data_markup, 0x00, BIN_RAW_MAX_MARKUP_COUNT * sizeof(BinRAW_Markup));
    bin_raw_decoder.adaptive_threshold_rssi = BIN_RAW_THRESHOLD_RSSI;
}

BinRawDecoder::~BinRawDecoder() {
    delete[] bin_raw_decoder.data_raw;
    delete[] bin_raw_decoder.data;
}

void BinRawDecoder::setRawSamples(const std::vector<int32_t>& samples) {
    raw_samples = samples;
}


bool BinRawDecoder::tryDecode() {
    // Validate input data
    if (raw_samples.empty() || raw_samples.size() > BIN_RAW_BUF_RAW_SIZE) {
        return false;
    }

    // Copy samples to internal buffer
    bin_raw_decoder.data_raw_ind = raw_samples.size();
    for (size_t i = 0; i < raw_samples.size(); i++) {
        bin_raw_decoder.data_raw[i] = raw_samples[i];
    }

    // Reset state
    bin_raw_decoder.data_count_bit = 0;
    memset(bin_raw_decoder.data, 0x00, BIN_RAW_BUF_RAW_SIZE);
    memset(bin_raw_decoder.data_markup, 0x00, BIN_RAW_MAX_MARKUP_COUNT * sizeof(BinRAW_Markup));

    // // Check if we have enough samples for decoding
    // if (bin_raw_decoder.data_raw_ind < BIN_RAW_BUF_MIN_DATA_COUNT) {
    //     return false;
    // }

    // // Try to decode the raw samples
    // if (subghz_protocol_bin_raw_check_remote_controller(&bin_raw_decoder)) {
    //     // Get the first valid sequence length
    //     uint16_t i = 0;
    //     // while (i < BIN_RAW_MAX_MARKUP_COUNT && bin_raw_decoder.data_markup[i].bit_count != 0) {
    //     // //    bin_raw_decoder.generic.data_count_bit += bin_raw_decoder.data_markup[i].bit_count;
    //     //     i++;
    //     // }
    //     return true;
    // }

    return false;
}


uint32_t BinRawDecoder::calculateHash() {
    if (raw_samples.empty()) {
        return 0;
    }
    
    // Implement your hash calculation logic here
    uint32_t hash = 0;
    for (size_t i = 0; i < raw_samples.size(); i++) {
        hash = hash * 31 + raw_samples[i];
    }
    return hash;
}

void BinRawDecoder::getDecodedString(std::string& output) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Raw Data Size: %zu, Hash: 0x%08X", 
             bin_raw_decoder.data_raw_ind, calculateHash());
    output = std::string(buffer);
}

uint8_t BinRawDecoder::readCC1101Reg(uint8_t addr) {
    // Implement your SPI communication here
    return 0;
}

bool BinRawDecoder::subghz_protocol_bin_raw_check_remote_controller(SubGhzProtocolBinRAW* instance) {
    // Initialize timing classification
    // struct TimingClass {
    //     float data;
    //     uint16_t count;
    // } classes[BIN_RAW_SEARCH_CLASSES] = {0};
    
    // Determine analysis window
    size_t ind = (instance->data_raw_ind < 512) ? 
                 (instance->data_raw_ind - 100) : 512;
    
    // // Classify timings
    // for(size_t i = 0; i < ind; i++) {
    //     for(size_t k = 0; k < BIN_RAW_SEARCH_CLASSES; k++) {
    //         float duration = abs(instance->data_raw[i]);
    //         if(classes[k].count == 0) {
    //             classes[k].data = duration;
    //             classes[k].count++;
    //             break;
    //         }
    //         if(DURATION_DIFF(duration, classes[k].data) < (classes[k].data / 4)) {
    //             classes[k].data += (duration - classes[k].data) * 0.05f;
    //             classes[k].count++;
    //             break;
    //         }
    //     }
    // }

    // // Sort classes by occurrence count
    // bool swap = true;
    // while(swap) {
    //     swap = false;
    //     for(size_t i = 1; i < BIN_RAW_SEARCH_CLASSES; i++) {
    //         if(classes[i].count > classes[i-1].count) {
    //             std::swap(classes[i], classes[i-1]);
    //             swap = true;
    //         }
    //     }
    // }

    // // Determine protocol type and timing parameters
    // instance->te = subghz_protocol_bin_raw_const.te_long * 2;
    // bool te_ok = false;
    // uint32_t gap = 0;
    // uint16_t gap_delta = 0;
    // BinRAWType bin_raw_type;

    // if(classes[0].count > BIN_RAW_TE_MIN_COUNT && classes[1].count == 0) {
    //     // Single timing element protocol
    //     instance->te = classes[0].data;
    //     te_ok = true;
    //     bin_raw_type = BinRAWTypeNoGap;
    // }
    // else {
    //     // Process multi-timing protocol
    //     if(!process_multi_timing(instance, classes, te_ok, gap, gap_delta, bin_raw_type)) {
    //         return false;
    //     }
    // }

    // // Decode based on protocol type
    // return bin_raw_type == BinRAWTypeGap ?
    //        decode_gap_protocol(instance, gap, gap_delta) :
    //        decode_no_gap_protocol(instance);
    return false;
}