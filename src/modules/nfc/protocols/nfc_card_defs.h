#ifndef NFC_CARD_DEFS_H // Renamed guard
#define NFC_CARD_DEFS_H

#include <Arduino.h>
#include <cstdint>
#include <cstddef>
#include <cstring>

namespace NFC {

// --- General Card Type Enum ---
enum class CardType {
    Unknown,
    MifareClassic,
    MifareUltralight,
    MifarePlus,     // ISO14443-4A compatible
    MifareDESFire,  // ISO14443-4A compatible
    ICODE,          // ISO15693 (Not handled here)
    Felica,         // (Not handled here)
    ISO14443_4A,    // Generic ISO14443-4A
    ISO14443_B,     // (Not handled here)
    // Add others as needed
};

// --- Mifare Classic Specific ---
namespace MfClassic {
    // Constants
    constexpr size_t BLOCK_SIZE = 16;
    constexpr size_t KEY_SIZE = 6;
    constexpr size_t ACCESS_BYTES_SIZE = 4;
    constexpr uint8_t MAX_SECTORS = 40;
    constexpr uint16_t MAX_BLOCKS = 256;
    constexpr size_t BLOCK_READ_MASK_SIZE = MAX_BLOCKS / 8;

    // Enums
    enum class Type { // Renamed from global Type
        Mini, Classic1k, Classic4k, Unknown
    };
    enum class KeyType : uint8_t { KeyA = 0, KeyB = 1 };
    enum class Action { /* ... Keep as before ... */ DataRead, TrailerRead };

    // Structs
    struct Key { uint8_t data[KEY_SIZE]; };
    union Block { uint8_t data[BLOCK_SIZE]; };
    struct AccessBits { uint8_t data[ACCESS_BYTES_SIZE]; };
    struct SectorTrailer { Key key_a; AccessBits access_bits; Key key_b; };

    // Card Data specific to Mifare Classic
    struct CardData {
        Type type = Type::Unknown;
        Block blocks[MAX_BLOCKS] = {0};
        uint64_t found_keys_a = 0;
        uint64_t found_keys_b = 0;
        uint8_t block_read_mask[BLOCK_READ_MASK_SIZE] = {0};
        uint64_t auth_key_type_mask = 0;
    };

    // Helper function declarations (implement in .cpp)
    uint8_t getTotalSectors(Type type);
    uint16_t getTotalBlocks(Type type);
    const char* getTypeName(Type type);
    uint8_t getSectorForBlock(uint16_t block_num);
    uint16_t getFirstBlockOfSector(uint8_t sector_num);
    uint16_t getSectorTrailerBlockNum(uint8_t sector_num);
    uint8_t getBlocksInSector(uint8_t sector_num);
    bool isSectorTrailer(uint16_t block_num);
    bool checkReadAccess(const AccessBits& ac_bits, uint16_t block_num, KeyType key_type);
    void initCardData(CardData& card_data);
    void setBlockReadFlag(CardData& card_data, uint16_t block_num, bool read);
    bool getBlockReadFlag(const CardData& card_data, uint16_t block_num);
    void setKeyFoundFlag(CardData& card_data, uint8_t sector_num, KeyType key_type, bool found);
    bool getKeyFoundFlag(const CardData& card_data, uint8_t sector_num, KeyType key_type);
    void setAuthKeyType(CardData& card_data, uint8_t sector_num, KeyType key_type);
    KeyType getAuthKeyType(const CardData& card_data, uint8_t sector_num);

} // namespace MfClassic

// --- Mifare Ultralight Specific ---
namespace MfUltralight {
    // Basic types based on SAK/ATQA (can be expanded)
    enum class Type {
        Ultralight, // SAK 0x00, ATQA 0x4400
        NTAG2xx,    // SAK 0x00, ATQA 0x4400 (Needs GET_VERSION)
        UltralightC,// SAK 0x00, ATQA 0x4400 (Needs specific command)
        Unknown
    };
    // Basic data - only UID/SAK/ATQA relevant for now
    struct CardData {
        Type type = Type::Unknown;
        // Add version, signature etc. later if GET_VERSION implemented
    };
    const char* getTypeName(Type type);
} // namespace MfUltralight


// --- Mifare DESFire Specific ---
namespace MfDESFire {
    // Basic type - identification relies on ISO14443-4 selection + GET_VERSION
     enum class Type {
        EV1, EV2, EV3, Unknown // Add others if needed
    };
     struct CardData {
        Type type = Type::Unknown;
        // Add version later if GET_VERSION implemented
    };
    const char* getTypeName(Type type);
} // namespace MfDESFire

// --- Mifare Plus Specific ---
namespace MfPlus {
     // Identification relies on ISO14443-4 selection + specific commands/responses
     enum class Type {
        S, X, SE, EV1, EV2, Unknown
    };
     enum class SecurityLevel {
        SL0, SL1, SL2, SL3, Unknown
    };
     struct CardData {
        Type type = Type::Unknown;
        SecurityLevel sl = SecurityLevel::Unknown;
        // Add version etc. later if commands implemented
    };
     const char* getTypeName(Type type);
} // namespace MfPlus


// --- Generic Card Info Structure ---
// Holds basic ISO14443-3A info and the identified higher-level type
struct NfcCard {
    // ISO14443-3A Data
    uint8_t uid[7] = {0};
    uint8_t uid_len = 0;
    uint8_t atqa[2] = {0}; // SENS_RES
    uint8_t sak = 0;     // SEL_RES

    // Identified Card Type
    CardType general_type = CardType::Unknown;

    // Specific Card Data (using a union might save memory if needed,
    // but separate structs are simpler for now)
    MfClassic::CardData mf_classic_data;
    MfUltralight::CardData mf_ul_data;
    MfDESFire::CardData mf_desfire_data;
    MfPlus::CardData mf_plus_data;
    // Add other card type data structs here...

    // Constructor to initialize
    NfcCard() { reset(); }

    void reset() {
        memset(uid, 0, sizeof(uid));
        uid_len = 0;
        memset(atqa, 0, sizeof(atqa));
        sak = 0;
        general_type = CardType::Unknown;
        MfClassic::initCardData(mf_classic_data);
        mf_ul_data.type = MfUltralight::Type::Unknown;
        mf_desfire_data.type = MfDESFire::Type::Unknown;
        mf_plus_data.type = MfPlus::Type::Unknown;
        // Reset other specific data...
    }
};


} // namespace NFC

#endif // NFC_CARD_DEFS_H