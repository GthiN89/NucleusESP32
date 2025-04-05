#include "iso14443astarget.h"

ISO14443ASTarget::ISO14443ASTarget()
 
{}

void ISO14443ASTarget::begin() {

    // uint32_t versiondata = nfc.getFirmwareVersion();
    // if (!versiondata) {
    //     Serial.println("Didn't find PN53x board");
    //     while (1);
    // }
    // Serial.print("Found chip PN5");
    // Serial.println((versiondata >> 24) & 0xFF, HEX);
    // Serial.print("Firmware ver. ");
    // Serial.print((versiondata >> 16) & 0xFF, DEC);
    // Serial.print('.');
    // Serial.println((versiondata >> 8) & 0xFF, DEC);

    // nfc.setPassiveActivationRetries(0xFF);
    // Serial.println("As Target... Approach the NFC PN532 Board to a PoS or terminal!");
    // delay(200);
}

void ISO14443ASTarget::process() {
    // uint8_t apdubuffer[255] = {};
    // uint8_t apdulen = 0;
    // uint8_t ppse[] = {
    //     0x8E, 0x6F, 0x23, 0x84, 0x0E, 0x32, 0x50, 0x41, 0x59, 0x2E,
    //     0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0xA5,
    //     0x11, 0xBF, 0x0C, 0x0E, 0x61, 0x0C, 0x4F, 0x07, 0xA0, 0x00,
    //     0x00, 0x00, 0x03, 0x10, 0x10, 0x87, 0x01, 0x01, 0x90, 0x00
    // };

    // nfc.AsTarget();
    // nfc.getDataTarget(apdubuffer, &apdulen);
    // if (apdulen > 0) {
    //     for (uint8_t i = 0; i < apdulen; i++) {
    //         Serial.print(" 0x");
    //         Serial.print(apdubuffer[i], HEX);
    //     }
    //     Serial.println();
    // }

    // nfc.setDataTarget(ppse, sizeof(ppse));
    // nfc.getDataTarget(apdubuffer, &apdulen);
    // if (apdulen > 0) {
    //     for (uint8_t i = 0; i < apdulen; i++) {
    //         Serial.print(" 0x");
    //         Serial.print(apdubuffer[i], HEX);
    //     }
    //     Serial.println();
    // }

    // delay(1000);
}
