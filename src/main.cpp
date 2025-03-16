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
#include "modules/RF/brute.h"


#include <Wire.h>
#include <SPI.h>

#include "GUI/logo.h"

#include <IRrecv.h>
#include <IRutils.h>
#include "modules/IR/ir.h"

#include "modules/nfc/nfc.h"




decode_results results;
IRsend Irsend(IR_TX);
IRrecv Irrecv(IR_RX);
IR_CLASS ir;
RCSwitch mySwitch1;

BRUTE::CC1101_BRUTE RFbruteForcer;
CC1101_CLASS CC1101;

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

void setup() {

  Serial.begin(115200);
  init_touch([]() { });
  smartdisplay_init();
  auto disp = lv_disp_get_default();

  #ifdef CYDV2
    touchscreen.setCalibration(153, 123, 1915, 1824);
  #endif
  #ifdef CYDV3
    touchscreen.setCalibration(180, 197, 1807, 1848);
  #endif

 // touchscreen.calibrate();

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
    digitalWrite(PN532_SS ,HIGH);
    digitalWrite(CC1101_CS, LOW);
    delay(5);

    if (CC1101.init()) {
        Serial.println(F("CC1101 initialized."));
        CC1101.emptyReceive();
    } else {
        Serial.println(F("Failed to initialize CC1101."));
    }
   
    digitalWrite(CC1101_CS, HIGH);  
    SPI.end();                     
    delay(10);                      

    // digitalWrite(PN532_SS, LOW);   
    // SPI.begin(CYD_SCLK, CYD_MISO, CYD_MOSI); 
    // delay(10);                      

    // if (NFCPN.init()) {
    //     Serial.println("PN532 initialized.");
    // } else {
    //     Serial.println("PN532 initialization failed!");
    // }

    // digitalWrite(PN532_SS, HIGH);   // Deselect PN532 explicitly after initialization

    // SPI.end(); 

    pinMode(IR_RX, INPUT_PULLUP);

    pinMode(26, OUTPUT);
    pinMode(21, OUTPUT);
    digitalWrite(21, HIGH);
      

}

void bruteForceTask(void *pvParameters) {


        if (BruteCurrentState == CAME_12bit) {
            Serial.println("came");
            if (RFbruteForcer.Came12BitBrute()) {
                C1101CurrentState = STATE_IDLE;
            }
            Serial.println(RFbruteForcer.counter);
            Serial.println(F("CAME codes sent"));
        }

        if (BruteCurrentState == NICE_12bit) {
            Serial.println("nice");
            if (RFbruteForcer.Nice12BitBrute()) {
                C1101CurrentState = STATE_IDLE;
            }
            Serial.println(RFbruteForcer.counter);
            Serial.println(F("NICE codes sent"));
        }

        if (BruteCurrentState == ANSONIC_12bit) {
            if (RFbruteForcer.Ansonic12BitBrute()) {
                C1101CurrentState = STATE_IDLE;
            }
            Serial.println(RFbruteForcer.counter);
        }

        if (BruteCurrentState == Holtek_12bit) {
            if (RFbruteForcer.Holtek12BitBrute()) {
                C1101CurrentState = STATE_IDLE;
            }
            Serial.println(RFbruteForcer.counter);
        }
        if (BruteCurrentState == Chamberlain_9bit) {
            if (RFbruteForcer.Chamberlain9BitBrute()) {
                C1101CurrentState = STATE_IDLE;
            }
            Serial.println(RFbruteForcer.counter);
        }
        if (BruteCurrentState == Chamberlain_9bit) {
            if (RFbruteForcer.Chamberlain9BitBrute()) {
                C1101CurrentState = STATE_IDLE;
            }
        }
        if (BruteCurrentState == Chamberlain_8bit) {
            if (RFbruteForcer.Chamberlain8BitBrute()) {
                C1101CurrentState = STATE_IDLE;
            }
        }
        if (BruteCurrentState == Chamberlain_7bit) {
            if (RFbruteForcer.Chamberlain7BitBrute()) {
                C1101CurrentState = STATE_IDLE;
            }
        }
        if (BruteCurrentState == Linear_10bit) {
            if (RFbruteForcer.Linear10BitBrute()) {
                C1101CurrentState = STATE_IDLE;
            }
        }

        if (BruteCurrentState == DeBrujin) {
            if (RFbruteForcer.debrujin()) {
                C1101CurrentState = STATE_IDLE;
            }
        }

        
      //  vTaskDelay(10 / portTICK_PERIOD_MS); // Prevent watchdog resets
    
}

 void CC1101Loop() {
    if(C1101CurrentState == STATE_CODEGRABBER) {
               // delay(50);
                Serial.println(gpio_get_level(CC1101_CCGDO0A));
        if (CC1101.CheckReceived()) {
             delay(5);
            CC1101.disableReceiver();
            delay(5);
            CC1101.handleSignal();
            CC1101.decode();

            C1101CurrentState = STATE_IDLE;
            runningModule = MODULE_NONE;
        }
    }

    if(C1101CurrentState == STATE_RAWREC) {
        Serial.println(gpio_get_level(CC1101_CCGDO0A));
        if (CC1101.CheckReceived()) {
            delay(5);
           CC1101.disableReceiver();
           delay(5);
           CC1101.handleSignal();


           C1101CurrentState = STATE_IDLE;
           runningModule = MODULE_NONE;
       }
    }

    if(C1101CurrentState == STATE_RCSWITCH) {
               // delay(50);
                Serial.println(gpio_get_level(CC1101_CCGDO0A));
        if (mySwitch1.available()) {
             delay(5);
            ir.output(mySwitch1.getReceivedValue(), mySwitch1.getReceivedBitlength(), mySwitch1.getReceivedDelay(), mySwitch1.getReceivedRawdata(),mySwitch1.getReceivedProtocol(), screenMgrM.getTextArea());
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
        // lv_label_set_text(screenMgrM.detectLabel, 
        // (String("Frequencies:\n") +
        // "Frequency: " + strongestASKFreqs[0] + " MHz | RSSI: " + strongestASKRSSI[0] + "\n" +
        // "Frequency: " + strongestASKFreqs[1] + " MHz | RSSI: " + strongestASKRSSI[1] + "\n" +
        // "Frequency: " + strongestASKFreqs[2] + " MHz | RSSI: " + strongestASKRSSI[2] + "\n" +
        // "Frequency: " + strongestASKFreqs[3] + " MHz | RSSI: " + strongestASKRSSI[3] + "\n\n")
        // .c_str());
    }

    // if(C1101CurrentState == STATE_SEND_FLIPPER) {
    //     SubGHzParser parser;
    //     parser.loadFile(EVENTS::fullPath);
    //   //  SubGHzData data = parser.parseContent();
    // }

    if(C1101CurrentState == STATE_BRUTE) {
            C1101CurrentState = STATE_IDLE;

    xTaskCreatePinnedToCore(
    bruteForceTask,  // Task function
    "BruteForceTask",
    8192,  // Stack size
    NULL,  // Parameters
    10,  // Highest user priority (22)
    NULL,
    1  // Run on Core 1
);


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
        ir.TVbGONE();
    }

    if(IRCurrentState == IR_STATE_LISTENING) {
        
        if (Irrecv.decode(&results)) {
            IRCurrentState = IR_STATE_IDLE;
            runningModule = MODULE_NONE;
            Serial.print(resultToHumanReadableBasic(&results));
            lv_textarea_set_text(screenMgrM.text_area_IR, "Received\n");
            lv_textarea_add_text(screenMgrM.text_area_IR, String(resultToHumanReadableBasic(&results)).c_str());
            Irrecv.resume();
        }
    }


}
 
  ulong next_millis;
  auto lv_last_tick = esp_timer_get_time() / 1000; // Convert to milliseconds
 
 auto previousMillis = esp_timer_get_time() / 1000;
 
 void loop() {
     auto const now = esp_timer_get_time() / 1000;
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
    if(RFbruteForcer.sendingFlag) {
         String text = String(RFbruteForcer.counter) + "/4096";
         lv_label_set_text(screenMgrM.getTextAreaRCSwitchMethod(), text.c_str());
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


