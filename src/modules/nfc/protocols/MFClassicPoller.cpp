#include "MFClassicPoller.hpp"
#include "globals.h" // For Serial output
#include <algorithm> // For std::copy if needed (not used in simplified detectType)

// --- Constants specific to the poller ---
namespace {
    constexpr uint32_t kDetectionTimeoutMs = 300; // Shorter timeout for polling detection
    // Universal Key B (often used as default or transport key)
    uint8_t kKeyUniversal[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
}

// --- MfClassicPoller Method Implementations ---

MfClassicPoller::MfClassicPoller(Adafruit_PN532* reader)
    : nfc(reader),
      cardType(MfClassicTypeNum),
      uidLength(0) {
    // Ensure UID buffer is zeroed initially (though array initialization does this)
    memset(uid, 0, sizeof(uid));
}

MfClassicType MfClassicPoller::detectType() {
    if (!nfc) {
        Serial.println("[Poller ERROR] NFC reader instance is null!");
        return MfClassicTypeNum;
    }

    // Reset internal state before detection
    uidLength = 0;
    cardType = MfClassicTypeNum;
    memset(uid, 0, sizeof(uid));

    // Try to read a card ID with the specified timeout
    // The library function handles CS pin and timing.
    uint8_t success = nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, kDetectionTimeoutMs);

    if (success) {
        // Basic check: Mifare Classic usually have 4-byte or 7-byte UIDs
        if (uidLength == 4) {
             Serial.println("[Poller] Detected ISO14443A card with 4-byte UID.");
             // Simplified: Assume 1k for 4-byte UIDs, as distinguishing requires more complex checks.
            cardType = MfClassicType1k;
             return cardType;
        } else if (uidLength == 7) {
             Serial.println("[Poller] Detected ISO14443A card with 7-byte UID.");
             // Could be Mifare Plus, DESFire EV1/2/3 in compatibility mode, etc.
             // Treat as non-Classic for this poller's purpose.
             cardType = MfClassicTypeNum;
             return cardType;
        } else {
            Serial.printf("[Poller] Detected ISO14443A card with unexpected UID length: %d\n", uidLength);
            cardType = MfClassicTypeNum;
            return cardType;
        }
    } else {
        // No card found within the timeout, this is normal during polling
        // Serial.println("[Poller] No card detected."); // Can be verbose, uncomment if needed
        return MfClassicTypeNum;
    }
}

bool MfClassicPoller::authenticateBlock(uint8_t blockNum, uint8_t* key, MfClassicKeyType keyType) {
    if (!nfc) return false;
    if (uidLength == 0) {
        Serial.println("[Poller AUTH] Error: No card detected/selected (UID length is 0).");
        return false; // Cannot authenticate without a selected card UID
    }
    if (!key) {
         Serial.println("[Poller AUTH] Error: Key pointer is null.");
         return false;
    }

    // Use the library function to authenticate
    // Library should handle CS pin and card state.
    return nfc->mifareclassic_AuthenticateBlock(
        this->uid,        // Use the UID stored in the class
        this->uidLength,  // Use the UID length stored in the class
        blockNum,
        static_cast<uint8_t>(keyType), // Cast enum to uint8_t (0 for KeyA, 1 for KeyB)
        key             // Pass the pointer to the 6-byte key
    );
}

bool MfClassicPoller::readBlock(uint8_t blockNum, MfClassicBlock* blockData) {
    if (!nfc || !blockData) return false;
    if (uidLength == 0) {
         // Can't read if no card selected
         return false;
    }
    // Use library function, assumes prior authentication of the sector
    return nfc->mifareclassic_ReadDataBlock(blockNum, blockData->data);
}

bool MfClassicPoller::writeBlock(uint8_t blockNum, MfClassicBlock* blockData) {
     if (!nfc || !blockData) return false;
     if (uidLength == 0) {
         // Can't write if no card selected
         return false;
    }
    // Use library function, assumes prior authentication of the sector
    // !! Be very careful when using write !!
    return nfc->mifareclassic_WriteDataBlock(blockNum, blockData->data);
}


void MfClassicPoller::dumpToSerial() {
    if (!nfc) return;
    if (uidLength == 0) {
        Serial.println("[Poller DUMP] No card detected/selected to dump.");
        return;
    }

    Serial.println("[Poller DUMP] ----------------------------------------");
    Serial.print("  Card Type: ");
    switch(cardType) {
        case MfClassicType1k: Serial.println("Mifare Classic 1k (Assumed)"); break;
        // Add cases if you implement better detection later
        // case MfClassicType4k: Serial.println("Mifare Classic 4k"); break;
        // case MfClassicTypeMini: Serial.println("Mifare Classic Mini"); break;
        default: Serial.println("Unknown/Non-Classic"); break;
    }

    Serial.print("  UID Length: "); Serial.print(uidLength, DEC); Serial.println(" bytes");
    Serial.print("  UID Value: "); nfc->PrintHex(uid, uidLength); Serial.println();

    // Only attempt block dump if it's assumed to be Mifare Classic (based on UID length 4)
    if (uidLength != 4 || cardType != MfClassicType1k) {
         Serial.println("[Poller DUMP] Not a recognized Mifare Classic 1k card. Cannot dump blocks.");
         Serial.println("-----------------------------------------------");
        return;
    }

    Serial.println("[Poller DUMP] Attempting dump of blocks 0-63 using universal Key B...");
    uint16_t blocksToDump = 64; // Hardcoded for 1k card dump
    uint8_t data[16];
    bool authenticated = false; // Authentication status per sector

    for (uint8_t currentBlock = 0; currentBlock < blocksToDump; currentBlock++) {
        bool isFirstBlock = nfc->mifareclassic_IsFirstBlock(currentBlock);

        if (isFirstBlock) {
            authenticated = false; // Reset authentication for the new sector
             Serial.printf("\n---- Sector %d ----\n", currentBlock / 4);
        }

        if (!authenticated) {
            // Try authenticating using the poller's authenticateBlock method
             Serial.printf("Authenticating Block %d (Key B)... ", currentBlock);
            authenticated = this->authenticateBlock(currentBlock, kKeyUniversal, MfClassicKeyB);

            if (!authenticated) {
                 Serial.println("FAILED!");
                // Skip to the end of this sector
                currentBlock = ((currentBlock / 4) * 4) + 3;
                continue;
            } else {
                 Serial.println("Success!");
            }
        }

        // If authenticated for this sector, read the block
        if (authenticated) {
            MfClassicBlock blockBuffer;
             Serial.printf("Reading Block %d ", currentBlock);
             if(currentBlock < 10) Serial.print(" ");

            if (this->readBlock(currentBlock, &blockBuffer)) {
                 Serial.print(": ");
                 nfc->PrintHexChar(blockBuffer.data, 16); // Use library helper to print
            } else {
                 Serial.println(" Read FAILED!");
                 // Authentication might be lost, or access denied
                 // Optionally force re-authentication for the next block attempt
                 // authenticated = false;
            }
        }
    }
    Serial.println("\n[Poller DUMP] ----------------------------------------");
}