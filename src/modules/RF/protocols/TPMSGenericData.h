#ifndef TPMS_GENERIC_DATA_H
#define TPMS_GENERIC_DATA_H

#include <string>
#include <cstdint>
#include <iostream> // For std::ostream, std::istream forward declarations are sufficient
                    // if you only use pointers/references, but full include is common.

// Define status enum similar to SubGhzProtocolStatus for clarity
// This enum defines the possible outcomes of serialization/deserialization operations.
enum class TPMSProcessingStatus {
    Ok,                // Operation completed successfully
    Error,             // Generic error
    ErrorIO,           // Input/Output stream error during read/write
    ErrorFormat,       // Incorrect format detected (e.g., missing key, bad value type)
    ErrorValueBitCount // Specific error: The bit count read doesn't match an expected value
};

/**
 * @brief Class to store and manage data fields common to many TPMS protocols.
 *
 * This class encapsulates the typical information extracted from a TPMS sensor reading,
 * such as sensor ID, raw data, pressure, temperature, etc. It also provides
 * methods to serialize this data to a text-based key-value format (similar to
 * Flipper .sub files) and deserialize it back from such a format.
 */
class TPMSGenericData {
public:
    // --- Public Member Variables ---
    // Store the essential data fields for a TPMS reading.

    std::string protocolName; // Name of the specific TPMS protocol (e.g., "Schrader", "Toyota")
    uint32_t id;              // Unique identifier of the TPMS sensor
    uint8_t dataCountBit;     // The number of bits in the 'data' field that are relevant
    uint64_t data;            // Raw data payload received from the sensor (often needs protocol-specific parsing)
    bool batteryLow;        // Flag indicating if the sensor reported a low battery condition
    float pressure;           // Pressure reading (units are protocol-dependent, e.g., kPa, PSI)
    float temperature;        // Temperature reading (units are protocol-dependent, e.g., Celsius, Fahrenheit)
    uint32_t timestamp;       // Unix timestamp, typically indicating when the data was saved or last received

    // --- Constructor ---
    /**
     * @brief Default constructor. Initializes members to default values (zeros, false, empty string).
     */
    TPMSGenericData();

    // --- Serialization Method ---
    /**
     * @brief Writes the object's data to an output stream in a key-value text format.
     *
     * Example output format:
     * Id: 12345678
     * Bit: 64
     * Data: AABBCCDDEEFF0011
     * Batt: 0
     * Pressure: 35.500
     * Ts: 1678886400
     * Temp: 22.50
     *
     * @param outputStream The std::ostream (e.g., std::ofstream, std::stringstream) to write to.
     * @return TPMSProcessingStatus indicating success (Ok) or failure (e.g., ErrorIO).
     *         The method is const as it does not modify the object's state.
     */
    TPMSProcessingStatus serialize(std::ostream& outputStream) const;

    // --- Deserialization Methods ---
    /**
     * @brief Reads and parses data from an input stream, populating the object's members.
     *
     * Expects the input stream to contain data in the key-value text format
     * produced by the serialize method or compatible formats (like Flipper .sub files
     * for TPMS). It searches for the required keys (Id, Bit, Data, Batt, Pressure, Ts, Temp).
     *
     * @param inputStream The std::istream (e.g., std::ifstream, std::stringstream) to read from.
     * @return TPMSProcessingStatus indicating success (Ok) or failure (e.g., ErrorFormat, ErrorIO).
     */
    TPMSProcessingStatus deserialize(std::istream& inputStream);

    /**
     * @brief Deserializes data from the input stream and additionally verifies the bit count.
     *
     * Calls the base deserialize method first. If successful, it then checks if the
     * 'dataCountBit' member matches the 'expectedCountBit' parameter.
     *
     * @param inputStream The std::istream to read from.
     * @param expectedCountBit The specific bit count expected for the protocol being parsed.
     * @return TPMSProcessingStatus:
     *         - Ok if deserialization succeeded and the bit count matches.
     *         - ErrorValueBitCount if deserialization succeeded but the bit count does not match.
     *         - Other error codes (ErrorFormat, ErrorIO) if the base deserialization fails.
     */
    TPMSProcessingStatus deserialize_check_count_bit(std::istream& inputStream, uint16_t expectedCountBit);

    // Note: No destructor needed as we are using standard types (std::string handles its own memory).
};

#endif // TPMS_GENERIC_DATA_H