#include <Arduino.h>


#include "ir.h"
#include "globals.h"
#include <lvgl.h>
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

String protDecode[] = {
    "Unknown",
    "01 Princeton, PT-2240",
    "02 AT-Motor?",
    "03",
    "04",
    "05",
    "06 HT6P20B",
    "07 HS2303-PT, i. e. used in AUKEY Remote",
    "08 Conrad RS-200 RX",
    "09 Conrad RS-200 TX",
    "10 1ByOne Doorbell",
    "11 HT12E",
    "12 SM5212",
    "13 Mumbi RC-10",
    "14 Blyss Doorbell Ref. DC6-FR-WH 656185",
    "15 sc2260R4",
    "16 Home NetWerks Bathroom Fan Model 6201-500",
    "17 ORNO OR-GB-417GD",
    "18 CLARUS BHC993BF-3",
    "19 NEC",
    "20 CAME 12bit",
    "21 FAAC 12bit",
    "22 NICE 12bit"
};



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

char * IR_CLASS::dec2binWzerofill(unsigned long Dec, unsigned int bitLength) {
  static char bin[64]; 
  unsigned int i=0;

  while (Dec > 0) {
    bin[32+i++] = ((Dec & 1) > 0) ? '1' : '0';
    Dec = Dec >> 1;
  }

  for (unsigned int j = 0; j< bitLength; j++) {
    if (j >= bitLength - i) {
      bin[j] = bin[ 31 + i - (j - (bitLength - i)) ];
    } else {
      bin[j] = '0';
    }
  }
  bin[bitLength] = '\0';
  
  return bin;
}

 void IR_CLASS::output(unsigned long decimal, unsigned int length, unsigned int delay, unsigned int* raw, unsigned int protocol, lv_obj_t * textareaRC) {

  const char* b = dec2binWzerofill(decimal, length);
//    lv_obj_t * textareaRC;

  lv_textarea_set_text(textareaRC, "\nDecimal: ");
  lv_textarea_add_text(textareaRC, String(decimal).c_str());
  lv_textarea_add_text(textareaRC, "\n (");
  lv_textarea_add_text(textareaRC,  String(length).c_str() );
  lv_textarea_add_text(textareaRC, "Bit) Binary: ");
  lv_textarea_add_text(textareaRC,  b );
  lv_textarea_add_text(textareaRC, "\nTri-State: ");
  lv_textarea_add_text(textareaRC,  IR_CLASS::bin2tristate( b) );
  lv_textarea_add_text(textareaRC, "\nPulseLength: ");
  lv_textarea_add_text(textareaRC, String(delay).c_str());
  lv_textarea_add_text(textareaRC, " micro");
  lv_textarea_add_text(textareaRC, "\nProtocol: ");
  lv_textarea_add_text(textareaRC, String(IR_CLASS::protDecode[protocol - 1]).c_str());
  
  //Serial.print("Raw data: ");
  for (unsigned int i=0; i<= length*2; i++) {
    //Serial.print(raw[i]);
    //Serial.print(",");
  }
  //Serial.println();
  //Serial.println();
}

const char* IR_CLASS::bin2tristate(const char* bin) {
  static char returnValue[50];
  int pos = 0;
  int pos2 = 0;
  while (bin[pos]!='\0' && bin[pos+1]!='\0') {
    if (bin[pos]=='0' && bin[pos+1]=='0') {
      returnValue[pos2] = '0';
    } else if (bin[pos]=='1' && bin[pos+1]=='1') {
      returnValue[pos2] = '1';
    } else if (bin[pos]=='0' && bin[pos+1]=='1') {
      returnValue[pos2] = 'F';
    } else {
      return "not applicable";
    }
    pos = pos+2;
    pos2++;
  }
  returnValue[pos2] = '\0';
  return returnValue;
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


