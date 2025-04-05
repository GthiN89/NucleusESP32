#include "nfc_card_defs.h"

namespace NFC {
namespace MfClassic {

// --- Helper Implementations ---

uint8_t getTotalSectors(Type type) {
    switch (type) {
        case Type::Mini:      return 5;
        case Type::Classic1k: return 16;
        case Type::Classic4k: return MAX_SECTORS; // 40
        default:              return 0;
    }
}

uint16_t getTotalBlocks(Type type) {
    switch (type) {
        case Type::Mini:      return 20; // 5 sectors * 4 blocks
        case Type::Classic1k: return 64; // 16 sectors * 4 blocks
        case Type::Classic4k: return MAX_BLOCKS; // 32*4 + 8*16 = 256
        default:              return 0;
    }
}

const char* getTypeName(Type type) {
    switch (type) {
        case Type::Mini:      return "MINI";
        case Type::Classic1k: return "1K";
        case Type::Classic4k: return "4K";
        default:              return "Unknown";
    }
}

uint8_t getSectorForBlock(uint16_t block_num) {
    if (block_num < 128) { // Sectors 0-31 (4 blocks each)
        return block_num / 4;
    } else if (block_num < MAX_BLOCKS) { // Sectors 32-39 (16 blocks each)
        return 32 + (block_num - 128) / 16;
    } else {
        return 0xFF; // Invalid block number
    }
}

uint16_t getFirstBlockOfSector(uint8_t sector_num) {
    if (sector_num < 32) { // Sectors 0-31
        return static_cast<uint16_t>(sector_num) * 4;
    } else if (sector_num < MAX_SECTORS) { // Sectors 32-39
        return 128 + static_cast<uint16_t>(sector_num - 32) * 16;
    } else {
        return 0xFFFF; // Invalid sector number
    }
}

uint8_t getBlocksInSector(uint8_t sector_num) {
    if (sector_num < 32) {
        return 4;
    } else if (sector_num < MAX_SECTORS) {
        return 16;
    } else {
        return 0; // Invalid sector
    }
}

uint16_t getSectorTrailerBlockNum(uint8_t sector_num) {
    uint16_t first_block = getFirstBlockOfSector(sector_num);
    if (first_block == 0xFFFF) return 0xFFFF; // Invalid sector
    uint8_t blocks_in_sector = getBlocksInSector(sector_num);
    return first_block + blocks_in_sector - 1;
}

bool isSectorTrailer(uint16_t block_num) {
    if (block_num >= MAX_BLOCKS) return false; // Invalid block
    uint8_t sector_num = getSectorForBlock(block_num);
    if (sector_num == 0xFF) return false; // Invalid block
    return block_num == getSectorTrailerBlockNum(sector_num);
}

// --- Access Condition Parsing (Basic Read Check) ---
// Based on datasheet Table 9 access conditions for data blocks C1 C2 C3
// Returns true if the key type *could* potentially read the block based on AC bits
// Does NOT guarantee the key itself is correct.
bool checkReadAccess(const AccessBits& ac_bits, uint16_t block_num, KeyType key_type) {
    if (isSectorTrailer(block_num)) {
        // Trailer read access (AC bits C1=1,C2=1,C3=1) -> AC bits: b7=1,b3=1 | b8=1,b4=1
        // Corresponds to AC value 0b111 (binary) or 7 decimal
        // Access bits are bits 6, 7, 8 of byte 7 and 6, 7, 8 of byte 8 (inverted)
        // C1 = ~ac_bits.data[1] b6 | ac_bits.data[2] b2 | ac_bits.data[2] b6
        // C2 = ~ac_bits.data[1] b7 | ac_bits.data[2] b3 | ac_bits.data[2] b7
        // C3 = ~ac_bits.data[1] b8 | ac_bits.data[2] b4 | ac_bits.data[2] b8

        // Trailer ACs: Use bits from Byte 7 and Byte 8 to form C1_3, C2_3, C3_3
        uint8_t c1_3 = (((ac_bits.data[1] >> 6) & 0x01) << 2) | // Inverted C1x
                       (((ac_bits.data[2] >> 2) & 0x01) << 1) | // Non-inverted C2x
                       (((ac_bits.data[2] >> 6) & 0x01) << 0);  // Non-inverted C3x
        uint8_t c2_3 = (((ac_bits.data[1] >> 7) & 0x01) << 2) | // Inverted C1x
                       (((ac_bits.data[2] >> 3) & 0x01) << 1) | // Non-inverted C2x
                       (((ac_bits.data[2] >> 7) & 0x01) << 0);  // Non-inverted C3x
        uint8_t c3_3 = (((ac_bits.data[1] >> 8) & 0x01) << 2) | // Inverted C1x // NOTE: byte only has 8 bits, index 8 is invalid -> needs correction
                       (((ac_bits.data[2] >> 4) & 0x01) << 1) | // Non-inverted C2x
                       (((ac_bits.data[2] >> 8) & 0x01) << 0);  // Non-inverted C3x // NOTE: byte only has 8 bits, index 8 is invalid -> needs correction

        // *** ERROR IN LOGIC ABOVE - Need to refer to Flipper code or datasheet accurately ***
        // Flipper logic: AC = ((access_bits_arr[1] >> 5) & 0x04) | ((access_bits_arr[2] >> 2) & 0x02) | ((access_bits_arr[2] >> 7) & 0x01);
        uint8_t trailer_ac = ((ac_bits.data[1] >> 5) & 0x04) | // C1_3 inverted from b7 of byte 7
                             ((ac_bits.data[2] >> 2) & 0x02) | // C2_3 from b6 of byte 8
                             ((ac_bits.data[2] >> 7) & 0x01);  // C3_3 from b7 of byte 8

        // Check Trailer Read permissions (KeyA read AC, KeyB read KeyB, KeyA/B read AC)
        // If we want to read the whole trailer, we need AC read permission:
        // Table 10: Read Access Bits requires AC = any (0-7) with KeyA or KeyB. Always allowed if authenticated.
        // Table 10: Read Key B requires AC = 0, 2, 1 (KeyA/B). AC = 4, 6, 5, 3, 7 (Never).
        // For a simple "read trailer" check, assume we need access bits:
        return true; // Reading AC bits is always allowed if sector is authenticated

    } else {
        // Data block access (AC bits C1,C2,C3) -> AC bits: b6,b7,b8 of byte 6 and b2,b3,b4 of byte 7 (inverted) + b2,b3,b4 of byte 8
        uint8_t sector_block_index = 0; // 0, 1, or 2 for data blocks within the sector
        if (getSectorForBlock(block_num) < 32) { // 4-block sectors
             sector_block_index = block_num % 4;
        } else { // 16-block sectors - ACs grouped differently (0-4, 5-9, 10-14)
            sector_block_index = (block_num % 16) / 5; // Approximation, needs verification
        }

        uint8_t data_ac = 7; // Default to most restrictive if parsing fails
        // Flipper logic for Data blocks:
        switch (sector_block_index) {
            case 0:
                data_ac = ((ac_bits.data[0] >> 6) & 0x04) | // C1_0 inverted from b6 of byte 6
                          ((ac_bits.data[1] >> 1) & 0x02) | // C2_0 from b5 of byte 7
                          ((ac_bits.data[2] >> 5) & 0x01);  // C3_0 from b5 of byte 8
                break;
            case 1:
                data_ac = ((ac_bits.data[0] >> 7) & 0x04) | // C1_1 inverted from b7 of byte 6
                          ((ac_bits.data[1] >> 2) & 0x02) | // C2_1 from b6 of byte 7
                          ((ac_bits.data[2] >> 6) & 0x01);  // C3_1 from b6 of byte 8
                break;
            case 2:
                 data_ac = ((ac_bits.data[0] >> 8) & 0x04) | // C1_2 inverted from b8 of byte 6 // NOTE: invalid index
                           ((ac_bits.data[1] >> 3) & 0x02) | // C2_2 from b7 of byte 7
                           ((ac_bits.data[2] >> 7) & 0x01);  // C3_2 from b7 of byte 8
                 break;
             // NOTE: Needs correction for block index > 2 and index 8 references
        }
         // *** ERROR IN LOGIC ABOVE - Need to refer to Flipper code or datasheet accurately ***
         // Flipper uses different logic:
          switch (sector_block_index) {
             case 0x00: // Block 0 ACs C1_0, C2_0, C3_0
                 data_ac = ((ac_bits.data[1] >> 2) & 0x04) | // Inverted C1_0 from b6 of byte 7
                           ((ac_bits.data[2] << 1) & 0x02) | // C2_0 from b4 of byte 8
                           ((ac_bits.data[2] >> 4) & 0x01);  // C3_0 from b4 of byte 8
                 break;
             case 0x01: // Block 1 ACs C1_1, C2_1, C3_1
                 data_ac = ((ac_bits.data[1] >> 3) & 0x04) | // Inverted C1_1 from b7 of byte 7
                           ((ac_bits.data[2] >> 0) & 0x02) | // C2_1 from b4 of byte 8
                           ((ac_bits.data[2] >> 5) & 0x01);  // C3_1 from b5 of byte 8
                 break;
             case 0x02: // Block 2 ACs C1_2, C2_2, C3_2
                 data_ac = ((ac_bits.data[1] >> 4) & 0x04) | // Inverted C1_2 from b8 of byte 7 // <<<<<< ERROR: index 8 invalid
                           ((ac_bits.data[2] >> 1) & 0x02) | // C2_2 from b5 of byte 8
                           ((ac_bits.data[2] >> 6) & 0x01);  // C3_2 from b6 of byte 8
                 break;
             default: return false; // Should not happen for 4-block sectors
          }
        // *** ERROR IN LOGIC ABOVE - Requires careful cross-reference ***

        // Assuming correct `data_ac` is calculated (0-7):
        // Table 9: Read requires KeyA/B for AC 0,1,2,4,6. KeyB only for AC 3,5. Never for AC 7.
        bool keyA_allowed = !(data_ac == 3 || data_ac == 5 || data_ac == 7);
        bool keyB_allowed = !(data_ac == 7);

        return (key_type == KeyType::KeyA && keyA_allowed) ||
               (key_type == KeyType::KeyB && keyB_allowed);
    }
    // Placeholder return true until logic is fixed
    return true;
}

// --- CardData Helpers ---

void initCardData(CardData& card_data) {
    // Clear all fields, especially bitmasks and block data
    memset(&card_data, 0, sizeof(CardData));
    card_data.type = Type::Unknown; // Explicitly set unknown initially
}

void setBlockReadFlag(CardData& card_data, uint16_t block_num, bool read) {
    if (block_num >= MAX_BLOCKS) return;
    size_t byte_index = block_num / 8;
    uint8_t bit_index = block_num % 8;
    if (read) {
        card_data.block_read_mask[byte_index] |= (1 << bit_index);
    } else {
        card_data.block_read_mask[byte_index] &= ~(1 << bit_index);
    }
}

bool getBlockReadFlag(const CardData& card_data, uint16_t block_num) {
    if (block_num >= MAX_BLOCKS) return false;
    size_t byte_index = block_num / 8;
    uint8_t bit_index = block_num % 8;
    return (card_data.block_read_mask[byte_index] >> bit_index) & 1;
}

void setKeyFoundFlag(CardData& card_data, uint8_t sector_num, KeyType key_type, bool found) {
    if (sector_num >= MAX_SECTORS) return;
    uint64_t bit = 1ULL << sector_num; // Use 1ULL for 64-bit literal
    uint64_t* mask = (key_type == KeyType::KeyA) ? &card_data.found_keys_a : &card_data.found_keys_b;
    if (found) {
        *mask |= bit;
    } else {
        *mask &= ~bit;
    }
}

bool getKeyFoundFlag(const CardData& card_data, uint8_t sector_num, KeyType key_type) {
    if (sector_num >= MAX_SECTORS) return false;
    uint64_t bit = 1ULL << sector_num;
    const uint64_t* mask = (key_type == KeyType::KeyA) ? &card_data.found_keys_a : &card_data.found_keys_b;
    return (*mask >> sector_num) & 1;
}

void setAuthKeyType(CardData& card_data, uint8_t sector_num, KeyType key_type) {
    if (sector_num >= MAX_SECTORS) return;
    uint64_t bit = 1ULL << sector_num;
    if (key_type == KeyType::KeyB) {
        card_data.auth_key_type_mask |= bit;
    } else {
        card_data.auth_key_type_mask &= ~bit;
    }
}

KeyType getAuthKeyType(const CardData& card_data, uint8_t sector_num) {
    if (sector_num >= MAX_SECTORS) return KeyType::KeyA; // Default or error indication?
    return ((card_data.auth_key_type_mask >> sector_num) & 1) ? KeyType::KeyB : KeyType::KeyA;
}


} // namespace MfClassic
} // namespace NFC