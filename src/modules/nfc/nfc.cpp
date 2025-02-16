#include "nfc.h"
#include <stdexcept>

#ifdef ARDUINO
  #include <Arduino.h>
  #define LOG_PRINTLN(msg) Serial.println(F(msg))
  #define LOG_PRINT(msg) Serial.print(F(msg))
  #define LOG_PRINT_VAR(msg) Serial.print(msg)
  #define HEX_FORMAT 16
#else
  #define LOG_PRINTLN(msg) 
  #define LOG_PRINT(msg) 
  #define LOG_PRINT_VAR(msg)
  #define HEX_FORMAT 16
#endif

NFC::NFC(std::unique_ptr<INFCReader> readerImplementation)
    : reader(std::move(readerImplementation))
    , callback(nullptr) {
    if (!reader) {
        throw std::invalid_argument("NFC reader implementation cannot be null");
    }
}

void NFC::begin() {
    if (!reader) {
        throw std::runtime_error("NFC reader not initialized");
    }

    try {
        reader->initialize();
        LOG_PRINTLN("NFC module initialized");
    } catch (const std::exception& e) {
        LOG_PRINT("NFC initialization failed: ");
        LOG_PRINT_VAR(e.what());
        state = NFCState::Idle;
        throw;
    }
}

void NFC::update() {
    if (!reader) {
        return;
    }

    try {
        if (reader->isNewCardPresent() && reader->readCardSerial()) {
            state = NFCState::Reading;
            const auto uid = reader->getUID();

            if (uid.size > 0) {
                String uidStr("");
                uidStr = formatUID(uid.data.data(), uid.size);
                LOG_PRINT("NFC Card detected: ");
                LOG_PRINT_VAR(uidStr);

                if (callback) {
                    try {
                        callback(uidStr);
                    } catch (...) {
                        LOG_PRINTLN("Error in card detection callback");
                    }
                }
            }

            reader->halt();
            state = NFCState::Idle;
        }
    } catch (const std::exception& e) {
        LOG_PRINT("Error during NFC update: ");
        LOG_PRINT_VAR(e.what());
        state = NFCState::Idle;
    }
}

void NFC::shutdown() {
    if (!reader) {
        return;
    }

    try {
        reader->powerDown();
        state = NFCState::Idle;
        LOG_PRINTLN("NFC module shutdown");
    } catch (const std::exception& e) {
        LOG_PRINT("Error during NFC shutdown: ");
        LOG_PRINT_VAR(e.what());
        throw;
    }
}

[[nodiscard]] auto NFC::getState() const -> NFCState {
    return state;
}

void NFC::setCardDetectedCallback(CardDetectedCallback callback) {
    this->callback = std::move(callback);
}

[[nodiscard]] auto NFC::formatUID(const byte* uidBytes, byte uidSize) const -> String {
    if (uidBytes == nullptr || uidSize == 0 || uidSize > 10) {
        return {};
    }

    String result("");
    result.reserve(static_cast<size_t>(uidSize) * 2);  // Pre-allocate space for efficiency

    for (byte i = 0; i < uidSize; i++) {
        if (uidBytes[i] < 0x10) {
            result += '0';
        }
        result += String(uidBytes[i], HEX_FORMAT);
    }
    result.toUpperCase();
    return result;
}

// Remaining setter/getter implementations...