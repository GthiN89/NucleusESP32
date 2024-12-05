
#include "Arduino.h"
#include "globals.h"
#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>
#include <MFRC522Debug.h>

enum NFC_STATE {
    NFC_IDLE,
    NFC_READ,
};

extern NFC_STATE NFCCurrentState;
extern volatile bool cardDetected;
extern MFRC522 mfrc522;

void enableRFID();
void readLoop();
void disableRFID();
