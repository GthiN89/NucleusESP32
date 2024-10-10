// File: src/events/events.cpp

#include "events.h"
#include "GUI/ScreenManager.h"
#include "modules/ETC/SDcard.h"
#include <RCSwitch.h>
#include <Arduino.h>
#include "GUI/constants.h"
#include "GUI/menus/RFMenu.h"
#include "modules/RF/CC1101.h"
#include "GUI/screens/replay.h"
#include "GUI/screens/playZero.h"
#include "globals.h"
#include "GUI/screens/replayScreen.h"
#include "GUI/menus/RFMenu.h"
#include <cstring> 
#include <iostream>
#include <unordered_map>
#include "ELECHOUSE_CC1101_SRC_DRV.h"
#include "main.h"
#include "screens/replay.h"
using namespace std;



ScreenManager& screenMgr = ScreenManager::getInstance();
lv_obj_t* ta = screenMgr.getFreqInput();
lv_obj_t* text_area = screenMgr.getTextArea();


ReplayScreen ReplayScr;

char EVENTS::frequency_buffer[10]; char EVENTS::selected_str[32];  

void EVENTS::btn_event_playZero_run(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        playZeroScreen();
    }
}


void EVENTS::btn_event_Replay_run(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
          
          ReplayScr.initialize();
    }
}

void EVENTS::btn_event_protoAnalyse_run(lv_event_t* e) {
    screenMgr.protoAnalysScreen();
}

void EVENTS::btn_event_teslaCharger_run(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        teslaScreen();
        // Uncomment and use if needed:
        // if (sendTeslaSignal(433.92)) {
        //     teslaSucessFlag = true;
        // }
        // if (sendTeslaSignal(315.00)) {
        //     teslaSucessFlag = true;
        // }
    }
}


void EVENTS::ta_freq_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * kb = static_cast<lv_obj_t *>(lv_event_get_user_data(e));
    ScreenManager& screenMgr = ScreenManager::getInstance();
    lv_obj_t * text_area = screenMgr.getTextArea();
    lv_obj_t * ta = screenMgr.getFreqInput();
    lv_obj_t * presetDropdown = screenMgr.getPresetDropdown();

    if (ta && kb && text_area) {
        if (code == LV_EVENT_FOCUSED) {
            lv_keyboard_set_textarea(kb, ta);
            lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN); 
            Serial.println("Numeric Keyboard shown for Frequency");
            lv_obj_add_flag(text_area, LV_OBJ_FLAG_HIDDEN); 
            lv_obj_add_flag(presetDropdown, LV_OBJ_FLAG_HIDDEN); 
        } else if (code == LV_EVENT_DEFOCUSED || code == LV_EVENT_READY) {
            lv_obj_clear_flag(text_area, LV_OBJ_FLAG_HIDDEN); 
            lv_keyboard_set_textarea(kb, NULL);
            lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(presetDropdown, LV_OBJ_FLAG_HIDDEN); 
            lv_obj_clear_state(text_area, LV_STATE_FOCUSED);
  

             strncpy(EVENTS::frequency_buffer, lv_textarea_get_text(ta), sizeof(EVENTS::frequency_buffer) - 1);
             EVENTS::frequency_buffer[sizeof(EVENTS::frequency_buffer) - 1] = '\0'; 

             Serial.print("Frequency set to: ");
             Serial.println(EVENTS::frequency_buffer);
             lv_textarea_add_text(text_area, "Frequency set to: ");
             lv_textarea_add_text(text_area, EVENTS::frequency_buffer);
             lv_textarea_add_text(text_area, "\n");
             CC1101_MHZ = atoi(EVENTS::frequency_buffer);

        }
    } else {
        Serial.println("Error: Frequency input, keyboard, or text_area is NULL");
    }
}

void EVENTS::ta_filename_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * kb = static_cast<lv_obj_t *>(lv_event_get_user_data(e));


    ScreenManager& screenMgr = ScreenManager::getInstance();
    lv_obj_t* ta = screenMgr.getFilenameInput();
    lv_obj_t* text_area = screenMgr.getTextArea();

    if (ta && kb && text_area) {
        if (code == LV_EVENT_FOCUSED) {
            lv_keyboard_set_textarea(kb, ta);
            lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);  
            Serial.println("QWERTY Keyboard shown for Filename");
            lv_obj_add_flag(text_area, LV_OBJ_FLAG_HIDDEN); 
        } else if (code == LV_EVENT_DEFOCUSED || code == LV_EVENT_READY) {
            lv_obj_clear_flag(text_area, LV_OBJ_FLAG_HIDDEN);  
            lv_keyboard_set_textarea(kb, NULL);
            lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);  
        }
    } else {
        Serial.println("Error: Filename input, keyboard, or text_area is NULL");
    }
}


void EVENTS::kb_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        Serial.println("Keyboard hidden by confirm/cancel");
    }
}

void EVENTS::kb_qwert_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        Serial.println("QWERTY Keyboard hidden by confirm/cancel");
    }
}



void EVENTS::ta_pulse_event_cb(lv_event_t * e) {
    char frequency_buffer[10];
    ScreenManager& screenMgr = ScreenManager::getInstance();
    lv_obj_t * text_area = screenMgr.getTextArea();
    lv_obj_t * ta = screenMgr.getFreqInput();
    strncpy(frequency_buffer, lv_textarea_get_text(ta), sizeof(frequency_buffer) - 1);
    frequency_buffer[sizeof(frequency_buffer) - 1] = '\0'; 
    C1101CurrentState = STATE_PULSE_SCAN;
    C1101LoadPreset = true;

     float freq = atof(frequency_buffer);
        lv_textarea_set_text(text_area, "Waiting for signal.\n");
//     char freq_str[20]; // Buffer to hold the converted float as a string
//     snprintf(freq_str, sizeof(freq_str), "%f", freq);

//     lv_textarea_add_text(text_area, freq_str);
//    // lv_textarea_set_text(text_area, "  " );
//     lv_textarea_set_text(text_area, String("Capture Started..").c_str());

    CC1101.setFrequency(freq);
    CC1101.enableReceiver();
    delay(20);

}

void EVENTS::ta_buffert_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    ScreenManager& screenMgr = ScreenManager::getInstance();
    lv_obj_t* text_area = screenMgr.getTextArea();
    lv_obj_t* dropdown = screenMgr.getPresetDropdown();
    lv_dropdown_get_selected_str(dropdown, EVENTS::selected_str, sizeof(EVENTS::selected_str));

    if (code == LV_EVENT_VALUE_CHANGED) {

        lv_textarea_add_text(text_area, "Buffer size: ");
        lv_textarea_add_text(text_area,EVENTS::selected_str);
        lv_textarea_add_text(text_area, "\n");
    }

}

void EVENTS::saveSignal(lv_event_t * e) {
    Serial.print("event se spustil");
    CC1101.saveSignal();

}


int receivedProtocol;
int receivedBitLength;

void EVENTS::ProtAnalyzerloop() {
   //
    
}

bool EVENTS::initCC1101() {
  return false;
}

void EVENTS::listenRFEvent(lv_event_t * e) {
    char frequency_buffer[10];
    ScreenManager& screenMgr = ScreenManager::getInstance();
    lv_obj_t * text_area = screenMgr.getTextArea();
    lv_obj_t * ta = screenMgr.getFreqInput();
    strncpy(frequency_buffer, lv_textarea_get_text(ta), sizeof(frequency_buffer) - 1);
    frequency_buffer[sizeof(frequency_buffer) - 1] = '\0'; 

    C1101LoadPreset = false;

    float freq = atof(frequency_buffer);
    char freq_str[20]; // Buffer to hold the converted float as a string
    snprintf(freq_str, sizeof(freq_str), "%f", freq);

    lv_textarea_add_text(text_area, freq_str);
   // lv_textarea_set_text(text_area, "  " );
    lv_textarea_set_text(text_area, String("Capture Started..").c_str());

    ELECHOUSE_cc1101.setSidle();  // Set to idle state
    ELECHOUSE_cc1101.goSleep();   // Put CC1101 into sleep mode
    
    // Optionally disable chip select (CS) to fully power down the CC1101
    digitalWrite(CC1101_CS, HIGH); 

    CC1101.initrRaw();
    delay(20);
    C1101CurrentState = STATE_CAPTURE;

}

void EVENTS::replayEvent(lv_event_t * e) {
    ScreenManager& screenMgr = ScreenManager::getInstance();
    lv_obj_t * text_area = screenMgr.getTextArea();
    lv_textarea_set_text(text_area, "Sending signal from buffer");

    EVENTS::playFromBuffer();
  //  lv_textarea_add_text(text_area, "Signal has been send");
}

void EVENTS::exitReplayEvent(lv_event_t * e) {
    ScreenManager& screenMgr = ScreenManager::getInstance();
    screenMgr.createRFMenu();
}

void EVENTS::btn_event_subGhzTools(lv_event_t * e) {
    RFMenuScreen RFMenuScr;
    RFMenuScr.initialize();
}

const char* presetStrings[] = {
    "AM650",
    "AM270",
    "FM238",
    "FM476",
    "FM95",
    "FM15k",
    "PAGER",
    "HND1",
    "HND2"
};


CC1101_PRESET EVENTS::stringToCC1101Preset(String presetStr) {
    presetStr.trim();

    for (int i = 0; i < sizeof(presetStrings) / sizeof(presetStrings[0]); i++) {
        if (presetStr == presetStrings[i]) {
            return static_cast<CC1101_PRESET>(i);  
        }
    }

    Serial.println("Invalid preset string: " + presetStr);  
    return AM650; 
}
void EVENTS::saveRFSettingEvent(lv_event_t *e) {
    ScreenManager &screenMgr = ScreenManager::getInstance();
    lv_obj_t *dropdownSync = screenMgr.getSyncDropdown();
    lv_obj_t *dropdownPTK = screenMgr.getPTKDropdown();

    int syncIndex = lv_dropdown_get_selected(dropdownSync);
    int ptkIndex = lv_dropdown_get_selected(dropdownPTK);

    CC1101.setSync(syncIndex);
    CC1101.setPTK(ptkIndex);

    screenMgr.createReplayScreen();
}

void EVENTS::cancelRFSettingEvent(lv_event_t *e) {
    ScreenManager &screenMgr = ScreenManager::getInstance();
    screenMgr.createReplayScreen();
}

void EVENTS::playFromBuffer()
{
  Serial.print("event_playback_rec_play");

  if (C1101CurrentState == STATE_IDLE)
  {
    // ScreenManager &screenMgr = ScreenManager::getInstance();
    // lv_obj_t *ta = screenMgr.getTextArea();

    // float freq = String(lv_textarea_get_text(ta)).toFloat();
   // CC1101.setFrequency(freq);

    C1101CurrentState = STATE_PLAYBACK;
  }
  else
  {
    Serial.print("NOT IDLE");
  }
}

