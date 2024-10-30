#include "screens.h"
//#include "modules/RF/subGhz.h"
#include <lvgl.h>
#include <iostream>
#include <cstring>
#include "playZero.h"
#include "sendTesla.h"
#include "../events.h"
#include "globals.h"

lv_obj_t* tesla_screen = nullptr;
lv_obj_t* teslaCont = nullptr;
int teslaTextLine = 0; // Initialize the teslaTextLine
lv_obj_t** labels = nullptr; // Initialize labels to nullptr

lv_obj_t* testLabel = nullptr;

void teslaScreen();
void createTeslaButtons(lv_obj_t* teslaCont);
void addLineToTeslaContainer(lv_obj_t* teslaCont, String text);

void teslaScreen() {
    Serial.println("Initializing playTeslaScreen...");  
    
    tesla_screen = lv_obj_create(NULL); 
    lv_scr_load(tesla_screen);  
    lv_obj_set_size(tesla_screen, 240, 320);
    lv_obj_set_flex_flow(tesla_screen, LV_FLEX_FLOW_COLUMN); 
    lv_obj_set_flex_align(tesla_screen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER); 
    

    Serial.println("Created playTeslaScreen");
    createTeslaScreenContainer(tesla_screen);
}

void createTeslaScreenContainer(lv_obj_t* parent_screen) {
    teslaCont = lv_obj_create(parent_screen);
    lv_obj_set_size(teslaCont, 215, 240);
    lv_obj_set_flex_flow(teslaCont, LV_FLEX_FLOW_COLUMN);

    testLabel = lv_label_create(teslaCont); 
    lv_label_set_text_fmt(testLabel, "%s", "Tesla charger port opener \n Press send to open");
    lv_obj_set_style_align(testLabel, LV_ALIGN_LEFT_MID, 0);

 //   addLineToTeslaContainer(teslaCont, "second text");
    createTeslaButtons(teslaCont);   
}

void addLineToTeslaContainer(lv_obj_t* teslaCont, String text) {
    teslaTextLine++; // Increment the line counter
    labels = (lv_obj_t**)realloc(labels, teslaTextLine * sizeof(lv_obj_t*)); // Reallocate memory for labels array

    if (labels == nullptr) {
        Serial.println("Error: Memory allocation failed for labels.");
        return;
    }

    labels[teslaTextLine - 1] = lv_label_create(teslaCont); 
    if (labels[teslaTextLine - 1] == nullptr) {
        Serial.println("Error: Label creation failed.");
        return;
    }

    lv_label_set_text_fmt(labels[teslaTextLine - 1], "%s", text.c_str());
    lv_obj_set_style_align(labels[teslaTextLine - 1], LV_ALIGN_LEFT_MID, 0);
}

void createTeslaButtons(lv_obj_t* teslaCont) {
    lv_obj_t* tesla_button_container = lv_obj_create(lv_scr_act());  // Keep buttons on screen, not in teslaCont
    lv_obj_set_size(tesla_button_container, 215, 30); 
    lv_obj_set_flex_flow(tesla_button_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(tesla_button_container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER); 
    lv_obj_clear_flag(tesla_button_container, LV_OBJ_FLAG_SCROLLABLE);
    

    lv_obj_t* load_btn = lv_btn_create(tesla_button_container);
    lv_obj_add_event_cb(load_btn, play_tesla_btn_event_cb, LV_EVENT_CLICKED, teslaCont);  // Pass teslaCont as user data
    lv_obj_t* label = lv_label_create(load_btn);
    lv_label_set_text(label, "Send code");

    lv_obj_t* back_btn = lv_btn_create(tesla_button_container);
    lv_obj_add_event_cb(back_btn, EVENTS::btn_event_subGhzTools, LV_EVENT_CLICKED, NULL);
    lv_obj_t* back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, LV_SYMBOL_LEFT "Exit");
}

void sendByte(uint8_t dataByte) {
    for (int8_t bit = 7; bit >= 0; bit--) { // MSB
        digitalWrite(CCGDO0A, (dataByte & (1 << bit)) != 0 ? HIGH : LOW);
        delayMicroseconds(pulseWidth);
    }
}

void sendBits(int* buff, int length, int gdo0) {
    for (int i = 0; i < length; i++) {
        digitalWrite(gdo0, buff[i] < 0 ? LOW : HIGH);
        delayMicroseconds(abs(buff[i]));
    }
    digitalWrite(gdo0, LOW);
}

bool sendTeslaSignal(lv_obj_t* teslaCont, float freq) {
    ELECHOUSE_cc1101.setSpiPin(CC1101_SCLK, CC1101_MISO, CC1101_MOSI, CC1101_CS);
    ELECHOUSE_cc1101.Init();
    Serial.println("init");

    // Ensure teslaCont is valid
    if (teslaCont == nullptr) {
        Serial.println("Error: teslaCont is a null pointer.");
        return false;
    }

    

    ELECHOUSE_cc1101.setGDO0(CCGDO0A);
    ELECHOUSE_cc1101.setMHZ(freq);
    Serial.println("freque");

    ELECHOUSE_cc1101.SetTx();
    pinMode(CCGDO0A, OUTPUT); 
    ELECHOUSE_cc1101.setModulation(2);
    ELECHOUSE_cc1101.setDeviation(0);
    
    for (uint8_t t = 0; t < transmtesla; t++) {
        for (uint8_t i = 0; i < messageLength; i++) {
            sendByte(sequence[i]); 
            Serial.println("bite");
        }
        digitalWrite(CCGDO0A, LOW);
        delay(messageDistance);
    }
    ELECHOUSE_cc1101.setSidle();
    Serial.println("idle");
    
    addLineToTeslaContainer(teslaCont, "Code has been send");
    return true;
}

void play_tesla_btn_event_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t* teslaCon = (lv_obj_t*)lv_event_get_user_data(e);
        if (teslaCon != nullptr && sendTeslaSignal(teslaCon, 433.92)) {
            teslaSucessFlag = true;
        }
    }
}
