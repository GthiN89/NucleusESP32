#include "nfc.h"


MFRC522DriverPinSimple ss_pin(RFID_CS); // Create pin driver. See typical pin layout above.

SPIClass &spiClass = SPI; // Alternative SPI e.g. SPI2 or from library e.g. softwarespi.

const SPISettings spiSettings = SPISettings(SPI_CLOCK_DIV4, MSBFIRST, SPI_MODE0); // May have to be set if hardware is not fully compatible to Arduino specifications.

MFRC522DriverSPI driver{ss_pin, spiClass, spiSettings}; // Create SPI driver.

MFRC522 mfrc522{driver}; // Create MFRC522 instance.



NFC_STATE NFCCurrentState = NFC_IDLE;

void enableRFID() {
    digitalWrite(NRF24_CS, HIGH);
    digitalWrite(CC1101_CS, HIGH);
    digitalWrite(RFID_CS, LOW);
    SPI.begin(CYD_SCLK, CYD_MISO, CYD_MOSI, RFID_CS);
    mfrc522.PCD_Init();   // Init MFRC522 board.

}

void readLoop(){
        if ( !mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
		    return;
	    }
    MFRC522Debug::PCD_DumpVersionToSerial(mfrc522, Serial);	// Show details of PCD - MFRC522 Card Reader details.
	Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
    MFRC522Debug::PICC_DumpToSerial(mfrc522, Serial, &(mfrc522.uid));
}

void disableRFID() {
    mfrc522.PCD_SoftPowerDown();
}