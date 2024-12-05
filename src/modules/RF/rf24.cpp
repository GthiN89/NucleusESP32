#include "rf24.h"
#include "Arduino.h"
#include "globals.h"

struct MyData {
    int id;
    float value;
    char message[32];
};


MyData dataRFTEST = {1, 23.45, "Hello, RF24!"};

// Create custom SPI instance and RF24 instance
RF24 radio(RF24_CE, NRF24_CS);

RF24State RF24CurrentState = RF24_STATE_IDLE;

void enableRF24() {
    // digitalWrite(RFID_CS, HIGH);
    // digitalWrite(CC1101_CS, HIGH);
    // digitalWrite(NRF24_CS, LOW);

    // pinMode(NRF24_CS, OUTPUT);
    // digitalWrite(NRF24_CS, HIGH); 

    // pinMode(RF24_CE, OUTPUT);
    // digitalWrite(RF24_CE, LOW); 

    // SPI.begin(CYD_SCLK,CYD_MISO,CYD_MOSI,NRF24_CS);
    radio.begin(&SPI);

    Serial.println("RF24 enable");
}

void testRF24() {

    digitalWrite(RFID_CS, HIGH);
    digitalWrite(CC1101_CS, HIGH);

    digitalWrite(NRF24_CS, LOW);

    radio.write(&dataRFTEST, sizeof(dataRFTEST));

    digitalWrite(NRF24_CS, HIGH);
}



