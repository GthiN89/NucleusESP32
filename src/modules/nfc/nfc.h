#ifndef NFC_H
#define NFC_H

#include "globals.h"
#include "Arduino.h"
#include "Adafruit_PN532.h"
#include "SPI.h"

#define ISO14443_3A_UID_4_BYTES  (4U)
#define ISO14443_3A_UID_7_BYTES  (7U)
#define ISO14443_3A_UID_10_BYTES (10U)
#define ISO14443_3A_MAX_UID_SIZE ISO14443_3A_UID_10_BYTES



namespace NFC {

    class NFC_CLASS {
    public:
        NFC_CLASS();
    
        /**
         * @brief Initialize the PN532 over SPI.
         * @return true if found PN532, else halts.
         */
        bool init();
    
        /**
         * @brief Main polling function. Detects a card, prints UID,
         * attempts Mifare read, attempts EMV SELECT PPSE.
         */
        void NFCloop();
    
        /**
         * @brief Attempt Mifare read of block #4 with Key B.
         */
        void attemptMifareRead(uint8_t* uid, uint8_t uidLen);
    
        /**
         * @brief Attempt EMV SELECT PPSE APDU exchange.
         */
        void attemptEmvSelectPPSE();
    
    private:
        
        uint8_t _keyB[6];   ///< Example Mifare Key B
    };
    
    } // namespace NFC

#endif // NFC_H