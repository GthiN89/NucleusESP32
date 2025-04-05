#ifndef KEELOQ_PROTOCOL_HPP
#define KEELOQ_PROTOCOL_HPP

#include <vector>
#include <string>
#include <cstdint>
#include <memory> // For unique_ptr maybe
#include <iostream> // Added

#include "KeeLoqData.hpp"
#include "KeeLoqCommon.hpp"

// Define a structure for pulses if not already available
struct Pulse {
    bool level;
    uint32_t duration;
};

// --- KeeLoq Decoder Class ---

class KeeLoqProtocolDecoder {
public:
    KeeLoqProtocolDecoder();
    ~KeeLoqProtocolDecoder(); // Add destructor if needed

    /**
     * @brief Resets the decoder state machine.
     */
    void reset();

    /**
     * @brief Feeds the decoder with the next level and duration from the signal.
     * @param level true for high, false for low.
     * @param duration Duration in microseconds.
     */
    void feed(bool level, uint32_t duration);

    /**
     * @brief Checks if a complete and valid packet has been decoded.
     * @return True if a valid packet is available, false otherwise.
     */
    bool hasResult() const;

    /**
     * @brief Attempts to decrypt the last received packet and get the decoded data.
     * @param result Output parameter where the KeeLoqData will be stored if successful.
     * @param provided_mf_key The manufacturer key to attempt decryption with.
     * @param provided_learning_type The learning type associated with the key.
     * @return KeeLoqStatus::Ok if decryption succeeded and data seems valid, ErrorDecrypt otherwise.
     */
    KeeLoqStatus getResult(
        KeeLoqData& result,
        uint64_t provided_mf_key,
        KeeLoqCommon::KeeLoqLearningType provided_learning_type,
        const std::string& manufacturer_name_hint /*= ""*/);

    // Basic serialize/deserialize for decoder state + result (if needed) - Stub
    KeeLoqStatus serialize(std::ostream& outputStream) const;
    KeeLoqStatus deserialize(std::istream& inputStream);


private:
    // Timing constants (taken from subghz_protocol_keeloq_const)
    static constexpr uint32_t TE_SHORT = 400;
    static constexpr uint32_t TE_LONG = 800;
    static constexpr uint32_t TE_DELTA = 200;
    static constexpr uint8_t MIN_COUNT_BIT = 64; // Expected data bits
    static constexpr uint8_t MAX_COUNT_BIT = 66; // Allow for 2 status bits

    // Decoder state machine steps
    enum class DecoderStep {
        Reset,
        CheckPreambula,
        SaveDuration,
        CheckDuration
    };

    DecoderStep parser_step_ = DecoderStep::Reset;
    uint16_t header_count_ = 0;
    uint64_t current_data_ = 0;       // Raw data being assembled bit-by-bit
    uint8_t decoded_bit_count_ = 0;
    uint32_t last_duration_ = 0;

    KeeLoqData last_decoded_packet_; // Store the last successfully received raw packet
    bool has_valid_packet_ = false;

    /**
     * @brief Helper to check if a duration matches TE_SHORT.
     */
    bool isDurationShort(uint32_t duration) const;

    /**
     * @brief Helper to check if a duration matches TE_LONG.
     */
    bool isDurationLong(uint32_t duration) const;

    /**
     * @brief Adds a bit to the currently assembling data.
     */
    void addBit(bool bit);

     /**
      * @brief Internal helper to check decrypted data validity (Simplified version).
      * Needs the decrypted hop part, expected button, and expected serial part.
      */
         bool checkDecryptedData(uint32_t decrypted_hop, uint8_t expected_btn, uint32_t expected_serial_part, uint16_t& out_counter, const std::string& manufacturer_name_hint);
};


// --- KeeLoq Encoder Class ---

class KeeLoqProtocolEncoder {
public:
    KeeLoqProtocolEncoder();
    ~KeeLoqProtocolEncoder();

    /**
     * @brief Loads the data to be encoded and transmitted.
     * @param data The KeeLoqData object containing serial, button, initial counter, etc.
     *        Requires manufacturer_key and learning_type to be set correctly within 'data'.
     * @param repeats Number of times to repeat the transmission.
     * @return KeeLoqStatus::Ok if data is valid and loaded, error otherwise.
     */
    KeeLoqStatus loadData(const KeeLoqData& data, uint32_t repeats = 5);

    /**
     * @brief Stops the transmission process.
     */
    void stop();

    /**
     * @brief Gets the next pulse (level and duration) for transmission.
     *        Automatically handles repeats and counter incrementing.
     * @return A Pulse object. Returns {false, 0} when transmission is complete.
     */
    Pulse yield();

private:
    // Timing constants (same as decoder)
    static constexpr uint32_t TE_SHORT = 400;
    static constexpr uint32_t TE_LONG = 800;

    KeeLoqData data_to_encode_;
    uint32_t repeats_remaining_ = 0;
    bool is_running_ = false;

    std::vector<Pulse> pulse_buffer_;
    size_t current_pulse_index_ = 0;

    /**
     * @brief Generates the next KeeLoq encrypted payload (updates hop part).
     *        Increments the counter in data_to_encode_.
     * @return True if successful, false otherwise (e.g., no key).
     */
    bool generateNextPayload();

    /**
     * @brief Generates the pulse sequence for the current payload in data_to_encode_.
     *        Fills the pulse_buffer_.
     * @return True if successful, false otherwise.
     */
    bool generateUpload();


};
#endif // KEELOQ_PROTOCOL_HPP