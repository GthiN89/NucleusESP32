#include "GUI/ScreenManager.h"
#include "modules/ETC/SDcard.h"
#include <RCSwitch.h>
#include <Arduino.h>
#include "modules/RF/CC1101.h"
#include "globals.h"
#include <cstring> 
#include <iostream>
#include <unordered_map>
#include "ELECHOUSE_CC1101_SRC_DRV.h"
#include "main.h"
#include "modules/BLE/SourApple.h"
#include "modules/BLE/BLESpam.h"
#include "events.h"
#include "lv_fs_if.h"
#include <cstdio>   // For snprintf
#include "modules/dataProcessing/SubGHzParser.h"
using namespace std;

#define MAX_PATH_LENGTH 256

int SpamDevice = 1;

char* selected_file;

//CC1101_CLASS CC1101;

ScreenManager& screenMgr = ScreenManager::getInstance();


char EVENTS::frequency_buffer[10];
char EVENTS::selected_str[32];

//file explorer
lv_obj_t* EVENTS::selected_item;
String EVENTS::cur_path = "nothing";
String EVENTS::sel_fn = "nothing";
char* EVENTS::fullPath;
lv_obj_t* EVENTS::label_sub;
static char buffer[256];

bool isWarmupStarted;

void EVENTS::btn_event_playZero_run(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);    
    if (code == LV_EVENT_CLICKED) {
        screenMgr.createFileExplorerScreen();

        isWarmupStarted = true;
    }
}

void EVENTS::warmup() {
    // Serial.println(millis());

    // if (millis() - previousMillis >= 500) {
    //     previousMillis = millis();
    //     isWarmupStarted = false;

    //     SDInit();
    //     if (SD.exists("/warmpup1.sub")) {
    //         read_sd_card_flipper_file("/warmpup1.sub");
    //         detachInterrupt(CC1101_CCGDO0A);
    //         CC1101.initrRaw();
    //         ELECHOUSE_cc1101.setCCMode(0); 
    //         ELECHOUSE_cc1101.setPktFormat(3);
    //         ELECHOUSE_cc1101.SetTx();
    //         pinMode(CC1101_CCGDO0A, OUTPUT);
    //         SubGHzParser parser;
    //         parser.loadFile("/warmpup1.sub");
    //         SubGHzData data = parser.parseContent();
    //     } else {
    //         Serial.println("File does not exist.");
    //     }
    //      SDInit();
    //     if (SD.exists("/warmpup1.sub")) {
    //         read_sd_card_flipper_file("/warmpup1.sub");
    //         detachInterrupt(CC1101_CCGDO0A);
    //         CC1101.initrRaw();
    //         ELECHOUSE_cc1101.setCCMode(0); 
    //         ELECHOUSE_cc1101.setPktFormat(3);
    //         ELECHOUSE_cc1101.SetTx();
    //         pinMode(CC1101_CCGDO0A, OUTPUT);
    //         SubGHzParser parser;
    //         parser.loadFile("/warmpup1.sub");
    //         SubGHzData data = parser.parseContent();
    //     } else {
    //         Serial.println("File does not exist.");
    //     }
    // }
}


void EVENTS::btn_event_Replay_run(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {          
          screenMgr.createReplayScreen();
    }
}

void EVENTS::btn_event_BruteForce_run(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {          
          screenMgr.createBruteForceScreen();
    }
}

void EVENTS::btn_event_teslaCharger_run(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
     //   teslaScreen();
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
     lv_obj_t * kb = (lv_obj_t *)lv_event_get_user_data(e); 

    lv_obj_t * text_area = screenMgr.getTextArea();
     lv_obj_t * ta = screenMgr.getFreqInput();
    // lv_obj_t * presetDropdown = screenMgr.getPresetDropdown();

    if (code == LV_EVENT_FOCUSED) {
         lv_keyboard_set_textarea(kb, ta);
         lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);  
         Serial.println("Keyboard shown");
     } else if (code == LV_EVENT_DEFOCUSED || code == LV_EVENT_READY) {
         lv_keyboard_set_textarea(kb, NULL);
         lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);  

         strncpy(frequency_buffer, lv_textarea_get_text(ta), sizeof(frequency_buffer) - 1);
         frequency_buffer[sizeof(frequency_buffer) - 1] = '\0'; 
         Serial.print("Frequency set to: \n");
         
         CC1101.CC1101_FREQ = atof(lv_textarea_get_text(ta));
         lv_textarea_add_text(text_area, "Frequency set to: \n");
         Serial.println(frequency_buffer);
         lv_textarea_add_text(text_area, frequency_buffer);
     }    
}

void EVENTS::ta_filename_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * kb = static_cast<lv_obj_t *>(lv_event_get_user_data(e));

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


void EVENTS::save_RF_to_sd_event(lv_event_t * e) {
    lv_obj_t* text_area = screenMgr.getTextArea();
    lv_textarea_set_text(text_area, "Moving to SD\n");
   // CC1101.saveToSD();
    lv_textarea_set_text(text_area, "Done.");
}

void EVENTS::saveSignal(lv_event_t * e) {
    Serial.print("event se spustil");
    CC1101.saveSignal();
}


void EVENTS::replayEvent(lv_event_t * e) {
    lv_obj_t * text_area = screenMgr.getTextArea();
    lv_textarea_set_text(text_area, "Sending signal from buffer");

      Serial.print("event_playback_rec_play");

  if (C1101CurrentState == STATE_IDLE)
  {     
    lv_obj_t *ta = screenMgr.getTextArea();
    float freq = String(lv_textarea_get_text(ta)).toFloat();
    CC1101.setFrequency(freq);

    C1101CurrentState = STATE_PLAYBACK;
  }
  else
  {
    Serial.print("NOT IDLE");
  }
    lv_textarea_add_text(text_area, "Signal has been send");
}

void EVENTS::exitReplayEvent(lv_event_t * e) {
    screenMgr.createRFMenu();
}
void EVENTS::sendCapturedEvent(lv_event_t * e) {
    lv_obj_t * text_area = screenMgr.getTextAreaRCSwitchMethod();
    CC1101.sendRaw();
}

void EVENTS::btn_event_subGhzTools(lv_event_t * e) {
    screenMgr.createRFMenu();
}

 void EVENTS::btn_event_SourApple(lv_event_t * e){
    screenMgr.createSourAppleScreen();
    sourApple sa;
    sa.setup();
 }

  void EVENTS::btn_event_BTSpam(lv_event_t * e){
    screenMgr.createBTSPamScreen();
 }

  void EVENTS::btn_event_SourApple_Start(lv_event_t * e){
    lv_obj_t * ta = screenMgr.getTextAreaSourAple();
    lv_textarea_set_text(ta, "Running");
    BTCurrentState = STATE_SOUR_APPLE;
 }

  void EVENTS::btn_event_SourApple_Stop(lv_event_t * e){
    lv_obj_t * ta = screenMgr.getTextAreaSourAple();
    lv_textarea_set_text(ta, "Not running");
    BTCurrentState = STATE_SOUR_APPLE_IDLE;
 }

   void EVENTS::btn_event_BTSpam_Start(lv_event_t * e){
    //BLEDevice::init("");
    lv_obj_t * ta = screenMgr.getTextAreaBTSpam();
    lv_textarea_set_text(ta, "Running");
    int *SpamDevice = (int *)lv_event_get_user_data(e); 
    BLESpam spam;
    spam.aj_adv(*SpamDevice);
 }

  void EVENTS::btn_event_BTSpam_Stop(lv_event_t * e){
    lv_obj_t * ta = screenMgr.getTextAreaBTSpam();
    lv_textarea_set_text(ta, "Not running");
    BLESpam spam;
    BLEDevice::deinit();
 }

 

void EVENTS::btn_event_BTTools(lv_event_t * e) {
    screenMgr.createBTMenu();
}

void EVENTS::btn_event_mainMenu_run(lv_event_t* e) {   
    screenMgr.createmainMenu();
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




void EVENTS::ta_preset_event_cb(lv_event_t * e) {
     char selected_text[32];
     lv_event_code_t code = lv_event_get_code(e);
     lv_obj_t* text_area = screenMgr.getTextArea();

    lv_dropdown_get_selected_str(screenMgr.C1101preset_dropdown_, selected_text, sizeof(selected_text));  
    if (code == LV_EVENT_VALUE_CHANGED) {
        C1101preset = stringToCC1101Preset(selected_text);
        lv_textarea_add_text(text_area, "Preset set to: ");
        lv_textarea_add_text(text_area,selected_text);
        lv_textarea_add_text(text_area, "\n");
    }  

    CC1101_PRESET preset = convert_str_to_enum(selected_text);
    C1101preset = preset;
    CC1101.loadPreset();

} 


void EVENTS::ta_rf_type_event_cb(lv_event_t * e) {
     char selected_text[32];
     lv_event_code_t code = lv_event_get_code(e);
     lv_obj_t* text_area = screenMgr.getTextArea();

    lv_dropdown_get_selected_str(screenMgr.C1101type_dropdown_, selected_text, sizeof(selected_text)); 

    if (code == LV_EVENT_VALUE_CHANGED) {
        C1101preset = stringToCC1101Preset(selected_text);
        lv_textarea_add_text(text_area, "Type set to: ");
        lv_textarea_add_text(text_area,selected_text);
        lv_textarea_add_text(text_area, "\n");
    }  
}


void EVENTS::btn_event_RAW_REC_run(lv_event_t* e)
{
    char selected_text[32];
    char selected_text_type[32];
    char frequency_buffer[10];
    lv_obj_t * text_area = screenMgr.getTextArea();
    lv_obj_t * ta = screenMgr.getFreqInput();
    strncpy(frequency_buffer, lv_textarea_get_text(ta), sizeof(frequency_buffer) - 1);
    frequency_buffer[sizeof(frequency_buffer) - 1] = '\0'; 
    CC1101_MHZ = atof(frequency_buffer);
    lv_dropdown_get_selected_str(screenMgr.C1101preset_dropdown_, selected_text, sizeof(selected_text));  
    lv_dropdown_get_selected_str(screenMgr.C1101type_dropdown_, selected_text_type, sizeof(selected_text_type)); 

    lv_textarea_set_text(text_area, "Waiting for signal.\n");

     CC1101.setCC1101Preset(convert_str_to_enum(selected_text));
   //  CC1101.loadPreset();
     if(strcmp(selected_text_type, "Raw") == 0){
     CC1101.enableReceiver();
     } else {

        CC1101.enableRCSwitch();
        lv_textarea_add_text(text_area, "Decoder active.\n");
     }
   //  CC1101.setFrequency(CC1101_MHZ);
     delay(20);
    
    C1101CurrentState = STATE_ANALYZER;
}


void EVENTS::btn_event_brute_run(lv_event_t* e) {
    char string[32]; 
    CC1101.setCC1101Preset(AM650);
    lv_obj_t * text_area__BruteForce = screenMgr.getTextAreaBrute();
    lv_dropdown_get_selected_str(screenMgr.brute_dropdown_, string, sizeof(string));    
    lv_textarea_set_text(text_area__BruteForce, "Brute forcing");
    delay(1000);
    if(strcmp(string, "Czech Bells") == 0) {
        Serial.println("czech bells");
    }
    C1101CurrentState = STATE_BRUTE;
}



void EVENTS::confirm_delete_event_handler(lv_event_t * e)
{
    lv_obj_t * msgbox = static_cast<lv_obj_t *>(lv_event_get_user_data(e));
    lv_obj_t * yes_btn = static_cast<lv_obj_t *>(lv_obj_get_user_data(msgbox));
    lv_obj_t * clicked_btn = static_cast<lv_obj_t *>(lv_event_get_target(e));




    if (clicked_btn == yes_btn) {
        // if(deleteFile(cur_path)) {
        //     lv_obj_t * file_explorer = static_cast<lv_obj_t *>(lv_event_get_user_data(e));
        //     const char * cur_path = lv_file_explorer_get_current_path(file_explorer);
        //     lv_file_explorer_open_dir(file_explorer, cur_path);
        // } else {
        //     Serial.println(F("Failed to delete file."));
        // }
    } else {
        lv_obj_del(msgbox);
    }

}

bool EVENTS::deleteFile(const char *path)
{


    // Use lv_fs_remove to delete the file through the filesystem driver
  //  lv_fs_res_t res = fs_remove(nullptr, path);

    // if(res == LV_FS_RES_OK) {
    //     Serial.println(F("File deleted successfully.");
    //     return true;
    // } else {
    //     Serial.println(F("Failed to delete file. Error code: %d", res);
    //     return false;
    // }
    return true;
}



void EVENTS::file_explorer_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    const lv_obj_t * file_explorer = static_cast<lv_obj_t *>(lv_event_get_target(e));    
    
    if(code == LV_EVENT_VALUE_CHANGED) {
        sel_fn = String(lv_file_explorer_get_selected_file_name(file_explorer));
        if(sel_fn.endsWith(".sub")) {          
            cur_path =  String(lv_file_explorer_get_current_path(file_explorer));
            String tempPath = String(cur_path) + sel_fn;
            tempPath = tempPath.substring(3);
            strncpy(buffer, tempPath.c_str(), sizeof(buffer) - 1);
            buffer[sizeof(buffer) - 1] = '\0';  
            EVENTS::fullPath = buffer;
            EVENTS::file_explorer_play_sub();  
        }

    }
    if(code == LV_EVENT_READY) {
        cur_path =  String(lv_file_explorer_get_current_path(file_explorer)); 
    }          
}

void EVENTS::file_explorer_event_delete_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {

lv_obj_t *mbox = lv_msgbox_create(lv_scr_act());
lv_obj_set_size(mbox, 220, 120);
lv_obj_align(mbox, LV_ALIGN_CENTER, 0, 0);
lv_obj_clear_flag(mbox, LV_OBJ_FLAG_SCROLLABLE); 


lv_obj_t *content_container = lv_obj_create(mbox);
lv_obj_set_size(content_container, LV_PCT(100), LV_PCT(100));  
lv_obj_set_flex_flow(content_container, LV_FLEX_FLOW_COLUMN);
lv_obj_set_style_pad_all(content_container, 10, 0);
lv_obj_clear_flag(content_container, LV_OBJ_FLAG_SCROLLABLE); 



lv_obj_t *label = lv_label_create(content_container);
lv_label_set_text_fmt(label, String("Do you want to delete\n" + sel_fn + "?").c_str());
lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 10); 

lv_obj_t *button_container = lv_obj_create(content_container);
lv_obj_set_size(button_container, LV_PCT(100), LV_SIZE_CONTENT);
lv_obj_set_flex_flow(button_container, LV_FLEX_FLOW_ROW);
lv_obj_set_flex_align(button_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
lv_obj_clear_flag(button_container, LV_OBJ_FLAG_SCROLLABLE); 


lv_obj_t *yes_btn = lv_btn_create(button_container);
lv_obj_set_size(yes_btn, 60, 30);
lv_obj_t *yes_label = lv_label_create(yes_btn);
lv_label_set_text(yes_label, "Yes");

lv_obj_t *no_btn = lv_btn_create(button_container);
lv_obj_set_size(no_btn, 60, 30);
lv_obj_t *no_label = lv_label_create(no_btn);
lv_label_set_text(no_label, "No");

lv_obj_set_user_data(mbox, yes_btn); 
lv_obj_add_event_cb(yes_btn, EVENTS::confirm_delete_event_handler, LV_EVENT_CLICKED, mbox);
lv_obj_add_event_cb(no_btn, EVENTS::confirm_delete_event_handler, LV_EVENT_CLICKED, mbox);
    }
}


lv_obj_t *button_container;

void EVENTS::file_explorer_play_sub()
{

lv_obj_t *mbox = lv_msgbox_create(lv_scr_act());
lv_obj_set_size(mbox, 220, 120);
lv_obj_align(mbox, LV_ALIGN_CENTER, 0, 0);
lv_obj_clear_flag(mbox, LV_OBJ_FLAG_SCROLLABLE); 



lv_obj_t *content_container = lv_obj_create(mbox);
lv_obj_set_size(content_container, LV_PCT(100), LV_PCT(100));  
lv_obj_set_flex_flow(content_container, LV_FLEX_FLOW_COLUMN);
lv_obj_set_style_pad_all(content_container, 10, 0);  
lv_obj_clear_flag(content_container, LV_OBJ_FLAG_SCROLLABLE); 



label_sub = lv_label_create(content_container);
lv_label_set_text(label_sub, String("Do you want to transmit\n" + sel_fn + " ?").c_str());
lv_obj_align(label_sub, LV_ALIGN_TOP_MID, 0, 10); 

button_container = lv_obj_create(content_container);
lv_obj_set_size(button_container, LV_PCT(100), LV_SIZE_CONTENT);
lv_obj_set_flex_flow(button_container, LV_FLEX_FLOW_ROW);
lv_obj_set_flex_align(button_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
lv_obj_clear_flag(button_container, LV_OBJ_FLAG_SCROLLABLE); 

lv_obj_t *yes_btn = lv_btn_create(button_container);
lv_obj_set_size(yes_btn, 60, 30);
lv_obj_t *yes_label = lv_label_create(yes_btn);
lv_label_set_text(yes_label, "Yes");

lv_obj_t *no_btn = lv_btn_create(button_container);
lv_obj_set_size(no_btn, 60, 30);
lv_obj_t *no_label = lv_label_create(no_btn);
lv_label_set_text(no_label, "No");

lv_obj_set_user_data(mbox, yes_btn); 
lv_obj_add_event_cb(yes_btn, EVENTS::confirm__explorer_play_sub_cb, LV_EVENT_CLICKED, mbox);
lv_obj_add_event_cb(no_btn, EVENTS::confirm__explorer_play_sub_cb, LV_EVENT_CLICKED, mbox);

}

void EVENTS::confirm__explorer_play_sub_cb(lv_event_t * e)
{
    lv_obj_t * msgbox = static_cast<lv_obj_t *>(lv_event_get_user_data(e));
    lv_obj_t * yes_btn = static_cast<lv_obj_t *>(lv_obj_get_user_data(msgbox));
    lv_obj_t * clicked_btn = static_cast<lv_obj_t *>(lv_event_get_target(e));

    if (clicked_btn == yes_btn) {
    
            Serial.print("Transmiting?");
            Serial.println(EVENTS::fullPath);
            lv_label_set_text(label_sub, "Transmitting\n");
            lv_obj_clean(button_container);
            lv_obj_set_size(button_container, LV_PCT(100), LV_SIZE_CONTENT);
            lv_obj_set_flex_flow(button_container, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(button_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_t *close_btn = lv_btn_create(button_container);
            lv_obj_set_size(close_btn, 80, 30);
            lv_obj_t *close_btn_lbl = lv_label_create(close_btn);
            lv_label_set_text(close_btn_lbl, "Close");
            lv_obj_add_event_cb(close_btn, EVENTS::close_explorer_play_sub_cb, LV_EVENT_CLICKED, msgbox);


        //warm up of CC1101
  


    Serial.println("Load button clicked.");
    if (strlen(EVENTS::fullPath) > 0) {
        detachInterrupt(CC1101_CCGDO0A);
        char* taskFullPath = strdup(EVENTS::fullPath);
          xTaskCreatePinnedToCore(
            EVENTS::CC1101TransmitTask, // Function to run 
            "Sub transmit",       // Name of the task
            1000,           // Stack size (in bytes)
            taskFullPath,            // Task input parameter
            5,               // Priority
            NULL,            // Task handle
            1                // Core
        );
    

    }

        // Signal transmitted, so let's refresh the screen
                   
        } else {
        lv_obj_del(msgbox);
    }
    
}


void EVENTS::close_explorer_play_sub_cb(lv_event_t * e) {
    lv_obj_t * msgbox = static_cast<lv_obj_t *>(lv_event_get_user_data(e));
    lv_obj_del(msgbox);
   // CC1101.disableTransmit();
   // digitalWrite(CC1101_CS, HIGH);
  //  C1101CurrentState = STATE_IDLE;
}



//transmit task on core 1

void EVENTS::CC1101TransmitTask(void* pvParameters) {    
    
    char* fullPath = static_cast<char*>(pvParameters);

    detachInterrupt(CC1101_CCGDO0A);
    CC1101.initrRaw();
    ELECHOUSE_cc1101.setCCMode(0); 
    ELECHOUSE_cc1101.setPktFormat(3);
    ELECHOUSE_cc1101.SetTx();
    pinMode(CC1101_CCGDO0A, OUTPUT);
    
    Serial.print("Loading file: ");
    Serial.println(fullPath);

    SubGHzParser parser;
    parser.loadFile(fullPath);
    SubGHzData data = parser.parseContent();

    SDInit();

    Serial.print("Using file at path: ");
    Serial.println(fullPath);

    if (SD.exists(fullPath)) {
        read_sd_card_flipper_file(fullPath);
        delay(1);
    } else {
        Serial.println("File does not exist.");
    }
    
    free(fullPath);

    vTaskDelete(NULL);
}
