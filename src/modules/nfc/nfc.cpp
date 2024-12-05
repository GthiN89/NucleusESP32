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
    Serial.println("RFID enable");
  MFRC522Debug::PCD_DumpVersionToSerial(mfrc522, Serial);	// Show details of PCD - MFRC522 Card Reader details.
	Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
}

void readLoop(){
  	// Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
	if ( !mfrc522.PICC_IsNewCardPresent()) {
		return;
	}

	// Select one of the cards.
	if ( !mfrc522.PICC_ReadCardSerial()) {
		return;
	}

	// Dump debug info about the card; PICC_HaltA() is automatically called.
  MFRC522Debug::PICC_DumpToSerial(mfrc522, Serial, &(mfrc522.uid));

    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      Serial.print(F("Reader "));
      static uint8_t i = 0;
      i++;
      Serial.print(i);
      
      // Show some details of the PICC (that is: the tag/card).
      Serial.print(F(": Card UID:"));
      MFRC522Debug::PrintUID(Serial, mfrc522.uid);
      Serial.println();
      
      Serial.print(F("PICC type: "));
      MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
      Serial.println(MFRC522Debug::PICC_GetTypeName(piccType));
      
      // Halt PICC.
      mfrc522.PICC_HaltA();
      // Stop encryption on PCD.
      mfrc522.PCD_StopCrypto1();
    
  }
}

void disableRFID() {
    mfrc522.PCD_SoftPowerDown();
    digitalWrite(RFID_CS, HIGH);

}