#ifndef MFCLASSICPOLLER_HPP
#define MFCLASSICPOLLER_HPP

#include <Arduino.h>
#include <Adafruit_PN532.h>

// Define Mifare Classic card types (simplified)
enum MfClassicType {
    MfClassicType1k,
    MfClassicType4k, // Note: Detection between 1k/4k/Mini is unreliable without SAK/Auth
    MfClassicTypeMini,
    MfClassicTypeNum // Represents "Unknown" or "Not Classic"
};

// Define Mifare Classic Key types for authentication
enum MfClassicKeyType : uint8_t {
    MfClassicKeyA = 0,
    MfClassicKeyB = 1
};

// Define a structure/union for a 16-byte Mifare block
typedef union {
    uint8_t data[16];
    uint32_t data_uint32[4];
    // Add other interpretations if needed
} MfClassicBlock;


/**
 * @brief A helper class to poll for Mifare Classic cards and perform basic operations.
 */
class MfClassicPoller {
public:
    /**
     * @brief Constructor.
     * @param reader Pointer to an initialized Adafruit_PN532 instance.
     */
    MfClassicPoller(Adafruit_PN532* reader);

    // No explicit begin() needed if initialization happens outside

    /**
     * @brief Tries to detect a Mifare Classic card within a timeout.
     * Populates internal UID and UID length if successful.
     * NOTE: Type detection (1k/4k/Mini) is simplified and assumes 1k for 4-byte UIDs.
     * @return The detected card type (MfClassicType1k if 4-byte UID found, MfClassicTypeNum otherwise).
     */
    MfClassicType detectType();

    /**
     * @brief Authenticates a specific block using the stored UID and provided key.
     * Assumes detectType() was called successfully before this.
     * @param blockNum The block number to authenticate (0-63 for 1k, 0-255 for 4k).
     * @param key Pointer to the 6-byte key array.
     * @param keyType MfClassicKeyA or MfClassicKeyB.
     * @return true if authentication was successful, false otherwise.
     */
    bool authenticateBlock(uint8_t blockNum, uint8_t* key, MfClassicKeyType keyType);

    /**
     * @brief Reads a 16-byte block from the card into the provided buffer.
     * Requires prior successful authentication of the sector containing the block.
     * @param blockNum The block number to read.
     * @param blockData Pointer to an MfClassicBlock structure to store the data.
     * @return true if the read was successful, false otherwise.
     */
    bool readBlock(uint8_t blockNum, MfClassicBlock* blockData);

    /**
     * @brief Writes 16 bytes from the provided buffer to a block on the card.
     * Requires prior successful authentication of the sector containing the block.
     * Be careful! This can permanently alter card data or brick it if access bits are changed incorrectly.
     * @param blockNum The block number to write.
     * @param blockData Pointer to an MfClassicBlock structure containing the data to write.
     * @return true if the write was successful, false otherwise.
     */
    bool writeBlock(uint8_t blockNum, MfClassicBlock* blockData);

    /**
     * @brief Dumps the detected card's UID and attempts to read all blocks (0-63)
     *        to Serial, using the universal Key B for authentication.
     * Assumes detectType() was called successfully before this.
     */
    void dumpToSerial();

    // --- Getters ---
    MfClassicType getCardType() const { return cardType; }
    const uint8_t* getUID() const { return uid; }
    uint8_t getUIDLength() const { return uidLength; }

private:
    Adafruit_PN532* nfc = nullptr; // Pointer to the main NFC driver instance
    MfClassicType cardType = MfClassicTypeNum;
    uint8_t uid[7] = {0}; // Buffer for card UID
    uint8_t uidLength = 0;
};

#endif // MFCLASSICPOLLER_HPP