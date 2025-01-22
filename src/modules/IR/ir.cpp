#include <Arduino.h>


#include "ir.h"
#define NUM_ELEM(x) (sizeof (x) / sizeof (*(x)));


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

        
    irsend.sendRC6(0x1, 20, 1);
    delay(500);
     irsend.sendRC6(0x2, 20, 1);
    delay(500);
     irsend.sendRC6(0x3, 20, 1);
    delay(500);
 //    irsend.sendRaw(0x1000C, 20, 10);
    delay(500);
//   irsend.sendRAW();
//   	    delay(50);

 //   }
}



uint8_t IR_CLASS::read_bits(uint8_t count)
{
  
  uint8_t tmp = 0;
  for (i = 0; i < count; i++) {
    if (bitsleft_r == 0) {
      bits_r = powerCode->codes[code_ptr++];
      bitsleft_r = 8;
    }
    bitsleft_r--;
    tmp |= (((bits_r >> (bitsleft_r)) & 1) << (count - 1 - i));
  }
  return tmp;
}

void IR_CLASS::TVbGONE() {
  uint16_t ontime, offtime;
  uint16_t i = 0;
  uint16_t rawData[300];
  uint16_t codesNum = NUM_ELEM(EUpowerCodes);

      for (i=0 ; i< codesNum; i++) {


        const uint8_t freq = powerCode->timer_val;
        const uint8_t numpairs = powerCode->numpairs;
        const uint8_t bitcompression = powerCode->bitcompression;

        for (uint8_t k=0; k<numpairs; k++) {
          uint16_t ti;
          ti = (IR_CLASS::read_bits(bitcompression)) * 2;
          offtime = powerCode->times[ti];  // read word 1 - ontime
          ontime = powerCode->times[ti + 1]; // read word 2 - offtime

          rawData[k*2] = offtime * 10;
          rawData[(k*2)+1] = ontime * 10;
        }
        irsend.sendRaw(rawData, (numpairs*2) , freq);
        delayMicroseconds(20500);


        }
}

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


