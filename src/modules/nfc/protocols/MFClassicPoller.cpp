#include "MFClassicPoller.hpp"
#include "globals.h"
#include <algorithm> // for std::copy

namespace {
    constexpr uint16_t kDetectionTimeoutMs = 500;
    uint8_t kKeyUniversal[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Removed const
}


MfClassicPoller::MfClassicPoller(Adafruit_PN532* reader) 
    : nfc(reader), 
      cardType(MfClassicType1k),
      uid{0},
      uidLength(0) {
}

bool MfClassicPoller::begin() {
    // Your original begin() implementation here
    return true;
}

MfClassicType MfClassicPoller::detectType() {
    const auto startTime = millis();
    uint8_t success = 0;
    uint8_t tempUid[7] = {0};
    uint8_t tempUidLength = 0;

    while ((millis() - startTime) < kDetectionTimeoutMs) {
        success = nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, tempUid, &tempUidLength);
        if (success) break;
        delay(100);
    }

    if (!success) {
        Serial.println("No card detected");
        return MfClassicTypeNum;
    }

    std::copy(tempUid, tempUid + tempUidLength, uid);
    uidLength = tempUidLength;

    if (uidLength == 4) {
        uint8_t blockData[16];
        if (nfc->mifareclassic_ReadDataBlock(0xFF, blockData)) {
            return MfClassicType4k;
        }
        if (nfc->mifareclassic_ReadDataBlock(0x3F, blockData)) {
            return MfClassicType1k;
        }
        return MfClassicTypeMini;
    }

    return MfClassicType1k;
}

bool MfClassicPoller::authenticateBlock(uint8_t blockNum, uint8_t key, MfClassicKeyType keyType) {
    return nfc->mifareclassic_AuthenticateBlock(
        uid, uidLength, blockNum, 
        static_cast<uint8_t>(keyType), &key);
}

bool MfClassicPoller::readBlock(uint8_t blockNum, MfClassicBlock* blockData) {
    return nfc->mifareclassic_ReadDataBlock(blockNum, blockData->data);
}

bool MfClassicPoller::writeBlock(uint8_t blockNum, MfClassicBlock* blockData) {
    return nfc->mifareclassic_WriteDataBlock(blockNum, blockData->data);
}

void MfClassicPoller::dumpToSerial() {
    Serial.println("----------------------------------------");
    
    uint8_t data[16];
    bool authenticated = false;

    nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

    delay(50);

    Serial.print("  UID Length: ");
    Serial.print(uidLength, DEC);
    Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc->PrintHex(uid, uidLength);
    Serial.println();

    if (uidLength != 4) {
        Serial.println("Ooops ... this doesn't seem to be a Mifare Classic card!");
        return;
    }

    Serial.println("Seems to be a Mifare Classic card (4 byte UID)");

    for (uint8_t currentBlock = 0; currentBlock < 64; currentBlock++) {
        if (nfc->mifareclassic_IsFirstBlock(currentBlock)) {
            authenticated = false;
        }

        if (!authenticated) {
            Serial.print("------------------------Sector ");
            Serial.print(currentBlock / 4, DEC);
            Serial.println("-------------------------");
            
            authenticated = nfc->mifareclassic_AuthenticateBlock(
                uid, uidLength, currentBlock, 1, kKeyUniversal);
            
            if (!authenticated) {
                Serial.println("Authentication error");
            }
        }

        if (!authenticated) {
            Serial.print("Block ");
            Serial.print(currentBlock, DEC);
            Serial.println(" unable to authenticate");
            continue;
        }

        if (nfc->mifareclassic_ReadDataBlock(currentBlock, data)) {
            Serial.print("Block ");
            Serial.print(currentBlock, DEC);
            Serial.print((currentBlock < 10) ? "  " : " ");
            nfc->PrintHexChar(data, 16);
        } else {
            Serial.print("Block ");
            Serial.print(currentBlock, DEC);
            Serial.println(" unable to read this block");
        }
    }
}