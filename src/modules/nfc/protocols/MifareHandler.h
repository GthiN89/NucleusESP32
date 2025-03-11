#ifndef MIFAREHANDLER_H
#define MIFAREHANDLER_H

#include <Arduino.h>
#include <stdint.h>
#include "Adafruit_PN532.h"

class MifareHandler {
public:
    /**
     * @brief Reads a single Mifare Classic block (16 bytes).
     * @param nfc       Reference to Adafruit_PN532
     * @param uid       Pointer to the card UID
     * @param uidLen    UID length (4 or 7 typically)
     * @param blockNum  Which block to read (e.g. 4)
     * @param keyType   0=Key A, 1=Key B
     * @param keyData   6-byte Mifare key
     * @param dataOut   16-byte buffer to store block data
     * @return true if successful, false otherwise
     */
    bool readBlock(Adafruit_PN532& nfc,
                   uint8_t* uid, uint8_t uidLen,
                   uint8_t blockNum,
                   uint8_t keyType, const uint8_t* keyData,
                   uint8_t* dataOut);
};

#endif // MIFAREHANDLER_H