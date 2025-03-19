
#include "nfc.h"
#include "protocols/MifareHandler.h"
#include "protocols/felica.h"
#include <aes/esp_aes.h> 


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

    // Optional: nfc.setPassiveActivationRetries(0xFF);
    // Optional: nfc.SAMConfig();

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
        for(uint8_t i = 0; i < uidLen; i++){
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
        0x9F,  // Block number
        0x19
    };

    Serial.println("Sending raw NFC command...");
    nfc.writecommand(command, sizeof(command));

    uint8_t response[512];
    nfc.readdata(response, sizeof(response));

    Serial.print("Response: ");
    for (int i = 0; i < sizeof(response); i++) {
        Serial.print(response[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    FelicaData* data = felica_alloc();
    felica_reset(data);

    uint8_t uid_example[FELICA_IDM_SIZE] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    if (felica_set_uid(data, uid_example, FELICA_IDM_SIZE)) {
        Serial.println("UID set successfully");
    }

    size_t uid_len;
    const uint8_t* felica_uid = felica_get_uid(data, &uid_len);
    Serial.print("UID: ");
    for (size_t i = 0; i < uid_len; i++) {
        Serial.print(felica_uid[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

     Format ff;
    ff.data = "Dummy format data";

    if(felica_load(data, &ff, 1)) {
        Serial.println("Felica data loaded successfully.");
    } else {
        Serial.println("Felica data load failed.");
    }

    // --- AES-based MAC calculation example ---
    esp_aes_context ctx;
    esp_aes_init(&ctx);
    uint8_t ck[16] = {0};   // Dummy 16-byte card key
    uint8_t rc[16] = {0};   // Dummy 16-byte random challenge
    uint8_t session_key[FELICA_DATA_BLOCK_SIZE];
    felica_calculate_session_key_aes(&ctx, ck, rc, session_key);
    Serial.print("Session key: ");
    for (uint8_t i = 0; i < FELICA_DATA_BLOCK_SIZE; i++) {
        Serial.print(session_key[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
    esp_aes_free(&ctx);

    if(felica_save(data, &ff)) {
        Serial.println("Felica data saved successfully.");
    } else {
        Serial.println("Felica data save failed.");
    }

    felica_free(data);
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
    Serial.print("Block #4: ");
    for (int i = 0; i < 16; i++){
        Serial.print(data[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

void NFC_CLASS::attemptEmvSelectPPSE() {
    Serial.println("Trying EMV SELECT PPSE...");
}

} // namespace NFC
