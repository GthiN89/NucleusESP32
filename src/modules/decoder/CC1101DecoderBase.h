#pragma once

#include <vector>
#include <functional>
#include <string>
#include <sstream>
#include <memory>
#include "driver/gpio.h"

namespace SubGHz {

class CC1101DecoderBase {
public:
    // Forward declarations for register addresses
    struct CC1101Registers {
        static constexpr uint8_t FREQ2 = 0x0D;
        static constexpr uint8_t FREQ1 = 0x0E;
        static constexpr uint8_t FREQ0 = 0x0F;
        static constexpr uint8_t FREQOFF = 0x0C;
        static constexpr uint8_t RSSI = 0x34;
        static constexpr uint8_t PKTSTATUS = 0x38;
        static constexpr uint8_t LQI = 0x33;
        static constexpr uint8_t MARCSTATE = 0x35;
    };

    struct DecodedSignal {
        std::vector<uint32_t> timings;
        uint64_t timestamp;
        uint32_t protocol_hash;
        std::string decoded_string;
        uint32_t frequency;
        int8_t rssi;
        bool carrier_present;
        float signal_quality;
    };

    struct CarrierInfo {
        uint32_t frequency;
        int8_t rssi;
        bool carrier_present;
        float lqi;
        int16_t freq_offset;
    };

    using DecoderCallback = std::function<void(const DecodedSignal&)>;

    // Constructor/Destructor declarations
    CC1101DecoderBase();
    virtual ~CC1101DecoderBase() = default;

    // Public interface
    void setCallback(DecoderCallback cb);
    void setRSSIThreshold(int8_t threshold_dbm);
    CarrierInfo detectCarrier();
    void __attribute__((iram_attr)) handleTiming(uint32_t timing);
    virtual void reset();

    // Configuration methods
    virtual void configure() = 0;
    virtual bool calibrate() = 0;

    // Pure virtual methods for protocol implementation
    virtual bool tryDecode() = 0;
    virtual uint32_t calculateHash() = 0;
    virtual void getDecodedString(std::string& output) = 0;

protected:
    // Constants
    static constexpr size_t MAX_SAMPLES = 1024;
    static constexpr uint32_t MIN_PULSE_LENGTH = 100;   // microseconds
    static constexpr uint32_t MAX_PULSE_LENGTH = 2500;  // microseconds
    static constexpr uint32_t DEFAULT_TOLERANCE = 120;  // microseconds
    static constexpr uint8_t CC1101_CS_MASK = 0x40;
    static constexpr float CRYSTAL_FREQ = 26000000.0f;  // 26MHz

    // Utility methods
    bool isTimingInRange(uint32_t timing, uint32_t expected, uint32_t tolerance = DEFAULT_TOLERANCE);
    bool findPreamble(const std::vector<uint32_t>& pattern, uint32_t tolerance = DEFAULT_TOLERANCE);
    
    template<typename T>
    bool findRepeatedPattern(const std::vector<T>& data, size_t& pattern_length);

    // Hardware access methods
    virtual uint8_t readCC1101Reg(uint8_t addr) = 0;
    virtual void writeCC1101Reg(uint8_t addr, uint8_t data) = 0;
    virtual uint8_t readCC1101Status(uint8_t addr) = 0;

    // Protected member variables
    DecoderCallback callback;
    uint64_t last_time;
    bool signal_detected;
    std::vector<uint32_t> raw_samples;
    int8_t rssi_threshold;

    // Signal quality metrics
    struct SignalMetrics {
        float avg_rssi;
        float peak_rssi;
        float lowest_rssi;
        uint32_t sample_count;
        float cumulative_lqi;
    } signal_metrics;

private:
    // Private helper methods
    float calculateSignalQuality();
    void updateSignalMetrics(int8_t rssi, float lqi);
    void resetSignalMetrics();
};

// Optional: Protocol registration interface
class ProtocolRegistry {
public:
    virtual void registerProtocol(const std::string& name, std::unique_ptr<CC1101DecoderBase> decoder) = 0;
    virtual CC1101DecoderBase* getDecoder(const std::string& name) = 0;
    virtual ~ProtocolRegistry() = default;
};

} // namespace SubGHz

#define REGISTER_PROTOCOL(name, class_name) \
    static bool registered_##class_name = []() { \
        ProtocolRegistry::instance().registerProtocol(name, std::make_unique<class_name>()); \
        return true; \
    }();