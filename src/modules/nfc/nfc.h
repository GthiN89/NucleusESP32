#ifndef NFC_H
#define NFC_H

#include "globals.h"
#include "Arduino.h"
#include "Adafruit_PN532.h"
#include "SPI.h"

namespace NFC {

class NFC_CLASS {
public:
    NFC_CLASS();
    bool init();
    Adafruit_PN532 nfc;
    void NFCloop(void);
private:

};

}

#endif