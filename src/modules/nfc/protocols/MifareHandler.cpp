#include "MifareHandler.h"

bool MifareHandler::readBlock(Adafruit_PN532& nfc,
                              uint8_t* uid, uint8_t uidLen,
                              uint8_t blockNum,
                              uint8_t keyType, const uint8_t* keyData,
                              uint8_t* dataOut)
{
    // Authenticate
    bool auth = nfc.mifareclassic_AuthenticateBlock(
        uid, uidLen, blockNum, keyType, (uint8_t*)keyData);
    if(!auth) {
        return false;
    }
    // If authenticated, read
    if(!nfc.mifareclassic_ReadDataBlock(blockNum, dataOut)) {
        return false;
    }
    return true;
}
