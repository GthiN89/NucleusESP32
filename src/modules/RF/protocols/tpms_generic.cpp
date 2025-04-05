#include "TPMSGenericData.h" // Include the header file we just created

#include <iostream> // Needed for cerr in helper, might remove later
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
//#include <vector> // No longer needed directly in this file
#include <cstdint>
#include <ctime>   // For std::time
#include <algorithm> // For std::find_if (used in trimming)

// --- Helper Function (Internal to this implementation file) ---

// Helper function to trim leading whitespace
// (Could be placed in an anonymous namespace or made static)
static inline std::string& ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    return s;
}

// Helper function to trim trailing whitespace
static inline std::string& rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
    return s;
}

// Helper function to trim both leading and trailing whitespace
static inline std::string& trim(std::string& s) {
    return ltrim(rtrim(s));
}


/**
 * @brief Helper function to find a key in a stream and read its corresponding value.
 * @param stream The input stream to read from.
 * @param key The key string to search for (case-sensitive).
 * @param value Output string where the found value will be stored (trimmed).
 * @return True if the key was found and value extracted, false otherwise.
 *
 * Note: This function currently rewinds the stream (`seekg(0)`) before searching,
 * mimicking FlipperFormat's behavior but inefficient for sequential reads on large files.
 * Consider adapting for sequential parsing if needed.
 */
static bool readKeyValuePair(std::istream& stream, const std::string& key, std::string& value) {
    std::string line;
    std::streampos originalPos = stream.tellg(); // Remember position

    stream.clear(); // Clear potential EOF/fail flags
    stream.seekg(0); // Rewind to search from the beginning

    std::string currentKey;
    char colon = ':'; // Expected separator

    while (std::getline(stream, line)) {
        std::size_t colonPos = line.find(colon);
        if (colonPos != std::string::npos) {
            currentKey = line.substr(0, colonPos);
            trim(currentKey); // Trim whitespace from the extracted key

            if (currentKey == key) {
                // Found the key, extract the value part
                value = line.substr(colonPos + 1);
                trim(value); // Trim whitespace from the extracted value

                // Restore original stream position (maybe not necessary depending on usage)
                // stream.clear();
                // stream.seekg(originalPos);
                return true; // Key found
            }
        }
    }
    // Key not found
    stream.clear(); // Clear EOF flag if reached
    stream.seekg(originalPos); // Restore position
    return false;
}

// --- TPMSGenericData Class Member Function Implementations ---

TPMSGenericData::TPMSGenericData() :
    protocolName("UnknownTPMS"), // Default protocol name
    id(0),
    dataCountBit(0),
    data(0),
    batteryLow(false),
    pressure(0.0f),
    temperature(0.0f),
    timestamp(0)
{}

/*************  ✨ Codeium Command ⭐  *************/
/******  eb15297f-5a80-4754-9d08-24472fbdd2c9  *******/

TPMSProcessingStatus TPMSGenericData::serialize(std::ostream& outputStream) const {
    // Note: Common fields like Protocol, Frequency, Preset are omitted here
    // as they were handled by a separate function in the original C code.
    // You might add them here or handle them separately depending on your file structure.

    // Write TPMS specific fields
    outputStream << "Id: " << id << std::endl;
    if (!outputStream.good()) return TPMSProcessingStatus::ErrorIO;

    // Cast dataCountBit to int for output, as uint8_t might print as a character
    outputStream << "Bit: " << static_cast<int>(dataCountBit) << std::endl;
    if (!outputStream.good()) return TPMSProcessingStatus::ErrorIO;

    // Write Data as hex, padded to 16 characters (8 bytes), uppercase
    outputStream << "Data: " << std::hex << std::uppercase << std::setw(16) << std::setfill('0') << data << std::dec << std::endl;
    if (!outputStream.good()) return TPMSProcessingStatus::ErrorIO;

    // Write Batt as 1 for true, 0 for false
    outputStream << "Batt: " << (batteryLow ? 1 : 0) << std::endl;
    if (!outputStream.good()) return TPMSProcessingStatus::ErrorIO;

    // Use fixed format and set precision for floats for consistent output
    outputStream << "Pressure: " << std::fixed << std::setprecision(3) << pressure << std::endl;
    if (!outputStream.good()) return TPMSProcessingStatus::ErrorIO;

    // Get current timestamp at the time of saving
    // Note: The original C code saved the *current* time, not necessarily the time
    // the data was originally captured if it was loaded from a file first.
    // This implementation mimics the C code's saving behavior.
    // If you want to preserve an older timestamp, ensure 'this->timestamp' holds it before calling serialize.
    uint32_t currentTimestamp = static_cast<uint32_t>(std::time(nullptr));
    outputStream << "Ts: " << currentTimestamp << std::endl;
    if (!outputStream.good()) return TPMSProcessingStatus::ErrorIO;

    outputStream << "Temp: " << std::fixed << std::setprecision(2) << temperature << std::endl;
    if (!outputStream.good()) return TPMSProcessingStatus::ErrorIO;

    // Reset float formatting to default
    outputStream.unsetf(std::ios_base::floatfield);
    outputStream << std::setprecision(6); // Default precision

    return outputStream.good() ? TPMSProcessingStatus::Ok : TPMSProcessingStatus::ErrorIO;
}


TPMSProcessingStatus TPMSGenericData::deserialize(std::istream& inputStream) {
    std::string valueStr;
    std::stringstream converter; // Use stringstream for robust conversions

    // Read Id
    if (!readKeyValuePair(inputStream, "Id", valueStr)) {
        // FURI_LOG_E(TAG, "Missing Id"); // Original log
        return TPMSProcessingStatus::ErrorFormat;
    }
    converter.str(valueStr);
    converter.clear(); // Important: clear state flags (like EOF) before reuse
    if (!(converter >> id)) {
        // FURI_LOG_E(TAG, "Invalid Id value");
        return TPMSProcessingStatus::ErrorFormat;
    }

    // Read Bit
    if (!readKeyValuePair(inputStream, "Bit", valueStr)) {
        // FURI_LOG_E(TAG, "Missing Bit");
        return TPMSProcessingStatus::ErrorFormat;
    }
    converter.str(valueStr);
    converter.clear();
    int tempBit; // Read into int first for range checking
    if (!(converter >> tempBit) || tempBit < 0 || tempBit > 255) {
         // FURI_LOG_E(TAG, "Invalid Bit value");
        return TPMSProcessingStatus::ErrorFormat;
    }
    dataCountBit = static_cast<uint8_t>(tempBit);

    // Read Data (Hex)
    if (!readKeyValuePair(inputStream, "Data", valueStr)) {
        // FURI_LOG_E(TAG, "Missing Data");
        return TPMSProcessingStatus::ErrorFormat;
    }
    converter.str(valueStr);
    converter.clear();
    if (!(converter >> std::hex >> data)) {
         // FURI_LOG_E(TAG, "Invalid Data value");
        return TPMSProcessingStatus::ErrorFormat;
    }

    // Read Batt
    if (!readKeyValuePair(inputStream, "Batt", valueStr)) {
        // FURI_LOG_E(TAG, "Missing Battery_low"); // Original key name in log
        return TPMSProcessingStatus::ErrorFormat;
    }
    converter.str(valueStr);
    converter.clear();
    int tempBatt;
    if (!(converter >> tempBatt)) {
         // FURI_LOG_E(TAG, "Invalid Batt value");
        return TPMSProcessingStatus::ErrorFormat;
    }
    batteryLow = (tempBatt != 0); // Treat any non-zero as true

    // Read Pressure
    if (!readKeyValuePair(inputStream, "Pressure", valueStr)) {
        // FURI_LOG_E(TAG, "Missing Pressure");
        return TPMSProcessingStatus::ErrorFormat;
    }
    converter.str(valueStr);
    converter.clear();
    if (!(converter >> pressure)) {
         // FURI_LOG_E(TAG, "Invalid Pressure value");
        return TPMSProcessingStatus::ErrorFormat;
    }

    // Read Ts (Timestamp)
    if (!readKeyValuePair(inputStream, "Ts", valueStr)) {
        // FURI_LOG_E(TAG, "Missing timestamp");
        // Decide if Ts is strictly required. The C code required it on load.
        return TPMSProcessingStatus::ErrorFormat;
    }
    converter.str(valueStr);
    converter.clear();
    if (!(converter >> timestamp)) {
         // FURI_LOG_E(TAG, "Invalid Ts value");
        return TPMSProcessingStatus::ErrorFormat;
    }

    // Read Temp
    if (!readKeyValuePair(inputStream, "Temp", valueStr)) {
        // FURI_LOG_E(TAG, "Missing Temperature"); // Original key name in log
        return TPMSProcessingStatus::ErrorFormat;
    }
    converter.str(valueStr);
    converter.clear();
    if (!(converter >> temperature)) {
        // FURI_LOG_E(TAG, "Invalid Temp value");
        return TPMSProcessingStatus::ErrorFormat;
    }

    // Optionally read Protocol Name if you include it in serialization
    // (Handling if it's missing)
    if (readKeyValuePair(inputStream, "Protocol", valueStr)) {
         protocolName = valueStr; // Assuming 'Protocol' is the key used in serialize
    } else {
        // Protocol key might not be present in all files, assign default or leave as is
        // protocolName = "UnknownTPMS"; // Or keep existing if already set
    }

    // Check stream state after all reads (though individual reads might have failed earlier)
    return inputStream.good() ? TPMSProcessingStatus::Ok : TPMSProcessingStatus::ErrorIO;
}


TPMSProcessingStatus TPMSGenericData::deserialize_check_count_bit(std::istream& inputStream, uint16_t expectedCountBit) {
    // First, attempt the standard deserialization
    TPMSProcessingStatus status = deserialize(inputStream);

    // If standard deserialization failed, return that error code immediately
    if (status != TPMSProcessingStatus::Ok) {
        return status;
    }

    // If deserialization succeeded, perform the bit count check
    if (dataCountBit != expectedCountBit) {
        // FURI_LOG_E(TAG, "Wrong number of bits in key"); // Original log
        return TPMSProcessingStatus::ErrorValueBitCount; // Return specific error for mismatch
    }

    // Deserialization was OK and the bit count matches
    return TPMSProcessingStatus::Ok;
}