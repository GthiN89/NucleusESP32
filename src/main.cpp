#include <Arduino.h>
#include <XPT2046_Bitbang.h>
#include "GUI/ScreenManager.h"
#include "esp32_smartdisplay/src/esp32_smartdisplay.h"
#include "GUI/events.h"
#include "modules/RF/CC1101.h"
#include "modules/ETC/SDcard.h"
#include <FFat.h>
#include "lv_fs_if.h"
#include "modules/dataProcessing/SubGHzParser.h"


#include <Wire.h>
#include <SPI.h>

#include "GUI/logo.h"

#include <IRrecv.h>
#include <IRutils.h>
#include "modules/IR/ir.h"
IRrecv Irrecv(IR_RX);

decode_results results;
IRsend Irsend(IR_TX);



SDcard& SD_CARD = SDcard::getInstance();

XPT2046_Bitbang touchscreen(MOSI_PIN, MISO_PIN, CLK_PIN, CS_PIN);
ScreenManager& screenMgrM = ScreenManager::getInstance();
static lv_indev_t *indev = nullptr;
TouchCallback _singleTouchCallback;

void register_touch(lv_disp_t *disp);
void my_touchpad_read(lv_indev_t * indev_driver, lv_indev_data_t * data);
void init_touch(TouchCallback singleTouchCallback);

 void init_touch(TouchCallback singleTouchCallback) {
     Serial.println(F("Initializing touch."));
     touchscreen.begin(); 
     _singleTouchCallback = singleTouchCallback; 
     Serial.println(F("Touch initialized."));
 }

 const uint32_t subghz_frequency_list[] = {
     300000000, 303875000, 304250000, 310000000, 315000000, 318000000,  //  300-348 MHz
     390000000, 418000000, 433075000, 433420000, 433920000, 434420000, 434775000, 438900000,  //  387-464 MHz
     868350000, 868000000, 915000000, 925000000  //  779-928 MHz
 };
static const char* bin2tristate(const char* bin);
static char * dec2binWzerofill(unsigned long Dec, unsigned int bitLength);

static char * dec2binWzerofill(unsigned long Dec, unsigned int bitLength) {
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

 void output(unsigned long decimal, unsigned int length, unsigned int delay, unsigned int* raw, unsigned int protocol) {

  const char* b = dec2binWzerofill(decimal, length);
lv_obj_t * textareaRC;
      if(C1101preset == CUSTOM){
        textareaRC = screenMgrM.text_area_SubGHzCustom;        
    } else {
       // Serial.println("Signal preset");
        textareaRC = screenMgrM.getTextArea();
    }
  lv_textarea_set_text(textareaRC, "\nDecimal: ");
  lv_textarea_add_text(textareaRC, String(decimal).c_str());
  lv_textarea_add_text(textareaRC, "\n (");
  lv_textarea_add_text(textareaRC,  String(length).c_str() );
  lv_textarea_add_text(textareaRC, "Bit) Binary: ");
  lv_textarea_add_text(textareaRC,  b );
  lv_textarea_add_text(textareaRC, "\nTri-State: ");
  lv_textarea_add_text(textareaRC,  bin2tristate( b) );
  lv_textarea_add_text(textareaRC, "\nPulseLength: ");
  lv_textarea_add_text(textareaRC, String(delay).c_str());
  lv_textarea_add_text(textareaRC, " micro");
  lv_textarea_add_text(textareaRC, "\nProtocol: ");
  lv_textarea_add_text(textareaRC, String(protDecode[protocol - 1]).c_str());
  
  Serial.print("Raw data: ");
  for (unsigned int i=0; i<= length*2; i++) {
    Serial.print(raw[i]);
    Serial.print(",");
  }
  Serial.println();
  Serial.println();
}

static const char* bin2tristate(const char* bin) {
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



void setup() {

  Serial.begin(115200);
  init_touch([]() { Serial.println(F("Single touch detected!")); });
  smartdisplay_init();
  auto disp = lv_disp_get_default();

  #ifdef CYDV2
    touchscreen.setCalibration(153, 123, 1915, 1824);
  #endif
  #ifdef CYDV3
    touchscreen.setCalibration(180, 197, 1807, 1848);
  #endif
    screenMgrM.draw_image();
    lv_task_handler();
    delay(3000);
    screenMgrM.createmainMenu();
    register_touch(disp);
    SPI.begin(CYD_SCLK, CYD_MISO, CYD_MOSI);

    if (!SD_CARD.initializeSD()) {
        Serial.println(F("Failed to initialize SD card!"));
    }
    lv_fs_if_init();

    if (CC1101.init()) {
        Serial.println(F("CC1101 initialized."));
        CC1101.emptyReceive();
    } else {
        Serial.println(F("Failed to initialize CC1101."));
    }

   // pinMode(IR_RX, INPUT_PULLUP);

    pinMode(26, OUTPUT);
    
    

}

RCSwitch mySwitch1;
 void CC1101Loop() {
    if(C1101CurrentState == STATE_ANALYZER) {
               // delay(50);
                Serial.println(gpio_get_level(CC1101_CCGDO2A));
        if (CC1101.CheckReceived()) {
             delay(50);
            Serial.println("Received");
            CC1101.disableReceiver();
            Serial.println("Receiver disabled.");
            delay(50);
            Serial.println("Analyzing signal...");
            CC1101.signalAnalyse();
            Serial.println("Signal analyzed.");
 

            C1101CurrentState = STATE_IDLE;
            runningModule = MODULE_NONE;
        }
    }
    if(C1101CurrentState == STATE_RCSWITCH) {
               // delay(50);
                Serial.println(gpio_get_level(CC1101_CCGDO2A));
        if (mySwitch1.available()) {
             delay(50);
            output(mySwitch1.getReceivedValue(), mySwitch1.getReceivedBitlength(), mySwitch1.getReceivedDelay(), mySwitch1.getReceivedRawdata(),mySwitch1.getReceivedProtocol());
            mySwitch1.resetAvailable();
 

            C1101CurrentState = STATE_IDLE;
            runningModule = MODULE_NONE;
        }
    }
    if(C1101CurrentState == STATE_PLAYBACK) {
        CC1101.initRaw();
        CC1101.sendRaw();
        CC1101.disableTransmit();
        C1101CurrentState = STATE_IDLE;
        runningModule = MODULE_NONE;
    }
    if(C1101CurrentState == STATE_DETECT) {
        lv_label_set_text(screenMgrM.detectLabel, 
        (String("Frequencies:\n") +
        "Frequency: " + strongestASKFreqs[0] + " MHz | RSSI: " + strongestASKRSSI[0] + "\n" +
        "Frequency: " + strongestASKFreqs[1] + " MHz | RSSI: " + strongestASKRSSI[1] + "\n" +
        "Frequency: " + strongestASKFreqs[2] + " MHz | RSSI: " + strongestASKRSSI[2] + "\n" +
        "Frequency: " + strongestASKFreqs[3] + " MHz | RSSI: " + strongestASKRSSI[3] + "\n\n")
        .c_str());
    }

    if(C1101CurrentState == STATE_SEND_FLIPPER) {
        SubGHzParser parser;
        parser.loadFile(EVENTS::fullPath);
        SubGHzData data = parser.parseContent();
    }
    if(C1101CurrentState == STATE_IDLE) {
        updatetransmitLabel = false;
        delay(20);
        runningModule = MODULE_NONE;
    }  
}
 
 void IRLoop() {
   
   if(IRCurrentState == IR_STATE_PLAYBACK) {
        Irsend.send(
            results.decode_type,
            results.value,
            results.bits,
            1
            );
   }
    
    if(IRCurrentState == IR_STATE_BGONE) {
        IR_CLASS ir;
        ir.TVbGONE();
    }

    if(IRCurrentState == IR_STATE_LISTENING) {
        
        if (Irrecv.decode(&results)) {
            IRCurrentState = IR_STATE_IDLE;
            runningModule = MODULE_NONE;
            Serial.print(resultToHumanReadableBasic(&results));
            lv_textarea_set_text(screenMgrM.text_area_IR, "Received\n");
            lv_textarea_add_text(screenMgrM.text_area_IR, String(resultToHumanReadableBasic(&results)).c_str());
         //   lastResults = results; 
            Irrecv.resume();
        }
    }


}
 
  ulong next_millis;
  auto lv_last_tick = millis();
 
 auto previousMillis = millis();
 
 void loop() {
     auto const now = millis();
   lv_tick_inc(now - lv_last_tick);
   lv_last_tick = now;
   lv_timer_handler();

   switch (runningModule)
   {
    case MODULE_CC1101:
        CC1101Loop();
        break;
    case MODULE_IR:
        IRLoop();
    break;
   default:
    break;
   }
       if(updatetransmitLabel) {
        String text = "Transmitting\n Codes send: " + String(codesSend);
        lv_label_set_text(label_sub, text.c_str());        
    }

}
 
 bool touched() {
Point touch = touchscreen.getTouch();

  if (touch.x > 0 && touch.x < 320 && touch.y > 0 && touch.y < 320) {
        return true;

    } else {
        return false;
    }

}
 
 
 void my_touchpad_read(lv_indev_t * indev_driver, lv_indev_data_t * data) {
    if (touched()) {
        Point touch = touchscreen.getTouch();
        int16_t x = touch.x;
        int16_t y = touch.y;

        // Apply coordinate swapping if DISPLAY_SWAP_XY is enabled
        if (false) {
            int16_t temp = x;
            x = y;
            y = temp;
        }

        // Apply mirroring based on DISPLAY_MIRROR_X and DISPLAY_MIRROR_Y
        if (true) {
            x = LV_HOR_RES - x;
        }
        if (DISPLAY_MIRROR_Y) {
            y = LV_VER_RES - y;
        }

        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PRESSED;

        Serial.print(F("Adjusted Touch at X: "));
        Serial.print(x);
        Serial.print(F(", Y: "));
        Serial.println(y);
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}
 
 
 void register_touch(lv_disp_t *disp) {
     Serial.println(F("Registering touch in lvgl."));
     indev = lv_indev_create();      
     lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);  
     lv_indev_set_read_cb(indev, my_touchpad_read);    
     Serial.println(F("Touch registered."));
 }


