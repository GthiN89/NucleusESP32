#ifndef FAKE_MFRC522_H
#define FAKE_MFRC522_H

#include <Arduino.h>

/**
 * A fake implementation of the MFRC522 class used to simulate NFC card events.
 */
class FakeMFRC522 {
public:
    // A structure for the UID, matching the real interface
    struct {
        byte uidByte[10];
        byte size;
    } uid;

    bool _newCardPresent;
    bool _readCardSerial;
    bool _softPowerDown;

    FakeMFRC522(uint8_t ssPin, uint8_t resetPin)
        : _newCardPresent(false), _readCardSerial(false), _softPowerDown(false) {
        uid.size = 4;
        for (byte i = 0; i < 10; i++) {
            uid.uidByte[i] = 0;
        }
    }

    void PCD_Init() {
        _softPowerDown = false;
    }

    void PCD_SoftPowerDown() {
        _softPowerDown = true;
    }

    bool PICC_IsNewCardPresent() {
        return _newCardPresent;
    }

    bool PICC_ReadCardSerial() {
        return _readCardSerial;
    }

    void PICC_HaltA() {
        _newCardPresent = false;
        _readCardSerial = false;
    }

    void PCD_StopCrypto1() {
        // Do nothing in fake implementation
    }

    // Test helper methods
    void simulateNewCard(const byte* uidBytes, byte size) {
        if (size > 10) size = 10;
        uid.size = size;
        for (byte i = 0; i < size; i++) {
            uid.uidByte[i] = uidBytes[i];
        }
        _newCardPresent = true;
        _readCardSerial = true;
    }
};

// For testing, alias MFRC522 to FakeMFRC522
#define MFRC522 FakeMFRC522

#endif  // FAKE_MFRC522_H 