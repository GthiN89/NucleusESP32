// nfc.cpp

#include "nfc.h"
#include "globals.h"
#include <SPI.h>
#include <Adafruit_PN532.h>
#include "protocols/nfc_card_defs.h" // Use the new combined header
#include "protocols/mifare_dictionaries.h" // Include the split PROGMEM dictionaries

// Ensure implementations for mf_classic_defs.h functions are linked (e.g., mf_classic_helpers.cpp)
// OR include the .cpp file here if you prefer (simpler for Arduino IDE, but less standard)
// #include "protocols/mf_classic_helpers.cpp"

namespace NFC {

// Use using declarations
using MfClassic::CardData;
using MfClassic::Type;
using MfClassic::KeyType;
using MfClassic::Action;
using MfClassic::Key;
using MfClassic::Block;
using MfClassic::AccessBits;
using MfClassic::SectorTrailer;
using ::DictionaryPart; // Assuming DictionaryPart is defined globally or adjust namespace

// --- Define global variables ---
Adafruit_PN532 nfc(PN532_SS, &SPI);
NfcCard currentNfcCard; // Use the generic card structure (holds state between calls)

// --- Forward declarations for static helpers within this file ---
static void printBlockOrTrailer(const MfClassic::CardData& cardData, uint16_t block_num, const Adafruit_PN532& nfc_reader, bool force_print = false);
static void printCardData(const NfcCard& cardData, const Adafruit_PN532& nfc_reader);
// Forward declare or include implementation for MfClassic helper functions if not linked
namespace MfClassic {
    // Declare functions needed if implementations are in a separate .cpp file
    extern uint8_t getTotalSectors(Type type);
    extern uint16_t getTotalBlocks(Type type);
    extern const char* getTypeName(Type type);
    extern uint8_t getSectorForBlock(uint16_t block_num);
    extern uint16_t getFirstBlockOfSector(uint8_t sector_num);
    extern uint16_t getSectorTrailerBlockNum(uint8_t sector_num);
    extern uint8_t getBlocksInSector(uint8_t sector_num);
    extern bool isSectorTrailer(uint16_t block_num);
    extern bool checkReadAccess(const AccessBits& ac_bits, uint16_t block_num, KeyType key_type);
    extern void initCardData(CardData& card_data);
    extern void setBlockReadFlag(CardData& card_data, uint16_t block_num, bool read);
    extern bool getBlockReadFlag(const CardData& card_data, uint16_t block_num);
    extern void setKeyFoundFlag(CardData& card_data, uint8_t sector_num, KeyType key_type, bool found);
    extern bool getKeyFoundFlag(const CardData& card_data, uint8_t sector_num, KeyType key_type);
    extern void setAuthKeyType(CardData& card_data, uint8_t sector_num, KeyType key_type);
    extern KeyType getAuthKeyType(const CardData& card_data, uint8_t sector_num);
}
// Add stubs or forward declarations for other card type helpers if needed
namespace MfUltralight { extern const char* getTypeName(Type type); }
namespace MfDESFire { extern const char* getTypeName(Type type); }
namespace MfPlus { extern const char* getTypeName(Type type); }


// --- NFC_CLASS Method Implementations ---

NFC_CLASS::NFC_CLASS() : _initialized(false) {}
NFC_CLASS::~NFC_CLASS() { deinit(); }
bool NFC_CLASS::isInitialized() const { return _initialized; }

bool NFC_CLASS::init() {
    if (_initialized) { Serial.println("[NFC] Already initialized."); return true; }
    Serial.println("[NFC] Initializing...");
    #ifdef CC1101_CS
    pinMode(CC1101_CS, OUTPUT); digitalWrite(CC1101_CS, HIGH);
    #endif
    #ifdef SD_CS
    pinMode(SD_CS, OUTPUT); digitalWrite(SD_CS, HIGH);
    #endif
    pinMode(PN532_SS, OUTPUT); digitalWrite(PN532_SS, HIGH); delay(10);
    SPI.begin(CYD_SCLK, CYD_MISO, CYD_MOSI, -1); delay(10);
    Serial.println("[NFC] SPI Initialized. Looking for PN532...");
    nfc.begin();
    uint32_t versiondata = nfc.getFirmwareVersion();
    if (!versiondata) {
        Serial.println("[NFC ERROR] Didn't find PN53x board!");
        digitalWrite(PN532_SS, HIGH); _initialized = false; return false;
    }
    Serial.print("[NFC] Found chip PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
    Serial.print("[NFC] Firmware ver. "); Serial.print((versiondata >> 16) & 0xFF, DEC);
    Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);
    uint8_t retries = 0x05;
    Serial.printf("[NFC] Setting passive activation retries to: 0x%02X\n", retries);
    if (!nfc.setPassiveActivationRetries(retries)) { Serial.println("[NFC WARNING] Failed to set retry count."); }
    Serial.println("[NFC] Configuring SAM...");
    if (!nfc.SAMConfig()) {
        Serial.println("[NFC ERROR] Failed to configure SAM!");
        digitalWrite(PN532_SS, HIGH); _initialized = false; return false;
    }
    Serial.println("[NFC] Initialization successful. Ready for cards.");
    _initialized = true;
    delay(50);
    return true;
}

void NFC_CLASS::deinit() {
    if (_initialized) {
        digitalWrite(PN532_SS, HIGH);
        _initialized = false;
        Serial.println("[NFC] Deinitialized");
    }
}


// --- Dump using ONLY default keys ---
bool NFC_CLASS::mifareDumpDefaultKeys() {
    if (!_initialized) {
        Serial.println("[NFC] Not initialized. Cannot dump.");
        return false;
    }

    currentNfcCard.reset(); // Reset data for new card read
    Serial.println("[NFC] Place Mifare Classic card on reader (Default Keys Check)...");
    delay(10);

    // --- Card Detection ---
    Serial.println("[NFC] Starting detection...");
    if (!nfc.startPassiveTargetIDDetection(PN532_MIFARE_ISO14443A)) { Serial.println("[NFC ERROR] Failed start passive detection."); nfc.inRelease(); return false; }
    Serial.println("[NFC] Waiting for card presence...");
    uint16_t timeout = 10000; uint16_t timer = 0; bool card_responded = false; // Shorter timeout maybe?
    while (timer < timeout) { if (nfc.isready()) { card_responded = true; break; } delay(50); timer += 50; yield(); }
    if (!card_responded) { Serial.println("[NFC ERROR] Timeout waiting for card."); nfc.inRelease(); return false; }
    if (!nfc.readDetectedPassiveTargetID()) { Serial.println("[NFC ERROR] Failed to read card data."); nfc.inRelease(); return false; }
    delay(50);

    // --- Copy Basic ISO14443-3A data ---
    currentNfcCard.uid_len = nfc.targetUid.size; memcpy(currentNfcCard.uid, nfc.targetUid.uidByte, currentNfcCard.uid_len);
    memcpy(currentNfcCard.atqa, nfc.targetUid.atqaByte, 2); currentNfcCard.sak = nfc.targetUid.sak;

    // --- Identify Card Type ---
    currentNfcCard.general_type = CardType::Unknown;
    if (currentNfcCard.uid_len == 4) { if (currentNfcCard.sak == 0x09) { currentNfcCard.general_type = CardType::MifareClassic; currentNfcCard.mf_classic_data.type = Type::Mini; } else if (currentNfcCard.sak == 0x08) { currentNfcCard.general_type = CardType::MifareClassic; currentNfcCard.mf_classic_data.type = Type::Classic1k; } else if (currentNfcCard.sak == 0x18) { currentNfcCard.general_type = CardType::MifareClassic; currentNfcCard.mf_classic_data.type = Type::Classic4k; } else if (currentNfcCard.sak == 0x00 && currentNfcCard.atqa[0] == 0x44 && currentNfcCard.atqa[1] == 0x00) { currentNfcCard.general_type = CardType::MifareUltralight; currentNfcCard.mf_ul_data.type = MfUltralight::Type::Unknown; } else if (currentNfcCard.sak & 0x10) { currentNfcCard.general_type = CardType::MifarePlus; currentNfcCard.mf_plus_data.type = MfPlus::Type::Unknown; } }
    if ((currentNfcCard.sak & 0x20) || (currentNfcCard.sak & 0x40)) { if(currentNfcCard.general_type == CardType::Unknown) currentNfcCard.general_type = CardType::ISO14443_4A; currentNfcCard.mf_desfire_data.type = MfDESFire::Type::Unknown; currentNfcCard.mf_plus_data.sl = MfPlus::SecurityLevel::Unknown; }

    // --- Print Initial Info ---
    Serial.print("[NFC] Found Card: "); nfc.PrintHex(currentNfcCard.uid, currentNfcCard.uid_len);
    Serial.print("[NFC] Type: ");
    switch(currentNfcCard.general_type) {
        case CardType::MifareClassic: Serial.println(MfClassic::getTypeName(currentNfcCard.mf_classic_data.type)); break;
        case CardType::MifareUltralight: Serial.println(MfUltralight::getTypeName(currentNfcCard.mf_ul_data.type)); break; // Assuming helper exists
        case CardType::MifareDESFire: Serial.println(MfDESFire::getTypeName(currentNfcCard.mf_desfire_data.type)); break; // Assuming helper exists
        case CardType::MifarePlus: Serial.println(MfPlus::getTypeName(currentNfcCard.mf_plus_data.type)); break;          // Assuming helper exists
        case CardType::ISO14443_4A: Serial.println("Generic ISO14443-4A"); break;
        default: Serial.println("Unknown"); break;
    }
    Serial.print("[NFC] ATQA: "); nfc.PrintHex(currentNfcCard.atqa, 2); Serial.printf(" SAK: 0x%02X\n", currentNfcCard.sak);

    // --- Only proceed if Mifare Classic ---
    if (currentNfcCard.general_type != CardType::MifareClassic) {
        Serial.println("[NFC] Not Mifare Classic. Skipping default key dump.");
        nfc.inRelease();
        return true; // Still return true as a card was detected
    }

    Serial.println("[NFC] Attempting dump with default keys ONLY...");
    MfClassic::CardData& classicData = currentNfcCard.mf_classic_data; // Use reference

    uint8_t keyA_default[6] = { 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5 };
    uint8_t keyB_default[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    uint8_t temp_block_buffer[16];

    uint8_t total_sectors = MfClassic::getTotalSectors(classicData.type);

    for (uint8_t s = 0; s < total_sectors; ++s) {
        Serial.printf("\n[NFC] ---- Sector %d ----\n", s);
        bool sector_authenticated = false;
        KeyType key_used = KeyType::KeyA;
        const Key* key_ptr_used = nullptr;
        uint16_t trailer_block_num = MfClassic::getSectorTrailerBlockNum(s);

        // --- Authentication with Defaults Only ---
        Serial.printf("[NFC] Auth S%d with default Key A... ", s);
        if (nfc.mifareclassic_AuthenticateBlock(currentNfcCard.uid, currentNfcCard.uid_len, trailer_block_num, 0, keyA_default)) {
             Serial.println("Success!"); sector_authenticated = true; key_used = KeyType::KeyA; key_ptr_used = reinterpret_cast<const Key*>(keyA_default);
             MfClassic::setKeyFoundFlag(classicData, s, KeyType::KeyA, true); MfClassic::setAuthKeyType(classicData, s, KeyType::KeyA); memcpy(classicData.blocks[trailer_block_num].data, key_ptr_used->data, MfClassic::KEY_SIZE);
        } else {
            Serial.println("FAILED!"); MfClassic::setKeyFoundFlag(classicData, s, KeyType::KeyA, false); delay(20); yield();
            Serial.printf("[NFC] Auth S%d with default Key B... ", s);
            if (nfc.mifareclassic_AuthenticateBlock(currentNfcCard.uid, currentNfcCard.uid_len, trailer_block_num, 1, keyB_default)) {
                Serial.println("Success!"); sector_authenticated = true; key_used = KeyType::KeyB; key_ptr_used = reinterpret_cast<const Key*>(keyB_default);
                MfClassic::setKeyFoundFlag(classicData, s, KeyType::KeyB, true); MfClassic::setAuthKeyType(classicData, s, KeyType::KeyB); memcpy(&classicData.blocks[trailer_block_num].data[10], key_ptr_used->data, MfClassic::KEY_SIZE);
            } else {
                Serial.println("FAILED!"); MfClassic::setKeyFoundFlag(classicData, s, KeyType::KeyB, false); delay(20); yield();
            }
        }

        // --- Read Blocks if Authenticated ---
        if (sector_authenticated) {
            if (key_ptr_used == nullptr) continue;
            delay(40); yield();
            uint16_t first_block = MfClassic::getFirstBlockOfSector(s);
            uint8_t blocks_in_sector = MfClassic::getBlocksInSector(s);
            for (uint8_t b_offset = 0; b_offset < blocks_in_sector; ++b_offset) {
                uint16_t block_num = first_block + b_offset;
                uint8_t success_read = nfc.mifareclassic_ReadDataBlock(block_num, classicData.blocks[block_num].data);
                delay(25); yield();
                MfClassic::setBlockReadFlag(classicData, block_num, success_read);
                if (success_read) {
                     if (MfClassic::isSectorTrailer(block_num)) { if (key_used == KeyType::KeyA) { memcpy(classicData.blocks[trailer_block_num].data, key_ptr_used->data, MfClassic::KEY_SIZE); if (!MfClassic::getKeyFoundFlag(classicData, s, KeyType::KeyB)) MfClassic::setKeyFoundFlag(classicData, s, KeyType::KeyB, true); } else { memcpy(&classicData.blocks[trailer_block_num].data[10], key_ptr_used->data, MfClassic::KEY_SIZE); if (!MfClassic::getKeyFoundFlag(classicData, s, KeyType::KeyA)) MfClassic::setKeyFoundFlag(classicData, s, KeyType::KeyA, true); } }
                     printBlockOrTrailer(classicData, block_num, nfc, true);
                } else {
                    memset(classicData.blocks[block_num].data, 0, MfClassic::BLOCK_SIZE);
                    if (MfClassic::isSectorTrailer(block_num)) { if (!MfClassic::getKeyFoundFlag(classicData, s, KeyType::KeyA)) MfClassic::setKeyFoundFlag(classicData, s, KeyType::KeyA, false); if (!MfClassic::getKeyFoundFlag(classicData, s, KeyType::KeyB)) MfClassic::setKeyFoundFlag(classicData, s, KeyType::KeyB, false); }
                    printBlockOrTrailer(classicData, block_num, nfc, true);
                }
            }
        } else { // Not authenticated with defaults
             Serial.println("[NFC] Default keys failed for sector.");
             memset(classicData.blocks[trailer_block_num].data, 0, MfClassic::BLOCK_SIZE);
             MfClassic::setBlockReadFlag(classicData, trailer_block_num, false);
        }
        delay(50); yield(); // Delay between sectors
    } // End sector loop

    Serial.println("\n[NFC] Default key dump phase finished.");
    nfc.inRelease(); delay(50);
    // No final print here, rely on the immediate prints or explicit call to printStoredCardData()
    return true; // Indicate card was detected
}

// --- Dictionary attack on sectors missed by default keys ---
bool NFC_CLASS::mifareDumpDictionaryAttack() {

  uint8_t keyA_default[6] = { 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5 };
  uint8_t keyB_default[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    if (!_initialized) {
        Serial.println("[NFC] Not initialized. Cannot run dictionary attack.");
        return false;
    }
    if (currentNfcCard.general_type != CardType::MifareClassic || currentNfcCard.mf_classic_data.type == MfClassic::Type::Unknown || currentNfcCard.uid_len == 0) {
        Serial.println("[NFC ERROR] No valid Mifare Classic data available. Run default key dump first.");
        return false;
    }

    Serial.println("\n[NFC] Starting dictionary attack on failed sectors...");
    MfClassic::CardData& classicData = currentNfcCard.mf_classic_data; // Use reference

    uint8_t current_key_buffer[6];
    uint8_t last_known_keyA[6] = { 0 }; bool known_keyA_found = false;
    uint8_t last_known_keyB[6] = { 0 }; bool known_keyB_found = false;
    uint8_t temp_block_buffer[16];

    // Re-populate last known keys based on current stored data
    for(uint8_t s_idx = 0; s_idx < MfClassic::getTotalSectors(classicData.type); ++s_idx) {
        if(MfClassic::getKeyFoundFlag(classicData, s_idx, KeyType::KeyA)) {
            memcpy(last_known_keyA, classicData.blocks[MfClassic::getSectorTrailerBlockNum(s_idx)].data, MfClassic::KEY_SIZE);
            known_keyA_found = true;
        }
         if(MfClassic::getKeyFoundFlag(classicData, s_idx, KeyType::KeyB)) {
            memcpy(last_known_keyB, &classicData.blocks[MfClassic::getSectorTrailerBlockNum(s_idx)].data[10], MfClassic::KEY_SIZE);
            known_keyB_found = true;
        }
    }

    uint8_t total_sectors = MfClassic::getTotalSectors(classicData.type);
    bool attack_performed_on_any_sector = false;

    for (uint8_t s = 0; s < total_sectors; ++s) {
        // --- Skip if sector already known ---
        if (MfClassic::getKeyFoundFlag(classicData, s, KeyType::KeyA) || MfClassic::getKeyFoundFlag(classicData, s, KeyType::KeyB)) {
            continue;
        }

        attack_performed_on_any_sector = true;
        Serial.printf("\n[NFC] ---- Attacking Sector %d ----\n", s);
        bool sector_authenticated = false;
        KeyType key_used = KeyType::KeyA;
        const Key* key_ptr_used = nullptr;
        uint16_t trailer_block_num = MfClassic::getSectorTrailerBlockNum(s);

        // --- Dictionary Attack Logic ---
        Serial.printf("[NFC] Starting dictionary attack for Sector %d...\n", s);
        uint32_t total_keys_tried = 0;
        for (size_t dict_part_idx = 0; dict_part_idx < mifare_dictionaries_count; ++dict_part_idx) {
            ::DictionaryPart current_dict_part_info;
            memcpy_P(&current_dict_part_info, &mifare_dictionaries[dict_part_idx], sizeof(::DictionaryPart));
            for (size_t key_idx = 0; key_idx < current_dict_part_info.count; ++key_idx) {
                memcpy_P(current_key_buffer, &current_dict_part_info.keys[key_idx], MfClassic::KEY_SIZE);
                total_keys_tried++;
                // Skip defaults and known keys
                if (memcmp(current_key_buffer, keyA_default, 6) == 0 || memcmp(current_key_buffer, keyB_default, 6) == 0) continue;
                if (known_keyA_found && memcmp(current_key_buffer, last_known_keyA, 6) == 0) continue;
                if (known_keyB_found && memcmp(current_key_buffer, last_known_keyB, 6) == 0) continue;

                if (total_keys_tried % 100 == 1 || key_idx == current_dict_part_info.count -1) { Serial.printf("[NFC] Trying key %u : ", total_keys_tried); nfc.PrintHex(current_key_buffer, 6); }

                if (nfc.mifareclassic_AuthenticateBlock(currentNfcCard.uid, currentNfcCard.uid_len, trailer_block_num, 0, current_key_buffer)) {
                    Serial.println(" Key A FOUND!"); sector_authenticated = true; key_used = KeyType::KeyA; key_ptr_used = reinterpret_cast<const Key*>(current_key_buffer);
                    MfClassic::setKeyFoundFlag(classicData, s, KeyType::KeyA, true); MfClassic::setAuthKeyType(classicData, s, KeyType::KeyA); memcpy(classicData.blocks[trailer_block_num].data, current_key_buffer, MfClassic::KEY_SIZE); memcpy(last_known_keyA, current_key_buffer, MfClassic::KEY_SIZE); known_keyA_found = true;
                    goto auth_done_dict_split_2; // Use unique label name
                }
                 delay(1); yield();

                if (nfc.mifareclassic_AuthenticateBlock(currentNfcCard.uid, currentNfcCard.uid_len, trailer_block_num, 1, current_key_buffer)) {
                   Serial.println(" Key B FOUND!"); sector_authenticated = true; key_used = KeyType::KeyB; key_ptr_used = reinterpret_cast<const Key*>(current_key_buffer);
                   MfClassic::setKeyFoundFlag(classicData, s, KeyType::KeyB, true); MfClassic::setAuthKeyType(classicData, s, KeyType::KeyB); memcpy(&classicData.blocks[trailer_block_num].data[10], current_key_buffer, MfClassic::KEY_SIZE); memcpy(last_known_keyB, current_key_buffer, MfClassic::KEY_SIZE); known_keyB_found = true;
                   goto auth_done_dict_split_2; // Use unique label name
                }

                if (total_keys_tried % 100 == 1 && !sector_authenticated) Serial.print(" Nope."); else if (key_idx == current_dict_part_info.count -1 && dict_part_idx == mifare_dictionaries_count -1 && !sector_authenticated) Serial.println(" Nope.");
                yield();
            } // End inner key loop
        } // End outer dictionary part loop
      auth_done_dict_split_2:; // Unique label
        if (!sector_authenticated) { Serial.printf("\n[NFC] Dictionary attack failed for sector %d after trying %u keys.\n", s, total_keys_tried); }
        else { if(key_ptr_used == nullptr || (memcmp(key_ptr_used->data, keyA_default, 6)==0 && key_used != KeyType::KeyA) || (memcmp(key_ptr_used->data, keyB_default, 6)==0 && key_used != KeyType::KeyB) ) { key_ptr_used = reinterpret_cast<const Key*>(current_key_buffer); } }


        // --- Read Blocks if Authenticated by Dictionary ---
        if (sector_authenticated) {
             if (key_ptr_used == nullptr) continue;
             delay(40); yield();
             uint16_t first_block = MfClassic::getFirstBlockOfSector(s);
             uint8_t blocks_in_sector = MfClassic::getBlocksInSector(s);
             for (uint8_t b_offset = 0; b_offset < blocks_in_sector; ++b_offset) {
                 uint16_t block_num = first_block + b_offset;
                 uint8_t success_read = nfc.mifareclassic_ReadDataBlock(block_num, classicData.blocks[block_num].data);
                 delay(25); yield();
                 MfClassic::setBlockReadFlag(classicData, block_num, success_read);
                 if (success_read) {
                     if (MfClassic::isSectorTrailer(block_num)) { /* Preserve known key */ if (key_used == KeyType::KeyA) { memcpy(classicData.blocks[trailer_block_num].data, key_ptr_used->data, MfClassic::KEY_SIZE); if (!MfClassic::getKeyFoundFlag(classicData, s, KeyType::KeyB)) MfClassic::setKeyFoundFlag(classicData, s, KeyType::KeyB, true); } else { memcpy(&classicData.blocks[trailer_block_num].data[10], key_ptr_used->data, MfClassic::KEY_SIZE); if (!MfClassic::getKeyFoundFlag(classicData, s, KeyType::KeyA)) MfClassic::setKeyFoundFlag(classicData, s, KeyType::KeyA, true); } }
                     printBlockOrTrailer(classicData, block_num, nfc, true);
                 } else {
                     memset(classicData.blocks[block_num].data, 0, MfClassic::BLOCK_SIZE);
                     if (MfClassic::isSectorTrailer(block_num)) { if (!MfClassic::getKeyFoundFlag(classicData, s, KeyType::KeyA)) MfClassic::setKeyFoundFlag(classicData, s, KeyType::KeyA, false); if (!MfClassic::getKeyFoundFlag(classicData, s, KeyType::KeyB)) MfClassic::setKeyFoundFlag(classicData, s, KeyType::KeyB, false); }
                     printBlockOrTrailer(classicData, block_num, nfc, true);
                 }
             }
        } else { // Sector still not authenticated after dictionary
             memset(classicData.blocks[trailer_block_num].data, 0, MfClassic::BLOCK_SIZE);
             MfClassic::setBlockReadFlag(classicData, trailer_block_num, false);
        }
        delay(50); yield();
    } // End sector loop

    // --- Cleanup ---
    Serial.println("\n[NFC] Dictionary attack finished.");
    nfc.inRelease(); delay(50);
    if (!attack_performed_on_any_sector) {
        Serial.println("[NFC] No sectors required dictionary attack.");
    }
    // Final print is handled by printStoredCardData() called separately
    return true; // Indicate attack process completed
}

// Add the new print function
void NFC_CLASS::printStoredCardData() {
     if (currentNfcCard.general_type == CardType::Unknown || currentNfcCard.uid_len == 0) {
        Serial.println("[NFC] No valid card data stored to print.");
        return;
    }
    Serial.println("[NFC] Printing Stored Card Data:");
    printCardData(currentNfcCard, nfc); // Call the static helper
}


// --- Definition for static print helpers ---
/*static*/ void printBlockOrTrailer(const MfClassic::CardData& cardData, uint16_t block_num, const Adafruit_PN532& nfc_reader, bool force_print /*= false*/) {
    uint8_t sector_num = MfClassic::getSectorForBlock(block_num);
    bool block_is_read = MfClassic::getBlockReadFlag(cardData, block_num);

    Serial.printf("  Block %2d: ", block_num);

    if (block_is_read) {
        if (MfClassic::isSectorTrailer(block_num)) {
             const SectorTrailer* trailer = reinterpret_cast<const SectorTrailer*>(&cardData.blocks[block_num]);
            bool key_a_found = MfClassic::getKeyFoundFlag(cardData, sector_num, KeyType::KeyA);
            bool key_b_found = MfClassic::getKeyFoundFlag(cardData, sector_num, KeyType::KeyB);

            if (key_a_found) nfc_reader.PrintHex(trailer->key_a.data, MfClassic::KEY_SIZE); else Serial.print("?? ?? ?? ?? ?? ?? ");
            nfc_reader.PrintHex(trailer->access_bits.data, MfClassic::ACCESS_BYTES_SIZE); Serial.print(" ");
            if (key_b_found) nfc_reader.PrintHex(trailer->key_b.data, MfClassic::KEY_SIZE); else Serial.print("?? ?? ?? ?? ?? ?? ");
            Serial.print(" (Trailer)");

        } else { // Data block
            nfc_reader.PrintHex(cardData.blocks[block_num].data, MfClassic::BLOCK_SIZE);
        }
    } else { // Block not read
         if (force_print) { // Immediate print after failure
            Serial.print("!! READ FAILED !!");
            bool key_a_known = MfClassic::getKeyFoundFlag(cardData, sector_num, KeyType::KeyA);
            bool key_b_known = MfClassic::getKeyFoundFlag(cardData, sector_num, KeyType::KeyB);
            uint16_t trailer_block_num = MfClassic::getSectorTrailerBlockNum(sector_num);
            bool trailer_read = MfClassic::getBlockReadFlag(cardData, trailer_block_num);

            if (key_a_known || key_b_known) {
                KeyType auth_key = MfClassic::getAuthKeyType(cardData, sector_num);
                 if (trailer_read) {
                     const SectorTrailer* trailer = reinterpret_cast<const SectorTrailer*>(&cardData.blocks[trailer_block_num]);
                     // *** NOTE: checkReadAccess logic needs to be correct ***
                     if (!MfClassic::checkReadAccess(trailer->access_bits, block_num, auth_key)) {
                         Serial.print(" (Access Denied by ACs)");
                     } else {
                         Serial.print(" (Comm Error?)");
                     }
                 } else {
                     Serial.print(" (ACs Unknown)");
                 }
            } else {
                 Serial.print(" (No Key Known)");
            }
         } else { // Final summary print
            Serial.print("?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ??");
            Serial.print(" (Read Failed - No Key Known)"); // Assume no key if not read by end
         }
    }
    Serial.println();
}

/*static*/ void printCardData(const NfcCard& cardData, const Adafruit_PN532& nfc_reader) {
    Serial.println("--- Card Data ---");
    Serial.printf("Card Type: %s\n", MfClassic::getTypeName(cardData.mf_classic_data.type)); // Assuming helper exists for Classic
    Serial.print("UID: "); nfc_reader.PrintHex(cardData.uid, cardData.uid_len);
    Serial.print("ATQA: "); nfc_reader.PrintHex(cardData.atqa, 2);
    Serial.printf("SAK: 0x%02X\n", cardData.sak);
    Serial.println("-----------------");

    if (cardData.general_type != CardType::MifareClassic || cardData.mf_classic_data.type == MfClassic::Type::Unknown) {
        Serial.println("Cannot print Mifare Classic block data.");
        return;
    }

    const MfClassic::CardData& classicData = cardData.mf_classic_data; // Use reference
    uint8_t total_sectors = MfClassic::getTotalSectors(classicData.type);
    for (uint8_t s = 0; s < total_sectors; ++s) {
        Serial.printf("\nSector %d:\n", s);
        uint16_t first_block = MfClassic::getFirstBlockOfSector(s);
        uint8_t blocks_in_sector = MfClassic::getBlocksInSector(s);
        for (uint8_t b_offset = 0; b_offset < blocks_in_sector; ++b_offset) {
            uint16_t block_num = first_block + b_offset;
            printBlockOrTrailer(classicData, block_num, nfc_reader, false); // Use false for final summary
        }
    }
    Serial.println("\n--- End Classic Data ---");
}

// --- Implementations for other GetTypeName helpers (stubs for now) ---
namespace MfUltralight { const char* getTypeName(Type type) { switch(type){ default: return "Ultralight/NTAG"; } } }
namespace MfDESFire { const char* getTypeName(Type type) { switch(type){ default: return "DESFire"; } } }
namespace MfPlus { const char* getTypeName(Type type) { switch(type){ default: return "Plus"; } } }


} // namespace NFC