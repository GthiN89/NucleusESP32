#include "GUI/ScreenManager.h"
#include <RCSwitch.h>
#include <Arduino.h>
#include "modules/RF/CC1101.h"
#include <cstring> 
#include <iostream>
#include <unordered_map>
#include "ELECHOUSE_CC1101_SRC_DRV.h"
#include "modules/BLE/SourApple.h"
#include "modules/BLE/BLESpam.h"
#include "events.h"
#include "lv_fs_if.h"
#include <cstdio>   
#include "modules/dataProcessing/SubGHzParser.h"
using namespace std;
#include "modules/ETC/SDcard.h"
#include "modules/IR/TV-B-Gone.h"
#include "modules/IR/ir.h"
#include "lvgl.h"
#include "modules/nfc/nfc.h"
#include "modules/RF/Radio.h"
#include "main.h"

#define MAX_PATH_LENGTH 256
CC1101_CLASS CC1101EV;
SDcard& SD_EVN = SDcard::getInstance();

int SpamDevice = 1;
bool updatetransmitLabel = false;
bool stopTransmit = false;

//datarate
int values[32];

char* selected_file;

int32_t valuesDrate[] = {300, 1200, 2400, 4800, 9600, 19200, 38400, 50000, 76800, 100000, 150000, 200000, 250000};
int32_t current_indexDrate = 0;
const int32_t value_countDrate = sizeof(values) / sizeof(values[0]);


ScreenManager& screenMgr = ScreenManager::getInstance();

lv_obj_t *button_container;

char EVENTS::frequency_buffer[10];
char EVENTS::selected_str[32];

//file explorer
lv_obj_t* EVENTS::selected_item;
String EVENTS::cur_path = "nothing";
String EVENTS::sel_fn = "nothing";
char* EVENTS::fullPath;
lv_obj_t* label_sub;
static char buffer[256];

TaskHandle_t taskHandle = NULL; 

void EVENTS::btn_event_playZero_run(lv_event_t* e) {
        lv_event_code_t code = lv_event_get_code(e);    
    if (code == LV_EVENT_CLICKED) {
    digitalWrite(SDCARD_CS_PIN, LOW);
   // delay(10); // Allow SD card to stabilize
    if (!SD_EVN.initializeSD()) {
        Serial.println(F("Failed to initialize SD card!"));
    }

        screenMgr.createFileExplorerScreen();
    }
}

void EVENTS::btn_event_Replay_run(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
                 
          screenMgr.createReplayScreen();
    }
}

void EVENTS::btn_event_detectForce_run(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {          
          screenMgr.createRFdetectScreen();
    }
}

void EVENTS::btn_event_teslaCharger_run(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        screenMgr.createTeslaScreen();
     //   delay(10);
        detachInterrupt(CC1101_CCGDO0A);
        CC1101_MODULATION = 2;
        CC1101EV.CC1101_FREQ = 433.92;
        CC1101EV.CC1101_PKT_FORMAT = 3;
        CC1101EV.initrRaw();
    }
}

void EVENTS::btn_event_IR_menu_run(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
            screenMgr.createIRMenuScreen();
    }
}

void EVENTS::btn_event_NFC_menu_run(lv_event_t* e) {
    //  lv_event_code_t code = lv_event_get_code(e);
    // if (code == LV_EVENT_CLICKED) {
    // RadioReceiver radio;
    // Serial.println("Test");
    // radio.setup();
    // radio.loop();
    // }
}

void EVENTS::btn_event_RF24_menu_run(lv_event_t* e) {
   // testRF24();
 //   RF24CurrentState = RF24_STATE_TEST;
}

void EVENTS::ta_freq_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *kb = screenMgr.getKeyboardFreq(); // Get the keyboard object
    const char *user_data = static_cast<const char *>(lv_event_get_user_data(e));

    if (code == LV_EVENT_FOCUSED) {
        if(strcmp(user_data, "freq") == 0) {
            lv_keyboard_set_textarea(kb, screenMgr.customPreset); // Link the textarea to the keyboard
        }
        if(strcmp(user_data, "drate") == 0) {
            lv_keyboard_set_textarea(kb, screenMgr.input_datarate); // Link the textarea to the keyboard
        }
        if(strcmp(user_data, "BW") == 0) {
            lv_keyboard_set_textarea(kb, screenMgr.input_bandwidth); // Link the textarea to the keyboard
        }
        if(strcmp(user_data, "DEV") == 0) {
            lv_keyboard_set_textarea(kb, screenMgr.input_deviation); // Link the textarea to the keyboard
        }
        lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN); // Show the keyboard
        Serial.println("Keyboard linked and shown");
    } else if (code == LV_EVENT_DEFOCUSED || code == LV_EVENT_READY) {
        lv_keyboard_set_textarea(kb, nullptr); // Unlink the keyboard
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN); // Hide the keyboard

        if(strcmp(user_data, "freq") == 0) {
            const char *freq_text = lv_textarea_get_text(screenMgr.customPreset);
        if (freq_text && strlen(freq_text) > 0) {
            CC1101EV.CC1101_FREQ = atof(freq_text); // Convert text to float
            Serial.printf("Frequency set to: %.2f MHz\n", CC1101EV.CC1101_FREQ);

            lv_obj_t *text_area = screenMgr.getTextArea();
            lv_textarea_add_text(text_area, "Frequency set to: ");
            lv_textarea_add_text(text_area, freq_text);
            lv_textarea_add_text(text_area, " MHz\n");
        } else {
            Serial.println("Frequency input is empty");
        }

        if(strcmp(user_data, "drate") == 0) {
            const char *drate_text = lv_textarea_get_text(screenMgr.input_datarate);
            CC1101EV.CC1101_DRATE = atof(drate_text);
        }
        if(strcmp(user_data, "BW") == 0) {
            const char *drate_text = lv_textarea_get_text(screenMgr.input_bandwidth);
            CC1101EV.CC1101_RX_BW = atof(drate_text);
        }
        if(strcmp(user_data, "DEV") == 0) {
            const char *drate_text = lv_textarea_get_text(screenMgr.input_deviation);
            CC1101EV.CC1101_DEVIATION = atof(drate_text);
        }

        }

    }
}

void EVENTS::dropdown_modulation_event_cb(lv_event_t *e) {
      lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
    int selected = lv_dropdown_get_selected(screenMgr.dropdown_modulation); // Get selected index

    if (selected == 0) { // ASK selected
        CC1101_MODULATION = 2;
        Serial.println("Modulation set to ASK.");
    } else if (selected == 1) { // FSK selected
        CC1101_MODULATION = 0;
        Serial.println("Modulation set to FSK.");
    }
    }
}

enum InputType {
    DATARATE,
    BANDWIDTH,
    DEVIATION,
    NONVALID
};

// Helper function to identify the input type
InputType get_input_type(lv_obj_t *textarea) {
    if (textarea == screenMgr.input_datarate) {
        return DATARATE;
    } else if (textarea == screenMgr.input_bandwidth) {
        return BANDWIDTH;
    } else if (textarea == screenMgr.input_deviation) {
        return DEVIATION;
    } else {
        return NONVALID;
    }
}

void EVENTS::ok_button_event_cb(lv_event_t *e) {
        lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
    lv_obj_t *container = static_cast<lv_obj_t *>(lv_event_get_user_data(e));
    lv_obj_t *textareas[] = { screenMgr.input_datarate, screenMgr.input_bandwidth, screenMgr.input_deviation };
    const char *messages[] = {
        "Data rate must be between 1-500 kbps and cannot be empty.",
        "Bandwidth must be between 50-600 kHz and cannot be empty.",
        "Deviation must be between 1-200 kHz and cannot be empty."
    };

    bool valid = true;
    char message[128];

    // Validate each input
    for (lv_obj_t *textarea : textareas) {
        const char *input = lv_textarea_get_text(textarea);
        int value = atoi(input);

        switch (get_input_type(textarea)) {
            case DATARATE:
                if (strlen(input) == 0 || value < 1 || value > 500) {
                    valid = false;
                    snprintf(message, sizeof(message), "%s", messages[DATARATE]);
                }
                break;
            case BANDWIDTH:
                if (strlen(input) == 0 || value < 50 || value > 600) {
                    valid = false;
                    snprintf(message, sizeof(message), "%s", messages[BANDWIDTH]);
                }
                break;
            case DEVIATION:
                if (strlen(input) == 0 || value < 1 || value > 200) {
                    valid = false;
                    snprintf(message, sizeof(message), "%s", messages[DEVIATION]);
                }
                break;
            default:
                valid = false;
                snprintf(message, sizeof(message), "Unknown input field.");
                break;
        }

        if (!valid) {
            lv_obj_t *msgbox = lv_msgbox_create(NULL);
            lv_obj_center(msgbox);
            return; // Prevent hiding the container
        }
    }

    if (container) {
        lv_obj_add_flag(container, LV_OBJ_FLAG_HIDDEN); // Hide the container
        Serial.printf("Saving values: Data Rate = %s kbps, Bandwidth = %s kHz, Deviation = %s kHz\n",
                      lv_textarea_get_text(screenMgr.input_datarate),
                      lv_textarea_get_text(screenMgr.input_bandwidth),
                      lv_textarea_get_text(screenMgr.input_deviation));
        lv_msgbox_create(NULL);
    }
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
        lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
    lv_obj_t* text_area = screenMgr.getTextArea();
    lv_textarea_set_text(text_area, "Moving to SD\n");
   // CC1101EV.saveToSD();
    lv_textarea_set_text(text_area, "Done.");
    }
}

void EVENTS::saveSignal(lv_event_t * e) {
        lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
    Serial.print("event se spustil");
    CC1101EV.saveSignal();
    }
}

void EVENTS::closeCC1101scanner(lv_event_t * e){    
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        CC1101EV.disableReceiver();
        C1101CurrentState = STATE_IDLE;
        runningModule = MODULE_NONE;
        screenMgr.createRFMenu();
    }
}

void EVENTS::replayEvent(lv_event_t * e) {
    lv_obj_t * text_area = screenMgr.getTextArea();
    lv_textarea_set_text(text_area, "Sending signal from buffer");
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
      Serial.print("event_playback_rec_play");

  if (C1101CurrentState == STATE_IDLE)
  {     
    lv_obj_t *ta = screenMgr.getTextArea();
    float freq = String(lv_textarea_get_text(ta)).toFloat();
    CC1101EV.setFrequency(freq);

    C1101CurrentState = STATE_PLAYBACK;
    runningModule = MODULE_CC1101;
  }
  else
  {
    Serial.print("NOT IDLE");
  }
    lv_textarea_add_text(text_area, "Signal has been send");
    }
}

void EVENTS::exitReplayEvent(lv_event_t * e) {
        lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
    screenMgr.createRFMenu();
    }
}
void EVENTS::sendCapturedEvent(lv_event_t * e) {
        lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
    CC1101EV.sendRaw();
    }
}

void EVENTS::sendCapturedIREvent(lv_event_t * e) {
        lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
    sendReceived();
    }
}

void EVENTS::btn_event_subGhzTools(lv_event_t * e) {
     lv_event_code_t code = lv_event_get_code(e);
     if (code == LV_EVENT_CLICKED) {
    screenMgr.createRFMenu();
     }
}

void EVENTS::btn_event_UR_BGONE(lv_event_t * e) {
        lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
    IRCurrentState = IR_STATE_BGONE;
    runningModule = MODULE_IR;
    }
}

void EVENTS::btn_event_IR_START_READ(lv_event_t * e) {
        lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
    screenMgr.createIRRecScreen();
    }
}

 void EVENTS::btn_event_SourApple(lv_event_t * e){
    // screenMgr.createSourAppleScreen();
    // sourApple sa;
    // sa.setup();
 }

  void EVENTS::btn_event_BTSpam(lv_event_t * e){
 //   screenMgr.createBTSPamScreen();
 }

  void EVENTS::btn_event_SourApple_Start(lv_event_t * e){
  //  lv_obj_t * ta = screenMgr.getTextAreaSourAple();
  //  lv_textarea_set_text(ta, "Running");
   // BTCurrentState = STATE_SOUR_APPLE;
 }

  void EVENTS::btn_event_SourApple_Stop(lv_event_t * e){
 //   lv_obj_t * ta = screenMgr.getTextAreaSourAple();
  //  lv_textarea_set_text(ta, "Not running");
  //  BTCurrentState = STATE_SOUR_APPLE_IDLE;
 }

   void EVENTS::btn_event_BTSpam_Start(lv_event_t * e){
//     //BLEDevice::init("");
//     lv_obj_t * ta = screenMgr.getTextAreaBTSpam();
//     lv_textarea_set_text(ta, "Running");
//     int *SpamDevice = (int *)lv_event_get_user_data(e); 
//    // BLESpam spam;
//     spam.aj_adv(*SpamDevice);
 }

  void EVENTS::btn_event_BTSpam_Stop(lv_event_t * e){
    // lv_obj_t * ta = screenMgr.getTextAreaBTSpam();
    // lv_textarea_set_text(ta, "Not running");
    // //BLESpam spam;
    // BLEDevice::deinit();
 }

 

void EVENTS::btn_event_BTTools(lv_event_t * e) {
        lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        screenMgr.createBTMenu();
}
}
void EVENTS::btn_event_mainMenu_run(lv_event_t* e) {   
     lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
    screenMgr.createmainMenu();
}
}

void EVENTS::ta_preset_event_cb(lv_event_t * e) {
     lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
     char selected_text[32];
     lv_event_code_t code = lv_event_get_code(e);

    lv_dropdown_get_selected_str(screenMgr.C1101preset_dropdown_, selected_text, sizeof(selected_text));  
    if (code == LV_EVENT_VALUE_CHANGED) {
        C1101preset = convert_str_to_enum(selected_text);
        if(C1101preset == CUSTOM) {
            lv_obj_remove_flag(screenMgr.mbox_container, LV_OBJ_FLAG_HIDDEN);




        }

    }  

    C1101preset = convert_str_to_enum(selected_text);
    CC1101EV.loadPreset();

} 
}

void EVENTS::ta_rf_type_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
     if (code == LV_EVENT_CLICKED) {
     char selected_text[32];
     lv_event_code_t code = lv_event_get_code(e);
     lv_obj_t* text_area = screenMgr.getTextArea();

    lv_dropdown_get_selected_str(screenMgr.C1101type_dropdown_, selected_text, sizeof(selected_text)); 

    if (code == LV_EVENT_VALUE_CHANGED) {
        C1101preset = convert_str_to_enum(selected_text);
        lv_textarea_add_text(text_area, "Type set to: ");
        lv_textarea_add_text(text_area,selected_text);
        lv_textarea_add_text(text_area, "\n");
    }  
     }
}

void EVENTS::btn_event_IR_run(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
     if (code == LV_EVENT_CLICKED) {

    IRCurrentState = IR_STATE_LISTENING;
        runningModule = MODULE_IR;

     }
}


void EVENTS::btn_event_RAW_REC_run(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
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

 //    CC1101EV.setCC1101Preset(convert_str_to_enum(selected_text));
 //    CC1101EV.loadPreset();
     if(strcmp(selected_text_type, "Raw") == 0){
     CC1101EV.enableReceiver();
     } else {

      //  CC1101EV.enableRCSwitch();
    lv_textarea_add_text(text_area, "Decoder active.\n");
     }
     CC1101EV.setFrequency(CC1101_MHZ);
   //  delay(20);
    runningModule = MODULE_CC1101;
    C1101CurrentState = STATE_ANALYZER;
    
    }
}

void EVENTS::sendTesla(lv_event_t* e) {   
    lv_event_code_t code = lv_event_get_code(e);
     if (code == LV_EVENT_CLICKED) {
    const uint16_t pulseWidth = 400;
    const uint8_t messageLength = 43;
    const uint8_t sequence[messageLength] = {
    0x02, 0xAA, 0xAA, 0xAA, // Preamble of 26 bits by repeating 1010
    0x2B,                   // Sync byte
    0x2C, 0xCB, 0x33, 0x33, 0x2D, 0x34, 0xB5, 0x2B, 0x4D, 0x32, 0xAD, 0x2C, 0x56, 0x59, 0x96, 0x66,
    0x66, 0x5A, 0x69, 0x6A, 0x56, 0x9A, 0x65, 0x5A, 0x58, 0xAC, 0xB3, 0x2C, 0xCC, 0xCC, 0xB4, 0xD2,
    0xD4, 0xAD, 0x34, 0xCA, 0xB4, 0xA0};
    
    CC1101EV.sendByteSequence(sequence, pulseWidth, messageLength);
    digitalWrite(CC1101_CCGDO0A, LOW);
     }
}


void EVENTS::btn_event_detect_run(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
     if (code == LV_EVENT_CLICKED) {
   // char string[32]; 
   Serial.println("Scanner");
   
  //  lv_obj_t * detectLabe = screenMgr.getdetectLabel();
   // lv_dropdown_get_selected_str(screenMgr.detect_dropdown_, string, sizeof(string));    
    CC1101EV.setCC1101Preset(AM650);
    CC1101EV.loadPreset();
    CC1101EV.enableScanner(300, 925);
    Serial.println("Scanner2");
    CC1101EV.startSignalanalyseTask();
 //   delay(5);
    C1101CurrentState = STATE_DETECT;
    runningModule = MODULE_CC1101;

      Serial.println("Scanner3");  
     }

}



void EVENTS::confirm_delete_event_handler(lv_event_t * e)
{
    lv_obj_t * msgbox = static_cast<lv_obj_t *>(lv_event_get_user_data(e));
    lv_obj_t * yes_btn = static_cast<lv_obj_t *>(lv_obj_get_user_data(msgbox));
    lv_obj_t * clicked_btn = static_cast<lv_obj_t *>(lv_event_get_target(e));


    lv_event_code_t code = lv_event_get_code(e);
     if (code == LV_EVENT_CLICKED) {

    if (clicked_btn == yes_btn) {

        if(deleteFile(sel_fn.c_str())) {
            lv_obj_t * file_explorer = static_cast<lv_obj_t *>(lv_event_get_user_data(e));
            const char * cur_path = lv_file_explorer_get_current_path(file_explorer);
            lv_file_explorer_open_dir(file_explorer, cur_path);
            delay(20);
            lv_obj_del(msgbox);
            lv_file_explorer_open_dir(file_explorer, cur_path);        } else {
            Serial.println(F("Failed to delete file."));
        }
    } else {
        lv_obj_del(msgbox);
    }

     }

}

void EVENTS::confirm_deleteAll_event_handler(lv_event_t *e) {

    lv_obj_t *msgbox = static_cast<lv_obj_t *>(lv_event_get_user_data(e));
    lv_obj_t *yes_btn = static_cast<lv_obj_t *>(lv_obj_get_user_data(msgbox));
    lv_obj_t *clicked_btn = static_cast<lv_obj_t *>(lv_event_get_target(e));

    if (clicked_btn == yes_btn) {
        lv_obj_t *file_explorer = static_cast<lv_obj_t *>(lv_event_get_user_data(e));
        const char *cur_path = lv_file_explorer_get_current_path(file_explorer);
        lv_file_explorer_open_dir(file_explorer, cur_path);
        oflag_t openMode = O_RDWR;
        File32 *root = SD_EVN.createOrOpenFile(EVENTS::fullPath, openMode);
        
    File32 file = root->openNextFile();

	while (file) {
		if (file.isDirectory()) {
			Serial.print("  DIR : ");
			Serial.println(file.name());
		}
		else {
			Serial.print("  FILE: ");
			Serial.print(file.name());
			Serial.print("  SIZE: ");
			Serial.print(file.size());
		}
		file = root->openNextFile(O_RDWR);
	}
 
        root->remove();
        root->close();
        Serial.println("All files in the directory deleted.");
    } else {
        lv_obj_del(msgbox);
    }
}


bool EVENTS::deleteFile(const char *path)
{   oflag_t openMode = O_RDWR;
    File32* file = SD_EVN.createOrOpenFile(EVENTS::fullPath, openMode);    

    if(file->remove()) {
        Serial.println(F("File deleted successfully."));
        return true;
    } else {
        Serial.println("Failed to delete file.");
        return false;
    }
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

    if(code == LV_EVENT_LONG_PRESSED) {
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
lv_label_set_text_fmt(label, String("Do you want to delete all?").c_str());
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
lv_obj_add_event_cb(yes_btn, EVENTS::confirm_deleteAll_event_handler, LV_EVENT_CLICKED, mbox);
lv_obj_add_event_cb(no_btn, EVENTS::confirm_delete_event_handler, LV_EVENT_CLICKED, mbox);
    } else if(code == LV_EVENT_PRESSED) {

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

     lv_event_code_t code = lv_event_get_code(e);
     if (code == LV_EVENT_CLICKED) {
    lv_obj_t * msgbox = static_cast<lv_obj_t *>(lv_event_get_user_data(e));
    lv_obj_t * yes_btn = static_cast<lv_obj_t *>(lv_obj_get_user_data(msgbox));
    lv_obj_t * clicked_btn = static_cast<lv_obj_t *>(lv_event_get_target(e));

    if (clicked_btn == yes_btn) {
    SD_EVN.restartSD();
            Serial.print("Transmiting?");
            Serial.println(EVENTS::fullPath);
            String text = "Transmitting\n Codes send: " + String(codesSend);
            lv_label_set_text(label_sub, text.c_str());
         //   updatetransmitLabel = true;
            lv_obj_clean(button_container);
            lv_obj_set_size(button_container, LV_PCT(100), LV_SIZE_CONTENT);
            lv_obj_set_flex_flow(button_container, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(button_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_t *close_btn = lv_btn_create(button_container);
            lv_obj_set_size(close_btn, 80, 30);
            lv_obj_t *close_btn_lbl = lv_label_create(close_btn);
            lv_label_set_text(close_btn_lbl, "Close");
            lv_obj_add_event_cb(close_btn, EVENTS::close_explorer_play_sub_cb, LV_EVENT_CLICKED, msgbox);


    Serial.println("Load button clicked.");
    if (strlen(EVENTS::fullPath) > 0) {
        detachInterrupt(CC1101_CCGDO0A);
        detachInterrupt(CC1101_CCGDO2A);
        char* taskFullPath = strdup(EVENTS::fullPath);
          xTaskCreatePinnedToCore(
            EVENTS::CC1101TransmitTask, // Function to run 
            "Sub transmit",       // Name of the task
            30000,           // Stack size (in bytes)
            taskFullPath,            // Task input parameter
            5,               // Priority
            &taskHandle,
            1                // Core
        );
    

    }                   
        } else {
            updatetransmitLabel = false;
        lv_obj_del(msgbox);
    }
     }
    
}


void EVENTS::close_explorer_play_sub_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
    stopTransmit = true;
    codesSend = 0;
    lv_obj_t * msgbox = static_cast<lv_obj_t *>(lv_event_get_user_data(e));
    lv_obj_del(msgbox);
    CC1101EV.disableTransmit();
    digitalWrite(CC1101_CS, HIGH);
    C1101CurrentState = STATE_IDLE;
    runningModule = MODULE_NONE;
}
}



//transmit task on core 1

void EVENTS::CC1101TransmitTask(void* pvParameters) {    
    

    char* fullPath = static_cast<char*>(pvParameters);

    detachInterrupt(CC1101_CCGDO0A);
    detachInterrupt(CC1101_CCGDO2A);
    digitalWrite(CC1101_CS, LOW);
    CC1101EV.initrRaw();
    ELECHOUSE_cc1101.setCCMode(0); 
    ELECHOUSE_cc1101.setPktFormat(3);
    ELECHOUSE_cc1101.SetTx();
    pinMode(CC1101_CCGDO0A, OUTPUT);
    digitalWrite(CC1101_CS, HIGH);
    
    Serial.print("Loading file: ");
    Serial.println(fullPath);
      if (SD_EVN.fileExists(fullPath)) {
     SD_EVN.read_sd_card_flipper_file(fullPath);
        SubGHzParser parser;
    parser.loadFile(fullPath);
    SubGHzData data = parser.parseContent();
   //   delay(1);
  } else {
      Serial.println("File does not exist.");
  }


    vTaskDelete(taskHandle);
    }



