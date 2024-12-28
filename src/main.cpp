#include <Arduino.h>
#include <XPT2046_Bitbang.h>
#include "GUI/ScreenManager.h"
#include "esp32_smartdisplay/src/esp32_smartdisplay.h"
#include "GUI/events.h"
#include "modules/RF/CC1101.h"
#include "modules/BLE/SourApple.h"
#include "modules/BLE/BLESpam.h"
#include "modules/ETC/SDcard.h"
#include <FFat.h>
#include "lv_fs_if.h"
#include "modules/dataProcessing/SubGHzParser.h"
#include "modules/ir/TV-B-Gone.h"
#include "modules/ir/WORLD_IR_CODES.h"
#include "modules/IR/ir.h"
#include "IRrecv.h"
#include <assert.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>
#include <Wire.h>
#include "modules/nfc/nfc.h"
#include "RF24.h"
#include "modules/RF/rf24.h"
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
#include <SPI.h>



IRrecv irrecv(IR_RX);   

decode_results lastResults;


//RF24Class RF24(RF24_CE, RF24_CS);

//SPIClass SPI(2);

SDcard& SD_CARD = SDcard::getInstance();

//SoftSpiDriver<SDCARD_MISO_PIN, SDCARD_MOSI_PIN, SDCARD_SCK_PIN> softSpiLCD;


XPT2046_Bitbang touchscreen(MOSI_PIN, MISO_PIN, CLK_PIN, CS_PIN);
ScreenManager& screenMgrM = ScreenManager::getInstance();

 // Touch handling variables
 static lv_indev_t *indev = nullptr;
 TouchCallback _singleTouchCallback;
 //SubGHzParser SubghzParser;


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
    
    esp_log_level_set("*", ESP_LOG_ERROR);        // set all components to ERROR level
    esp_log_level_set("wifi", ESP_LOG_WARN);      // enable WARN logs from WiFi stack
    esp_log_level_set("dhcpc", ESP_LOG_INFO);     // enable INFO logs from DHCP client


    // pinMode(CC1101_CS, PULLUP);
    // pinMode(CC1101_CS, OUTPUT);
    // pinMode(RFID_CS, PULLUP);
    // pinMode(RFID_CS, OUTPUT);
    // pinMode(RF24_CS, PULLUP);
    // pinMode(RF24_CS, OUTPUT);

  Serial.begin(115200);

    // digitalWrite(RFID_CS, HIGH);
    // digitalWrite(RF24_CS, HIGH);
    // digitalWrite(CC1101_CS, HIGH);

    init_touch([]() { Serial.println(F("Single touch detected!")); });
    smartdisplay_init();
    auto disp = lv_disp_get_default();

    

#ifdef CYDV2
        touchscreen.setCalibration(153, 123, 1915, 1824);
#endif
#ifdef CYDV3
    touchscreen.setCalibration(180, 197, 1807, 1848);
#endif



//touchscreen.calibrate();
 //touchscreen.setCalibration(185, 280, 3700, 3850);

    screenMgrM.createmainMenu();
    register_touch(disp);

// pinMode(21, OUTPUT);
//    digitalWrite(21, LOW);

  // Initialize CS pins
    pinMode(CC1101_CS, OUTPUT);
    pinMode(SDCARD_CS_PIN, OUTPUT);
    pinMode(RF24_CS, OUTPUT);
    digitalWrite(CC1101_CS, HIGH);
    digitalWrite(SDCARD_CS_PIN, HIGH);
    digitalWrite(RF24_CS, HIGH);

    // Initialize SPI
    SPI.begin(CYD_SCLK, CYD_MISO, CYD_MOSI);

    // SD Card Initialization
    digitalWrite(SDCARD_CS_PIN, LOW);  // Select SD card
  //  SD_CARD.forceOff = false;
    delay(10);
    if (!SD_CARD.initializeSD()) {
        Serial.println(F("Failed to initialize SD card!"));
    }
    lv_fs_if_init();
    delay(10);
   // SD_CARD.endSD();
  //  SD_CARD.forceOff = true;
    digitalWrite(SDCARD_CS_PIN, HIGH); // Deselect SD card
  //  SD.end(); // Finalize SD card

    delay(20);

    // CC1101 Initialization
    digitalWrite(CC1101_CS, LOW); // Select CC1101
    if (CC1101.init()) {
        Serial.println(F("CC1101 initialized."));
    } else {
        Serial.println(F("Failed to initialize CC1101."));
    }
    digitalWrite(CC1101_CS, HIGH); // Deselect CC1101
    delay(20);

    // IR receiver setup
    pinMode(IR_RX, INPUT_PULLUP);
        pinMode(IR_TX, OUTPUT);

    irrecv.enableIRIn();


 
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
       // Serial.println(digitalRead(CCGDO2A));
             if (CC1101.CheckReceived())
             {
                delay(10);
               CC1101.signalanalyse();
               CC1101.disableReceiver();
               delay(10);
               C1101CurrentState = STATE_IDLE;
             }
             delay(10);
     }

     if(C1101CurrentState == STATE_PLAYBACK) {
         CC1101.initrRaw();
         CC1101.sendRaw();
         CC1101.disableTransmit();
       C1101CurrentState = STATE_IDLE;
     };


    if(C1101CurrentState == STATE_DETECT)
    {
                                        lv_label_set_text(
                                                          screenMgrM.detectLabel,
                                                          (String("Frequency: ") + strongestASKFreqs[0] +
                                                           " MHz | RSSI: " + strongestASKRSSI[0] +
                                                           "\nFrequency: " + strongestASKFreqs[1] +
                                                           " MHz | RSSI: " + strongestASKRSSI[1] +
                                                           "\nFrequency: " + strongestASKFreqs[2] +
                                                           " MHz | RSSI: " + strongestASKRSSI[2])
                                                              .c_str());
                                                           

        // xTaskCreatePinnedToCore(
        // CC1101_CLASS::fskAnalyze,  // Function to run
        // "fskAnalyze",               // Task name
        // 8192,                              // Stack size in bytes
        // NULL,                              // Parameter for task
        // 1,                                 // Priority
        // NULL,                              // Task handle
        // 1                                  // Core 1
        // );

    }

     if(C1101CurrentState == STATE_SEND_FLIPPER) {        
        SubGHzParser parser;
        parser.loadFile(EVENTS::fullPath);
        SubGHzData data = parser.parseContent();
     }

     if(IRCurrentState == IR_STATE_PLAYBACK){
        sendReceived();
     }

    if(IRCurrentState == IR_STATE_BGONE){
  const uint8_t num_EUcodes = sizeof(EUpowerCodes) / sizeof(EUpowerCodes[0]);
  
  sendAllCodes(EUpowerCodes, num_EUcodes);

  IRCurrentState = IR_STATE_IDLE;
    }

    while(IRCurrentState == IR_STATE_LISTENING) {
        Serial.println(digitalRead(IR_RX));
       if (irrecv.decode(&results))
        {
        IRCurrentState = IR_STATE_IDLE;
        Serial.println(results.value, HEX);
        lv_textarea_set_text(screenMgrM.text_area_IR, "Received\n");
        lv_textarea_add_text(screenMgrM.text_area_IR, String(results.value, HEX).c_str());
        lastResults = results; 
        irrecv.resume();
        
    }
    }

    // if(RF24CurrentState == RF24_STATE_TEST){
    //      //testRF24();
    //      RF24CurrentState = RF24_STATE_IDLE;
    // };

    //  while(NFCCurrentState == NFC_READ){
    //   //   readLoop();
    //      NFCCurrentState = NFC_IDLE;
    //  }






    if(RFstate == WARM_UP){    
        auto const tedkom = millis();
        
        
    if (millis() - previousMillis >= 50) {
        RFstate = GENERAL; 

       //SDInit();
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

   // IRCurrentState = STATE_IDDLE;

   switch (currentState) {
        case IDLE:
            // Do nothing or handle idle tasks
            break;
        case BLUETOOTH_JAM:
            jammer.bluetooth_jam();
            break;
        case DRONE_JAM:
            jammer.drone_jam();
            break;
        case BLE_JAM:
            jammer.ble_jam();
            break;
        case WIFI_JAM:
            jammer.wifi_jam();
            break;
        case ZIGBEE_JAM:
            jammer.zigbee_jam();
            break;
        case MISC_JAM:
            jammer.misc_jam(10, 20); // Example channels
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


