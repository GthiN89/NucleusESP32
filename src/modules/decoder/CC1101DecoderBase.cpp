#pragma once

#include <vector>
#include <functional>
#include <string>
#include <sstream>
#include <memory>
#include "driver/gpio.h"
#include "esp_timer.h"
#include <esp_timer.h>

class CC1101DecoderBase {
public:
    struct DecodedSignal {
        std::vector<uint32_t> timings;
        uint64_t timestamp;
        uint32_t protocol_hash;
        std::string decoded_string;
        uint32_t frequency;
        int8_t rssi;
        bool carrier_present;
    };

    struct CarrierInfo {
        uint32_t frequency;
        int8_t rssi;
        bool carrier_present;
        float lqi;  // Link Quality Indicator
        int16_t freq_offset;
    };

    using DecoderCallback = std::function<void(const DecodedSignal&)>;

protected:
    DecoderCallback callback;
    uint64_t last_time;
    bool signal_detected;
    std::vector<uint32_t> raw_samples;
    static constexpr size_t MAX_SAMPLES = 1024;
    
    // CC1101 specific configurations
    static constexpr uint8_t CC1101_FREQ2 = 0x0D;
    static constexpr uint8_t CC1101_FREQ1 = 0x0E;
    static constexpr uint8_t CC1101_FREQ0 = 0x0F;
    static constexpr uint8_t CC1101_FREQOFF = 0x0C;
    static constexpr uint8_t CC1101_RSSI = 0x34;
    static constexpr uint8_t CC1101_PKTSTATUS = 0x38;
    static constexpr uint8_t CC1101_LQI = 0x33;
    static constexpr uint8_t CC1101_CS_MASK = 0x40;
    
    // Default timing configs
    static constexpr uint32_t MIN_PULSE_LENGTH = 100;  // microseconds
    static constexpr uint32_t MAX_PULSE_LENGTH = 2500; // microseconds
    static constexpr uint32_t DEFAULT_TOLERANCE = 120;  // microseconds
    
public:
    CC1101DecoderBase() : 
        signal_detected(false), 
        last_time(0),
        rssi_threshold(-85) {  // Default RSSI threshold in dBm
        raw_samples.reserve(MAX_SAMPLES);
    }

    virtual ~CC1101DecoderBase() = default;

    void setCallback(DecoderCallback cb) {
        callback = std::move(cb);
    }

    void setRSSIThreshold(int8_t threshold_dbm) {
        rssi_threshold = threshold_dbm;
    }

    CarrierInfo detectCarrier() {
        CarrierInfo info;
        
        // Read frequency registers
        uint8_t freq2 = readCC1101Reg(CC1101_FREQ2);
        uint8_t freq1 = readCC1101Reg(CC1101_FREQ1);
        uint8_t freq0 = readCC1101Reg(CC1101_FREQ0);
        
        uint32_t freqReg = (freq2 << 16) | (freq1 << 8) | freq0;
        constexpr float FXOSC = 26000000.0;  // 26MHz crystal
        
        info.frequency = (uint32_t)((freqReg * FXOSC) / 65536.0);
        
        // Read and calculate frequency offset
        int8_t freqOffset = (int8_t)readCC1101Reg(CC1101_FREQOFF);
        info.freq_offset = freqOffset * 1600;  // Approximate offset in Hz
        
        // Read RSSI
        info.rssi = readCC1101Reg(CC1101_RSSI);
        if (info.rssi >= 128)
            info.rssi = ((info.rssi - 256) / 2) - 74;
        else
            info.rssi = (info.rssi / 2) - 74;

        // Read LQI
        uint8_t lqi = readCC1101Reg(CC1101_LQI) & 0x7F;
        info.lqi = (float)lqi / 127.0f;  // Normalize to 0-1

        // Check carrier sense
        uint8_t pktstatus = readCC1101Reg(CC1101_PKTSTATUS);
        info.carrier_present = (pktstatus & CC1101_CS_MASK) && (info.rssi > rssi_threshold);
        
        return info;
    }

    // Main timing handler - called from ISR
    __attribute__((iram_text)) void handleTiming(uint32_t timing) {
        if (timing < MIN_PULSE_LENGTH || timing > MAX_PULSE_LENGTH) {
            reset();
            return;
        }

        if (raw_samples.size() >= MAX_SAMPLES) {
            reset();
            return;
        }

        raw_samples.push_back(timing);
        
        if (tryDecode()) {
            DecodedSignal signal;
            signal.timings = raw_samples;
            signal.timestamp = esp_timer_get_time();
            signal.protocol_hash = calculateHash();
            getDecodedString(signal.decoded_string);
            
            // Get carrier information
            CarrierInfo carrier = detectCarrier();
            signal.frequency = carrier.frequency;
            signal.rssi = carrier.rssi;
            signal.carrier_present = carrier.carrier_present;
            
            if (callback) {
                callback(signal);
            }
            reset();
        }
    }

    virtual void reset() {
        signal_detected = false;
        raw_samples.clear();
        last_time = 0;
    }

    // Pure virtual methods to be implemented by specific protocol decoders
    virtual bool tryDecode() = 0;
    virtual uint32_t calculateHash() = 0;
    virtual void getDecodedString(std::string& output) = 0;

protected:
    // Utility methods for protocol implementations
    bool isTimingInRange(uint32_t timing, uint32_t expected, uint32_t tolerance = DEFAULT_TOLERANCE) {
        return (timing >= (expected - tolerance)) && 
               (timing <= (expected + tolerance));
    }

    bool findPreamble(const std::vector<uint32_t>& pattern, uint32_t tolerance = DEFAULT_TOLERANCE) {
        if (raw_samples.size() < pattern.size()) return false;
        
        for (size_t i = 0; i < pattern.size(); i++) {
            if (!isTimingInRange(raw_samples[i], pattern[i], tolerance)) {
                return false;
            }
        }
        return true;
    }

    // Template method for finding repeated patterns
    template<typename T>
    bool findRepeatedPattern(const std::vector<T>& data, size_t& pattern_length) {
        if (data.size() < 4) return false;  // Need minimum data for pattern detection
        
        for (pattern_length = 2; pattern_length <= data.size() / 2; pattern_length++) {
            bool pattern_found = true;
            for (size_t i = 0; i < pattern_length; i++) {
                for (size_t j = 1; j * pattern_length + i < data.size(); j++) {
                    if (data[i] != data[j * pattern_length + i]) {
                        pattern_found = false;
                        break;
                    }
                }
                if (!pattern_found) break;
            }
            if (pattern_found) return true;
        }
        return false;
    }

private:
    int8_t rssi_threshold;

    // Virtual method for CC1101 register access - implement in derived class
    virtual int readCC1101Reg(uint8_t addr) = 0;
};

// Example implementation for a specific protocol
class ExampleProtocolDecoder : public CC1101DecoderBase {
private:
    static constexpr uint32_t SHORT_PULSE = 400;
    static constexpr uint32_t LONG_PULSE = 800;
    std::vector<uint8_t> decoded_bits;

public:
    bool tryDecode() override {
        if (raw_samples.size() < 8) return false;
        
        decoded_bits.clear();
        for (size_t i = 0; i < raw_samples.size() - 1; i += 2) {
            if (isTimingInRange(raw_samples[i], SHORT_PULSE) &&
                isTimingInRange(raw_samples[i + 1], LONG_PULSE)) {
                decoded_bits.push_back(1);
            } else if (isTimingInRange(raw_samples[i], LONG_PULSE) &&
                      isTimingInRange(raw_samples[i + 1], SHORT_PULSE)) {
                decoded_bits.push_back(0);
            } else {
                return false;
            }
        }
        return decoded_bits.size() > 0;
    }

    uint32_t calculateHash() override {
        uint32_t hash = 0;
        for (uint8_t bit : decoded_bits) {
            hash = (hash << 1) | bit;
        }
        return hash;
    }

    void getDecodedString(std::string& output) override {
        std::stringstream ss;
        ss << "Protocol: Example\n";
        ss << "Bits: ";
        for (uint8_t bit : decoded_bits) {
            ss << static_cast<int>(bit);
        }
        output = ss.str();
    }
};