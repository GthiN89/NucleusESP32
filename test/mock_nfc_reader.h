#ifndef MOCK_NFC_READER_H
#define MOCK_NFC_READER_H

#include "../src/modules/nfc/infc_reader.h"
#include <stdexcept>

class MockNFCReader : public INFCReader {
public:
    MockNFCReader() 
        : initialized(false)
        , powered_down(false)
        , card_present(false)
        , serial_read(false)
        , should_fail_init(false) {
        // Initialize with a default test UID
        test_uid.size = 4;
        test_uid.data[0] = 0xDE;
        test_uid.data[1] = 0xAD;
        test_uid.data[2] = 0xBE;
        test_uid.data[3] = 0xEF;
    }

    void initialize() override {
        if (should_fail_init) {
            throw std::runtime_error("Simulated initialization failure");
        }
        initialized = true;
        powered_down = false;
    }

    void powerDown() override {
        powered_down = true;
        initialized = false;
        card_present = false;
        serial_read = false;
    }

    bool isNewCardPresent() override {
        return initialized && !powered_down && card_present;
    }

    bool readCardSerial() override {
        if (isNewCardPresent()) {
            serial_read = true;
            return true;
        }
        return false;
    }

    UID getUID() const override {
        if (!initialized || !serial_read) {
            UID empty_uid;
            empty_uid.size = 0;
            return empty_uid;
        }
        return test_uid;
    }

    void halt() override {
        card_present = false;
        serial_read = false;
    }

    // Test helper methods
    void simulateCardPresent(bool present = true) {
        card_present = present && initialized && !powered_down;
    }

    void setTestUID(const byte* uid, byte size) {
        if (!uid) {
            test_uid.size = 0;
            return;
        }
        
        test_uid.size = size;
        if (test_uid.size > sizeof(test_uid.data)) {
            test_uid.size = sizeof(test_uid.data);
        }
        
        for (byte i = 0; i < test_uid.size; i++) {
            test_uid.data[i] = uid[i];
        }
    }

    void setInitializationShouldFail(bool should_fail) {
        should_fail_init = should_fail;
    }

    bool isInitialized() const { return initialized; }
    bool isPoweredDown() const { return powered_down; }
    bool isCardPresent() const { return card_present; }
    bool isSerialRead() const { return serial_read; }

private:
    bool initialized;
    bool powered_down;
    bool card_present;
    bool serial_read;
    bool should_fail_init;
    UID test_uid;
};

#endif // MOCK_NFC_READER_H 