
#include <Arduino.h>
#include "GUI/ScreenManager.h"
#include "XPT2046_Bitbang.h"
#include <esp32_smartdisplay.h>
#include "GUI/events.h"
#include "modules/RF/CC1101.h"
#include "modules/BLE/SourApple.h"
#include "modules/BLE/BLESpam.h"
#include "modules/ETC/SDcard.h"
#include "globals.h"
#include <nvs.h>
#include <nvs_flash.h>


// Pin definitions for touchscreen and SD card
#define MOSI_PIN 32
#define MISO_PIN 39
#define CLK_PIN  25
#define CS_PIN   33
#define SD_CS 5 // SD card CS pin

 XPT2046_Bitbang touchscreen(MOSI_PIN, MISO_PIN, CLK_PIN, CS_PIN);

 // Touch handling variables
 static lv_indev_t *indev = nullptr;
 TouchCallback _singleTouchCallback;


#define REBOOT_THRESHOLD 3   // Number of reboots to trigger calibration

void check_and_calibrate_touchscreen() {
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // Open NVS handle
    nvs_handle_t nvs_handle;
    err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    ESP_ERROR_CHECK(err);

    // Read the reboot counter
    int32_t reboot_counter = 0;
    err = nvs_get_i32(nvs_handle, "reboot_count", &reboot_counter);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        reboot_counter = 0;  // Initialize if not found
    } else {
        ESP_ERROR_CHECK(err);
    }

    reboot_counter++;
    err = nvs_set_i32(nvs_handle, "reboot_count", reboot_counter);
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(nvs_commit(nvs_handle));

    Serial.print("Reboot count: ");
    Serial.println(reboot_counter);

    if (reboot_counter >= REBOOT_THRESHOLD) {
        Serial.println("Repeated reboot detected. Running calibration...");
        
        // Run calibration
        touchscreen.calibrate();

        // Reset the reboot counter after calibration
        reboot_counter = 0;
        err = nvs_set_i32(nvs_handle, "reboot_count", reboot_counter);
        ESP_ERROR_CHECK(err);
        ESP_ERROR_CHECK(nvs_commit(nvs_handle));
    }

    // Close NVS handle
    nvs_close(nvs_handle);
}

 // Function prototypes
void register_touch(lv_disp_t *disp);
void my_touchpad_read(lv_indev_t * indev_driver, lv_indev_data_t * data);
void init_touch(TouchCallback singleTouchCallback);

void init_touch(TouchCallback singleTouchCallback) {
    Serial.println(F("Initializing touch."));
    touchscreen.begin(); 

    if (!touchscreen.loadCalibration()) {
        touchscreen.calibrate();    }


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
    Serial.setDebugOutput(true);
  
    init_touch([]() { Serial.println(F("Single touch detected!")); });
    check_and_calibrate_touchscreen();
    smartdisplay_init();
    ScreenManager& screenMgr = ScreenManager::getInstance();
    screenMgr.createmainMenu();

    auto disp = lv_disp_get_default();

    register_touch(disp);

    SDInit();  
      Serial.begin(115200);


  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
    return;
  }
}

 ulong next_millis;
 auto lv_last_tick = millis();

void loop()
{
   auto const now = millis();
   lv_tick_inc(now - lv_last_tick);
   lv_last_tick = now;
   lv_timer_handler();

  delay(100); 
 
     if(C1101CurrentState == STATE_ANALYZER) {
             if (CC1101.CheckReceived())
             {
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
     if(C1101CurrentState == STATE_BRUTE) {
         CC1101.sendBrute(1);
             ScreenManager& screenMgr = ScreenManager::getInstance();
             lv_obj_t* ta = screenMgr.getTextAreaBrute();
             lv_textarea_set_text(ta, ("Brute forcing\n attempt number: " + String(bruteCounter)).c_str());            
         CC1101.disableTransmit();
       C1101CurrentState = STATE_IDLE;
     };
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
         BLESpam spam;
         spam.aj_adv(SpamDevice);
     }
}

bool touched() {
Point touch = touchscreen.getTouch();

  if (touch.x >= 0 && touch.x < TFT_WIDTH && touch.y >= 0 && touch.y < TFT_HEIGHT) {
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

        Serial.print("Adjusted Touch at X: ");
        Serial.print(x);
        Serial.print(", Y: ");
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
