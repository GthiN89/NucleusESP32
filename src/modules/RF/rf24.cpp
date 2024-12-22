#include "rf24.h"
#include "Arduino.h"
#include "globals.h"

struct MyData {
    int id;
    float value;
    char message[32];
};

#define CHANNELS  64
int channel[CHANNELS];


MyData dataRFTEST = {1, 23.45, "Hello, RF24!"};

RF24 radio(RF24_CE, RF24_CS);

RF24State RF24CurrentState = RF24_STATE_IDLE;

void enableRF24() {
  //  vspi.beginTransaction(spiSettings);
         digitalWrite(RF24_CS, LOW);
    radio.begin(RF24_CE, RF24_CS); 
         digitalWrite(RF24_CS, LOW);
  //  vspi.endTransaction();

    Serial.println("RF24 enable");
}
void jam(){
   int channel = 0;
  const char text[] = "xxxxxxxxxxxxxxxx"; // send the noise
  for (int i = ((channel * 5) + 1); i < ((channel * 5) + 23); i++) {
    radio.setChannel(i);
    radio.write( & text, sizeof(text));
  }

}

void testRF24() {
    if(    RF24CurrentState == RF24_STATE_TEST) {
             digitalWrite(SD_CS, HIGH);
             digitalWrite(CC1101_CS, HIGH);
             digitalWrite(RF24_CS, LOW);
    radio.testCarrier();
   // radio.write(&dataRFTEST, sizeof(dataRFTEST));
    jam();

     digitalWrite(RF24_CS, HIGH);
    }


}
   


