#include <Arduino.h>


#include "ir.h"


typedef struct IR_DATA
{
    uint8_t                             protocol;                                   // protocol, e.g. NEC_PROTOCOL
    uint16_t                            address;                                    // address
    uint16_t                            command;                                    // command
    uint8_t                             flags;                                      // flags, e.g. repetition
} IR_DATA;

IR_DATA* IRData;

IRsend irsend(IR_TX);
IRrecv irrecv(IR_RX);

IRState IRCurrentState = IR_STATE_IDLE;
void IR_CLASS::setupIR()
{
//	xTaskCreatePinnedToCore(recvIR, "recvIR", 2048, NULL, 10, NULL, 1);
  irsend.begin();
}

void IR_CLASS::receiveIR() {
    irrecv.enableIRIn();  // Start the receiver
    IRCurrentState =   IR_STATE_LISTENING;

}

void IR_CLASS::sendPower() {
  //  for(int i = 0; IR_CLASS::power.size(); i++) {

        
    irsend.sendRC6(0x0, 0x1, 1);
    delay(500);
     irsend.sendRC6(0x0, 0x2, 1);
    delay(500);
     irsend.sendRC6(0x0, 0x3, 1);
    delay(500);
     irsend.sendRC6(0x0, 0x3, 10);
    delay(500);
//   irsend.sendRAW();
//   	    delay(50);

 //   }
}

// void IR_CLASS::Receive() {

// }
// void irReceived(irproto brand, uint32_t code, size_t len, rmt_symbol_word_t *item){
// // //	if( code ){
// // 		Serial.printf("IR %s, code: %#x, bits: %d\n",  proto[brand].name, code, len);
// // //	}

// // 	if(true){//debug
// // 		Serial.printf("Rx%d: ", len);							
// // 		for (uint8_t i=0; i < len ; i++ ) {
// // 			int d0 = item[i].duration0; if(!item[i].level0){d0 *= -1;}
// // 			int d1 = item[i].duration1; if(!item[i].level1){d1 *= -1;}
// // 			Serial.printf("%d,%d ", d0, d1);
// // 		}								
// // 		Serial.println();
// // 	}
  
// }
void sendVolumeUp() {

}
void sendVolumeDown() {

}
void sendMute() {

}
void sendChannelUp() {

}

void sendChannelDown() {

}


