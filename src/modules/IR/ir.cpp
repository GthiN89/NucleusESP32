#include <IRremoteESP8266.h>
#include <IRsend.h>
#include "ir.h"
#include "main.h"

// Define the IR transmitter pin
#define IR_TX_PIN IR_TX
IRsend irsend(IR_TX_PIN); // IRsend object for transmitting IR signals
IRState IRCurrentState;
decode_results results;  


   

void sendReceived() {

        irsend.begin();
        irsend.send(lastResults.decode_type, lastResults.value, lastResults.bits);


}
