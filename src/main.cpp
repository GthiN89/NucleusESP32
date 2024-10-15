// File: src/main.cpp

#include <Arduino.h>
#include <esp32_smartdisplay.h>
#include <SPIFFS.h>
#include <lvgl.h>
#include "globals.h"
#include "GUI/ScreenManager.h"
#include "GUI/screens/replay.h"
#include "GUI/screens/c1101Scan.h"
#include "modules/RF/CC1101.h"
#include "modules/ETC/SDcard.h"
#include "XPT2046_Bitbang.h"

#include "GUI/events.h"
#include "GUI/menus/RFMenu.h"
#include "modules/RF/CC1101.h"
#include "FS.h"
#include <LittleFS.h>
#include "SPIFFS.h"

#define FORMAT_LITTLEFS_IF_FAILED true


// Pin definitions for touchscreen and SD card
#define MOSI_PIN 32
#define MISO_PIN 39
#define CLK_PIN  25
#define CS_PIN   33
#define SD_CS 5 // SD card CS pin

// Touchscreen object
XPT2046_Bitbang touchscreen(MOSI_PIN, MISO_PIN, CLK_PIN, CS_PIN);

// Touch handling variables
static lv_indev_drv_t touchDriver;
static uint32_t firstTouchTime = 0;
static bool doubleTouch = false;
static int touchCounter = 0;
static int touched = 0;
TouchCallback _singleTouchCallback;
TouchCallback _doubleTouchCallback;

// Function prototypes
void on_touch_change(lv_indev_drv_t *touchDriver, lv_indev_data_t *touchData);
void calibrate();
void init_touch(TouchCallback singleTouchCallback, TouchCallback doubleTouchCallback);
void register_touch(lv_disp_t *disp);
void my_touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data);
static lv_obj_t *touch_marker = nullptr;

// Event handler declaration
void radioHandlerOnChange();

// Function prototypes for RF handling
void ProtAnalyzerloop();



const uint32_t subghz_frequency_list[] = {
    300000000, 303875000, 304250000, 310000000, 315000000, 318000000,  //  300-348 MHz
    390000000, 418000000, 433075000, 433420000, 433920000, 434420000, 434775000, 438900000,  //  387-464 MHz
    868350000, 868000000, 915000000, 925000000  //  779-928 MHz
};

CC1101_CLASS CC1101;


void setup() {
    // Inicializace sériové komunikace
        //esp_debug_initialize();  // Initialize the GDB stub
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println(F("Starting setup..."));

    // Inicializace LVGL a displeje
    smartdisplay_init(); // Initialize the display and LVGL
    Serial.println(F("LVGL initialized."));

     if (!SPIFFS.begin(true, "/littlefs")) {
         Serial.println("SPIFFS Mount Failed");
         return;
     }
  File root = SPIFFS.open("/");
 
  File file = root.openNextFile();
 
  while(file){
 
      Serial.print("FILE: ");
      Serial.println(file.name());
 
      file = root.openNextFile();
  }


  

    // Inicializace touch
    init_touch([]() { Serial.println(F("Single touch detected!")); }, []() { Serial.println(F("Double touch detected!")); });

    // Register touch input device with LVGL
    auto disp = lv_disp_get_default();
    register_touch(disp);

      Serial.print("Initializing CC1101...");
  if (CC1101.init())
  {
    Serial.print("CC1101 initialized.");
    CC1101_init = true;
  }

    MainMenuScreen MainMenuScreen;
    MainMenuScreen.initialize(); // Load main menu

//       Serial.print("Initializing CC1101...");

//   if (CC1101.init())
//   {
//     Serial.print("CC1101 initialized.");
//     CC1101_is_initialized = true;
//   }
//   else
//   {
//     Serial.print("CC1101 not initialized.");
//   }
//     // Inicializace RF Modules
   
  
//     // Připojení Interrupt Handler
//     attachInterrupt(digitalPinToInterrupt(CCGDO0A), radioHandlerOnChange, CHANGE);
//     CC1101_interup_attached = true;

}

void loop() {
    lv_task_handler(); // Handle LVGL tasks
    delay(5); // Minimal delay to allow other tasks to run


    if(C1101CurrentState == STATE_CAPTURE) {
        int pinState1 = digitalRead(CC1101_CCGDO0A);
            Serial.print("CC1101 GDO Pin (CC1101_CCGDO2A) state: ");
            Serial.println(pinState1 == HIGH ? "HIGH" : "LOW");
            delay(10);
            CC1101.captureLoop();
            C1101CurrentState = STATE_IDLE;
    }

    if(C1101CurrentState == STATE_PULSE_SCAN) {
        if (CC1101.CheckReceived())
            {
              CC1101.getPulseLenghtLoop();
              CC1101.disableReceiver();
              delay(10);
              C1101CurrentState = STATE_IDLE;
            }
            delay(1);
    }

    if(C1101CurrentState == STATE_ANALYZER) {
            if (CC1101.CheckReceived())
            {
              CC1101.getPulseLenghtLoop();
              CC1101.disableReceiver();
              delay(10);
              C1101CurrentState = STATE_IDLE;
            }
            delay(1);

    }


    if(C1101CurrentState == STATE_PLAYBACK) {
                int pinState1 = digitalRead(CC1101_CCGDO2A);
            Serial.print("CC1101 GDO Pin (CC1101_CCGDO2A) state: ");
            Serial.println(pinState1 == HIGH ? "HIGH" : "LOW");
      //  CC1101.enableTransmit();
        CC1101.sendCapture();
      //  CC1101.disableTransmit();
      C1101CurrentState = STATE_IDLE;
    };

  if (C1101CurrentState == STATE_SEND_FLIPPER)
  {
        if (tempSampleCount % 2 == 0) {
    Serial.println("Číslo je sudé.");
    } else {
    tempSampleCount++;
    }
    Serial.print(disconnectSD());
    int samplesClean[tempSampleCount];
    Serial.print(String("Send RAW Data, sample count: " + String(tempSampleCount) + String(" | Frequency: ") + String(tempFreq)).c_str());

    for (int i = 0; i < tempSampleCount; i++) {
    samplesClean[i] = 100;
    }

        for (int i = 0; i < tempSampleCount; i++) {        
        if (tempSample[i]>0)
        {
            Serial.print(String(tempSample[i]).c_str());
            samplesClean[i] = tempSample[i];
            Serial.print(", ");
        } else {            
            if(tempSample[i] * -1 > 0) {
            Serial.print(String(tempSample[i] * -1).c_str());
            samplesClean[i] = tempSample[i] * -1;
            }
            Serial.print(", ");
        }
    }

        for (int i = 0; i < tempSampleCount; i++) {        
        Serial.print(String(samplesClean[i]).c_str());
            Serial.print(", ");
        }


//    Serial.println(); // Print a newline at the end
    CC1101_FREQ = tempFreq;
//    CC1101.sendSamples(sample,tempSampleCount);
   // CC1101.enableTransmit();
   // CC1101.sendSamples(tempSample, tempSampleCount);
    CC1101.sendSamples(samplesClean, tempSampleCount);



    //lv_label_set_text(ui_lblPresetsStatus, String("Sending Flipper Complete ! \n\nSample: " + String(tempSampleCount) + String(" | Freq: ") + String(tempFreq) + String(" mHz")).c_str());
    C1101CurrentState = STATE_IDLE;

  }
}




// Touch input reading function
void my_touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data) {
    on_touch_change(indev_driver, data);
}

// Touch event handler with single and double touch detection
void on_touch_change(lv_indev_drv_t *touchDriver, lv_indev_data_t *touchData) {
    Point touchPoint = touchscreen.getTouch();

    if (touchPoint.x >= 0 && touchPoint.x < TFT_WIDTH && touchPoint.y >= 0 && touchPoint.y < TFT_HEIGHT) {
        touched = 1;
        touchData->state = LV_INDEV_STATE_PRESSED;
        touchData->point.x = touchPoint.x;
        touchData->point.y = touchPoint.y;

        // Move the touch marker to the corrected location and show it
        lv_obj_set_pos(touch_marker, touchPoint.x - 5, touchPoint.y - 5); // Center the circle at the touch point
        lv_obj_clear_flag(touch_marker, LV_OBJ_FLAG_HIDDEN); // Show the marker
    } else {
        touchData->state = LV_INDEV_STATE_RELEASED;
        if (touched == 1) {
            uint32_t capturedTouchReleaseTime = millis();

            if (touchCounter == 1) {
                if (capturedTouchReleaseTime - firstTouchTime <= 300) {
                    doubleTouch = true;
                } else {
                    touchCounter--;
                }
            }

            touchCounter++;

            if (touchCounter == 1) {
                firstTouchTime = capturedTouchReleaseTime;
            }

            touched = 0;
            lv_obj_add_flag(touch_marker, LV_OBJ_FLAG_HIDDEN); // Hide the marker when touch is released
        }
    }

    if (touchCounter == 1 && millis() - firstTouchTime > 300) {
        _singleTouchCallback();
        touchCounter = 0;
    }

    if (doubleTouch) {
        _doubleTouchCallback();
        doubleTouch = false;
        touchCounter = 0;
    }
}

// Perform touch calibration if needed
void calibrate() {
    Serial.println(F("Calibrating touch."));
    delay(2000);
    touchscreen.calibrate();
    touchscreen.saveCalibration();
    Serial.println(F("Touch calibrated."));
}

// Initialize the touch system
void init_touch(TouchCallback singleTouchCallback, TouchCallback doubleTouchCallback) {
    Serial.println(F("Initializing touch."));
    touchscreen.begin();

    touchscreen.setCalibration(109, 150, 1936, 1912);  // xMin, xMax, yMin, yMax
    if (!touchscreen.loadCalibration()) {
        calibrate();
    }

    // Create a small circle that will represent the touch point
    lv_obj_t * scr = lv_scr_act(); // Get the current screen
    if (touch_marker == nullptr) {  // Make sure touch_marker is not already created
        touch_marker = lv_obj_create(scr);
        lv_obj_set_size(touch_marker, 10, 10); // Set the size of the circle
        lv_obj_set_style_radius(touch_marker, LV_RADIUS_CIRCLE, 0); // Make it a circle
        lv_obj_set_style_bg_color(touch_marker, lv_color_hex(0xFF0000), 0); // Set color to red
        lv_obj_add_flag(touch_marker, LV_OBJ_FLAG_HIDDEN); // Hide it initially
    }

    _singleTouchCallback = singleTouchCallback;
    _doubleTouchCallback = doubleTouchCallback;
    Serial.println(F("Touch initialized."));
}

// Register the touch input device with LVGL
void register_touch(lv_disp_t *disp) {
    Serial.println(F("Registering touch in lvgl."));
    lv_indev_drv_init(&touchDriver);
    touchDriver.disp = disp;
    touchDriver.type = LV_INDEV_TYPE_POINTER;
    touchDriver.read_cb = my_touchpad_read;
    lv_indev_drv_register(&touchDriver);
    Serial.println(F("Touch registered."));
}
