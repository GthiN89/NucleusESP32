
#include "arduino.h"
#include "SdFat.h"
#include <Arduino.h>
#include "GUI/ScreenManager.h"
#include <esp32_smartdisplay.h>
#include "GUI/events.h"
#include "modules/RF/CC1101.h"
#include "modules/BLE/SourApple.h"
#include "modules/BLE/BLESpam.h"
#include "modules/ETC/SDcard.h"
#include <FFat.h>

#include "lv_fs_if.h"
#include "modules/dataProcessing/SubGHzParser.h"

SDcard& SD_CARD = SDcard::getInstance();

SoftSpiDriver<SDCARD_MISO_PIN, SDCARD_MOSI_PIN, SDCARD_SCK_PIN> softSpiLCD;


XPT2046_Bitbang touchscreen(MOSI_PIN, MISO_PIN, CLK_PIN, CS_PIN);

 // Touch handling variables
 static lv_indev_t *indev = nullptr;
 TouchCallback _singleTouchCallback;
 SubGHzParser SubghzParser;

 // Function prototypes
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
  Serial.begin(115200);
  // Wait for USB Serial
    gpio_set_pull_mode(GPIO_NUM_17, GPIO_PULLDOWN_ONLY);
    gpio_install_isr_service(0);

   // CC1101.init();

   
  

  init_touch([]() { Serial.println(F("Single touch detected!")); });
    smartdisplay_init();
    ScreenManager& screenMgr = ScreenManager::getInstance();
    



    auto disp = lv_disp_get_default();


    touchscreen.setCalibration(153, 123, 1915, 1824);

        screenMgr.createmainMenu();
    register_touch(disp);

    SD_CARD.initializeSD();


        lv_fs_if_init();


    
 
}



 ulong next_millis;
 auto lv_last_tick = millis();

auto previousMillis = millis();


void loop() {


     auto const now = millis();
   lv_tick_inc(now - lv_last_tick);
   lv_last_tick = now;
   lv_timer_handler();

  delay(1); 
 
     if(C1101CurrentState == STATE_ANALYZER) {
      //   CC1101.fskAnalyze();
             if (CC1101.CheckReceived())
             {
                delay(5);
               CC1101.signalanalyse();
               CC1101.disableReceiver();
               delay(10);
               C1101CurrentState = STATE_IDLE;
             }
             delay(1);
     }
      if(C1101CurrentState == STATE_RCSWITCH) 
 {
     RCSwitch mySwitch = CC1101.getRCSwitch();
     if (mySwitch.available())
     {
         ScreenManager& screenMgr = ScreenManager::getInstance();
         lv_obj_t* ta = screenMgr.getTextArea();      
         lv_textarea_set_text(ta, (String("New Signal Received. \nvalue: ") + String(mySwitch.getReceivedValue()) + String(" (") + String(mySwitch.getReceivedBitlength()) + String("bit)\n Protocol: ") + String(mySwitch.getReceivedProtocol())).c_str());
         C1101CurrentState = STATE_IDLE;
     }
 }

     if(C1101CurrentState == STATE_PLAYBACK) {
         CC1101.enableTransmit();
         CC1101.sendRaw();
         CC1101.disableTransmit();
       C1101CurrentState = STATE_IDLE;
     };
     if(BTCurrentState == STATE_SOUR_APPLE) {
         sourApple sa;
         sa.loop();
     } 
     if(BTCurrentState == STATE_BT_IDDLE) {
        //  BLESpam spam;
        //  spam.aj_adv(SpamDevice);
     }
     if(C1101CurrentState == STATE_SEND_FLIPPER) {        
        SubGHzParser parser;
        parser.loadFile(EVENTS::fullPath);
        SubGHzData data = parser.parseContent();
     }
    if(RFstate == WARM_UP){    
        auto const tedkom = millis();
        
        
    if (millis() - previousMillis >= 50) {
        RFstate = GENERAL; 

      //  SDInit();
        if (SD_CARD.fileExists("/warmpup1.sub")) {
            SD_CARD.read_sd_card_flipper_file("/warmpup1.sub");
            detachInterrupt(CC1101_CCGDO0A);
            CC1101.initrRaw();
            ELECHOUSE_cc1101.setCCMode(0); 
            ELECHOUSE_cc1101.setPktFormat(3);
            ELECHOUSE_cc1101.SetTx();
            pinMode(CC1101_CCGDO0A, OUTPUT);
            SubGHzParser parser;
            parser.loadFile("/warmpup1.sub");
            SubGHzData data = parser.parseContent();
        } else {
            Serial.println("File does not exist.");
        }
       //  SDInit();
        // if (SD_CARD.fileExists("/warmpup1.sub")) {
        //     SD_CARD.read_sd_card_flipper_file("/warmpup1.sub");
        //     detachInterrupt(CC1101_CCGDO0A);
        //     CC1101.initrRaw();
        //     ELECHOUSE_cc1101.setCCMode(0); 
        //     ELECHOUSE_cc1101.setPktFormat(3);
        //     ELECHOUSE_cc1101.SetTx();
        //     pinMode(CC1101_CCGDO0A, OUTPUT);
        //     SubGHzParser parser;
        //     parser.loadFile("/warmpup1.sub");
        //     SubGHzData data = parser.parseContent();
        // } else {
        //     Serial.println("File does not exist.");
        // }
    }
    }
    if(updatetransmitLabel){
        String text;
        if(!SD_CARD.FlipperFileFlag){
            text = "Transmitting\n Count: " + String(codesSend);
        } else {
            text = "Transmision complete";
            SD_CARD.FlipperFileFlag = false;
        }
        
        lv_label_set_text(label_sub, text.c_str());
    }

    
}

bool touched() {
Point touch = touchscreen.getTouch();

  if (touch.x > 0 && touch.x < TFT_WIDTH && touch.y > 0 && touch.y < TFT_HEIGHT) {
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
        if (DISPLAY_SWAP_XY) {
            int16_t temp = x;
            x = y;
            y = temp;
        }

        // Apply mirroring based on DISPLAY_MIRROR_X and DISPLAY_MIRROR_Y
        if (DISPLAY_MIRROR_X) {
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


