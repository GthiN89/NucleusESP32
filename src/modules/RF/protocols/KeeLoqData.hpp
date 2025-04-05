#ifndef KEELOQ_DATA_HPP
#define KEELOQ_DATA_HPP

#include <cstdint>
#include <string>
#include <vector>   // For simple serialization/deserialization helpers
#include <sstream>  // For string conversions
#include <iomanip>  // For hex formatting
#include <iostream> // For serialize/deserialize stubs
#include "KeeLoqCommon.hpp" // Include the common functions header

// Basic status enum
enum class KeeLoqStatus {
    Ok,
    Error,
    ErrorIO,
    ErrorFormat,
    ErrorDecrypt, // Specific error if decryption fails or doesn't match
    ErrorNoKey,   // Specific error if key is missing for an operation
    ErrorNotSupported // Feature/Type not supported
};

// Helper to simulate FlipperFormat-like key-value reading (basic version)
// Note: Copied from TPMS port, assumes stream supports seekg(0) for rewind.
// Adapt if sequential parsing is required.
static bool readKVP(std::istream& stream, const std::string& key, std::string& value) {
    std::string line;
    std::streampos originalPos = stream.tellg();
    stream.clear();
    stream.seekg(0);
    std::string currentKey;
    char colon = ':';
    while (std::getline(stream, line)) {
        std::size_t colonPos = line.find(colon);
        if (colonPos != std::string::npos) {
            currentKey = line.substr(0, colonPos);
            // Basic trim
            currentKey.erase(0, currentKey.find_first_not_of(" \t"));
            currentKey.erase(currentKey.find_last_not_of(" \t") + 1);

            if (currentKey == key) {
                value = line.substr(colonPos + 1);
                 // Basic trim
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);
                // stream.seekg(originalPos); // Optional: restore position
                return true;
            }
        }
    }
    stream.clear();
    stream.seekg(originalPos);
    return false;
}


struct KeeLoqData {
    // --- Core Data ---
    uint64_t data = 0;
    uint8_t data_count_bit = 64;

    // --- Decoded/Input Data ---
    uint32_t serial = 0;
    uint8_t btn = 0;
    uint16_t cnt = 0;
    uint32_t seed = 0;

    // --- Keying Material & Metadata ---
    std::string manufacturer_name = "Unknown";
    uint64_t manufacturer_key = 0; // MUST be provided for encoding/decoding most types
    KeeLoqCommon::KeeLoqLearningType learning_type = KeeLoqCommon::KeeLoqLearningType::Unknown;

    // --- Parsed Components (derived from raw data) ---
    uint32_t fix_part = 0;
    uint32_t hop_part = 0;

    bool isValid() const {
        return data_count_bit == 64;
    }

    // Helper to reverse bits in a 64-bit number
    static uint64_t reverseBits(uint64_t n) {
        uint64_t reversed = 0;
        for (int i = 0; i < 64; ++i) {
            reversed <<= 1;
            if (n & 1) reversed ^= 1;
            n >>= 1;
        }
        return reversed;
    }

    // Update fix/hop parts and basic serial/btn from the raw 'data' field
    void updateDerivedPartsFromData() {
        if (data_count_bit == 64) {
            uint64_t reversed_data = reverseBits(data); // Use the static helper
            fix_part = static_cast<uint32_t>(reversed_data >> 32);
            hop_part = static_cast<uint32_t>(reversed_data & 0xFFFFFFFFULL);
            // Also update primary serial/btn based on fix_part immediately
            serial = fix_part & 0x0FFFFFFF; // Assume 28 bit serial base
            btn = static_cast<uint8_t>(fix_part >> 28);
        } else {
            fix_part = 0;
            hop_part = 0;
        }
    }

    // --- Serialization/Deserialization (More complete example) ---
    KeeLoqStatus serialize(std::ostream& outputStream) const {
        outputStream << "Protocol: KeeLoq" << std::endl; // Consistent name
        outputStream << "Bit: " << static_cast<int>(data_count_bit) << std::endl;
        outputStream << "Key: " << std::hex << std::uppercase << std::setw(16) << std::setfill('0') << data << std::dec << std::endl;
        outputStream << "Serial: " << std::hex << std::uppercase << std::setw(7) << std::setfill('0') << (serial & 0x0FFFFFFF) << std::dec << std::endl; // Save 28 bits
        outputStream << "Btn: " << static_cast<int>(btn) << std::endl;
        outputStream << "Cnt: " << cnt << std::endl;
        outputStream << "Seed: " << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << seed << std::dec << std::endl;
        outputStream << "Manufacture: " << manufacturer_name << std::endl;
        // Optionally save the key and type if known and needed for reload
        // outputStream << "MF_Key: " << std::hex << manufacturer_key << std::dec << std::endl;
        // outputStream << "Learn_Type: " << static_cast<int>(learning_type) << std::endl;

        return outputStream.good() ? KeeLoqStatus::Ok : KeeLoqStatus::ErrorIO;
    }

    KeeLoqStatus deserialize(std::istream& inputStream) {
         std::string valueStr;
         std::stringstream converter;
         uint32_t temp_u32;
         int temp_int;

        // Assuming basic Key: Value format
        if (!readKVP(inputStream, "Bit", valueStr)) return KeeLoqStatus::ErrorFormat;
        converter.str(valueStr); converter.clear();
        if (!(converter >> temp_int) || temp_int != 64) return KeeLoqStatus::ErrorFormat; // Only support 64 bit for now
        data_count_bit = static_cast<uint8_t>(temp_int);

        if (!readKVP(inputStream, "Key", valueStr)) return KeeLoqStatus::ErrorFormat;
        converter.str(valueStr); converter.clear();
        if (!(converter >> std::hex >> data)) return KeeLoqStatus::ErrorFormat;

        if (!readKVP(inputStream, "Serial", valueStr)) return KeeLoqStatus::ErrorFormat;
        converter.str(valueStr); converter.clear();
        if (!(converter >> std::hex >> serial)) return KeeLoqStatus::ErrorFormat;
        serial &= 0x0FFFFFFF; // Ensure only 28 bits

        if (!readKVP(inputStream, "Btn", valueStr)) return KeeLoqStatus::ErrorFormat;
        converter.str(valueStr); converter.clear();
        if (!(converter >> temp_int) || temp_int < 0 || temp_int > 15) return KeeLoqStatus::ErrorFormat;
        btn = static_cast<uint8_t>(temp_int);

        if (!readKVP(inputStream, "Cnt", valueStr)) return KeeLoqStatus::ErrorFormat;
        converter.str(valueStr); converter.clear();
        if (!(converter >> temp_u32) || temp_u32 > 0xFFFF) return KeeLoqStatus::ErrorFormat;
        cnt = static_cast<uint16_t>(temp_u32);

        // Seed might be optional in some files
        if (readKVP(inputStream, "Seed", valueStr)) {
            converter.str(valueStr); converter.clear();
            if (!(converter >> std::hex >> seed)) return KeeLoqStatus::ErrorFormat;
        } else {
            seed = 0; // Default if missing
        }

        if (!readKVP(inputStream, "Manufacture", valueStr)) return KeeLoqStatus::ErrorFormat;
        manufacturer_name = valueStr;

        // Update derived parts after loading raw data
        updateDerivedPartsFromData();

        // Note: Does not load manufacturer_key or learning_type - these must be set externally if needed.
        learning_type = KeeLoqCommon::KeeLoqLearningType::Unknown;
        manufacturer_key = 0;

        return KeeLoqStatus::Ok;
    }

        // --- Formatting ---
    /**
     * @brief Generates a formatted string representation of the KeeLoq data.
     * @return std::string containing the formatted data.
     */
    std::string getCodeString() const; // Declaration added

};

#endif // KEELOQ_DATA_HPP