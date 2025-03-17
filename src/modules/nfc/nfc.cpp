#include "nfc.h"
#include "protocols/MifareHandler.h"

namespace NFC {

NFC_CLASS::NFC_CLASS()
{
    // Initialize example Key B
    _keyB[0] = 0xFF; 
    _keyB[1] = 0xFF; 
    _keyB[2] = 0xFF; 
    _keyB[3] = 0xFF; 
    _keyB[4] = 0xFF; 
    _keyB[5] = 0xFF;
}

bool NFC_CLASS::init() {
    pinMode(PN532_SS, OUTPUT);
    digitalWrite(PN532_SS, LOW);

    // Re-init SPI with your pins
    SPI.end();
    
    delay(10);

    SPI.begin(CYD_SCLK, CYD_MISO, CYD_MOSI);
    delay(10);

    nfc.begin();
    uint32_t versiondata = nfc.getFirmwareVersion();
    if(!versiondata) {
        Serial.println("PN532 not found");
        while(true) { delay(10); }
    }

    // Print version info
    Serial.print("Found chip PN5");
    Serial.println((versiondata >> 24) & 0xFF, HEX);
    Serial.print("Firmware ver. ");
    Serial.print((versiondata >> 16) & 0xFF, DEC);
    Serial.print('.');
    Serial.println((versiondata >> 8) & 0xFF, DEC);

    // Example optional calls:
    // nfc.setPassiveActivationRetries(0xFF);
    // nfc.SAMConfig();

    return true;
}

void NFC_CLASS::NFCloop() {
    uint8_t uid[7] = {0};
    uint8_t uidLen = 0;

    bool success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A,
                                           uid, &uidLen,
                                           1000);
    if(success) {
        Serial.println("\nCard detected!");
        Serial.print("UID Length: "); Serial.println(uidLen);
        Serial.print("UID Value: ");
        for(uint8_t i=0; i<uidLen; i++){
            Serial.print(uid[i], HEX); 
            Serial.print(" ");
        }
        Serial.println();

        // Attempt Mifare
        attemptMifareRead(uid, uidLen);

        // Attempt EMV
        attemptEmvSelectPPSE();
    }

    uint8_t command[] = { 
        0x40,  // InDataExchange
        0x01,  // Target number (first tag)
        0x60,  // MIFARE Authentication (Key A)
        0x9F,   // Block number
        0x19
    };

    // uint8_t rawCmd[] = {
    // //     0x80,  // InDataExchange command
    // //     0XCA,
    // //     0x01,  // Target number
    // //     0x9F,  // 0x5F (High byte)
    // //     0x19 // 0x20 (Low byte)
    // // };

    Serial.println("Sending raw NFC command...");

    // Send the raw command to the PN532 via SPI
    nfc.writecommand(command, sizeof(command));


    // Read the response
    uint8_t response[512];
    nfc.readdata(response, sizeof(response));

    // Print the raw response
    Serial.print("Response: ");
    for (int i = 0; i < sizeof(response); i++) {
        Serial.print(response[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

void NFC_CLASS::attemptMifareRead(uint8_t* uid, uint8_t uidLen) {
    Serial.println("Trying Mifare Classic read (block #4)...");
    MifareHandler mifare;

    uint8_t data[16];
    bool ok = mifare.readBlock(nfc, uid, uidLen,
                               4,       // block number
                               1,       // Key Type => KEY_B
                               _keyB,   // key
                               data);
    if(!ok) {
        Serial.println("Mifare Auth failed or not a Mifare Classic card.");
        return;
    }
    // Print block data
    Serial.print("Block #4: ");
    for(int i=0; i<16; i++){
        Serial.print(data[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

void NFC_CLASS::attemptEmvSelectPPSE() {
    Serial.println("Trying EMV SELECT PPSE...");
 
}

} // namespace NFC