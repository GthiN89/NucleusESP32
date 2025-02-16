#include "mfrc522_reader.h"
#include <SPI.h>
#include <stdexcept>

MFRC522Reader::MFRC522Reader(uint8_t ssPin, uint8_t rstPin)
    : mfrc(ssPin, rstPin)
    , initialized(false) {
}

void MFRC522Reader::initialize() {
    try {
        SPI.begin();
        mfrc.PCD_Init();
        
        // Verify the reader is responding
        byte version = mfrc.PCD_ReadRegister(MFRC522::VersionReg);
        if (version != 0x91 && version != 0x92) {
            throw std::runtime_error("Failed to communicate with MFRC522");
        }

        initialized = true;
        Serial.println(F("MFRC522 Reader initialized"));
    }
    catch (const std::exception& e) {
        initialized = false;
        Serial.print(F("MFRC522 initialization failed: "));
        Serial.println(e.what());
        throw;
    }
}

void MFRC522Reader::powerDown() {
    if (initialized) {
        mfrc.PCD_SoftPowerDown();
        initialized = false;
        Serial.println(F("MFRC522 Reader powered down"));
    }
}

bool MFRC522Reader::isNewCardPresent() {
    if (!initialized) {
        return false;
    }
    return mfrc.PICC_IsNewCardPresent();
}

bool MFRC522Reader::readCardSerial() {
    if (!initialized) {
        return false;
    }
    return mfrc.PICC_ReadCardSerial();
}

UID MFRC522Reader::getUID() const {
    UID uid;
    if (!initialized) {
        uid.size = 0;
        return uid;
    }

    uid.size = mfrc.uid.size;
    if (uid.size > sizeof(uid.data)) {
        uid.size = sizeof(uid.data);
    }
    
    for (byte i = 0; i < uid.size; i++) {
        uid.data[i] = mfrc.uid.uidByte[i];
    }
    return uid;
}

void MFRC522Reader::halt() {
    if (initialized) {
        mfrc.PICC_HaltA();
        mfrc.PCD_StopCrypto1();
    }
} 