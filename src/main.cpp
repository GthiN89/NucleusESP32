#include <Arduino.h>
#include "main.h"
#include "modules/RF/CC1101.h"
IRrecv irrecv(IR_RX);   

decode_results lastResults;

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


void setup() {
CC1101_CLASS CC1101;
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
    } else {
        Serial.println(F("Failed to initialize CC1101."));
    }

    pinMode(IR_RX, INPUT_PULLUP);
    irrecv.enableIRIn();
    pinMode(IR_TX, OUTPUT);

}
 
 void CC1101Loop() {
    CC1101_CLASS CC1101;
    if(C1101CurrentState == STATE_ANALYZER) {
                    delay(50);
        if (CC1101.CheckReceived()) {
            delay(50);
            CC1101.signalanalyse();
            CC1101.disableReceiver();
            delay(50);
            C1101CurrentState = STATE_IDLE;
            runningModule = MODULE_NONE;
        }
    }
    if(C1101CurrentState == STATE_PLAYBACK) {
        CC1101.initrRaw();
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
        runningModule = MODULE_NONE;
    }  

}
 
 void IRLoop() {
    switch (IRCurrentState)
    {
    case IR_STATE_BGONE:
        {
            const uint8_t num_EUcodes = sizeof(EUpowerCodes) / sizeof(EUpowerCodes[0]);        
            sendAllCodes(EUpowerCodes, num_EUcodes);
            IRCurrentState = IR_STATE_IDLE;
            runningModule = MODULE_NONE;
        }
        break;
    
    case IR_STATE_LISTENING:
        if (irrecv.decode(&results)) {
            IRCurrentState = IR_STATE_IDLE;
            runningModule = MODULE_NONE;
            Serial.println(results.value, HEX);
            lv_textarea_set_text(screenMgrM.text_area_IR, "Received\n");
            lv_textarea_add_text(screenMgrM.text_area_IR, String(results.value, HEX).c_str());
            lastResults = results; 
            irrecv.resume();
        }
        break;
    
    default:
        break;
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


