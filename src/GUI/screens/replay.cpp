#include "screens.h"
//#include "modules/RF/subGhz.h"
#include "modules/ETC/SDcard.h"
#include <lvgl.h>
#include <iostream>
#include <cstring>
#include "playZero.h"
#include "GUI/screens/replay.h"
#include "globals.h"
#include "GUI/ScreenManager.h"
#include "GUI/constants.h"


#include "../events.h"
#include "../menus/MainMenuScreen.h"
#include "../../modules/ETC/SDcard.h"

/* Global variable to store the reference to the text area */
lv_obj_t *text_are_;
lv_obj_t *ta_;
lv_obj_t *ta_filename;

static lv_obj_t * kb;
static lv_obj_t * kb_qwert;

lv_obj_t* C1101AnaCont = nullptr;
lv_obj_t* c1101AnaScreen = nullptr;
lv_obj_t* subghz_analyzer_label = nullptr;
lv_obj_t* txtProtAnaReceived;
lv_obj_t* txtProtAnaBitLength;
lv_obj_t* txtProtAnaBinary;
lv_obj_t* txtProtAnaPulsLen;
lv_obj_t* txtProtAnaProtAnaTriState;
lv_obj_t* txtProtAnaProtocol;
lv_obj_t* txtProtAnaResults;
lv_obj_t* label;
lv_obj_t* label_filename;
lv_obj_t* fileName_container;
lv_obj_t* topLabel_container;
lv_obj_t* button_container1;
lv_obj_t* button_container2;


 lv_obj_t* container;
 char frequency_buffer[10];
 char selected_str[32]; 
 bool ProtAnaRxEn;
 char filename_buffer[32];

bool recievedSubGhz = false;
bool receivedProtocol_;

//RCSwitch mySwitch = RCSwitch();
//CC1101 cc1101(CC1101_SCLK, CC1101_MISO, CC1101_MOSI, CC1101_CS, CCGDO2A, CCGDO0A);

void kb_qwert_event_cb(lv_event_t * e);

volatile long last_micros;
String buffer_RAW;

float start_freq = 433;
float stop_freq = 434;

float freq = start_freq;
long compare_freq;
float mark_freq;
int rssi;
int mark_rssi = -100;

int receivedBitLength_;

static const uint32_t subghz_frequency_list[] = {
    300000000, 303875000, 304250000, 310000000, 315000000, 318000000,  //  300-348 MHz
    390000000, 418000000, 433075000, 433420000, 433920000, 434420000, 434775000, 438900000,  //  387-464 MHz
    868350000, 868000000, 915000000, 925000000  //  779-928 MHz
};

int currentFrequencyIndex = 0;

/* Event handler for frequency text area to show and hide keyboard */
void ta_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * kb = (lv_obj_t *)lv_event_get_user_data(e);  // Cast void* to lv_obj_t*

    if (code == LV_EVENT_FOCUSED) {
        lv_keyboard_set_textarea(kb, ta_);
        lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);  // Show the keyboard
        Serial.println("Keyboard shown");
    } else if (code == LV_EVENT_DEFOCUSED || code == LV_EVENT_READY) {
        lv_keyboard_set_textarea(kb, NULL);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);  // Hide the keyboard

        /* Store the frequency when the keyboard is closed */
        strncpy(frequency_buffer, lv_textarea_get_text(ta_), sizeof(frequency_buffer) - 1);
        frequency_buffer[sizeof(frequency_buffer) - 1] = '\0';  // Ensure null termination

        Serial.print("Frequency set to: ");
        lv_textarea_add_text(text_are_, "Frequency set to: ");
        Serial.println(frequency_buffer);
        lv_textarea_add_text(text_are_, frequency_buffer);
    }
}

/* Event handler for keyboard confirm and cancel buttons */
void kb_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);  // Hide the keyboard when confirm or cancel is pressed
        Serial.println("Keyboard hidden by confirm/cancel");
    }
}

/* Function to create the c1101 analysis screen */
void create_c1101_screen(void)
{
    int button_width = 90;
    int button_height = 30;

    /* Create the new screen */
    lv_obj_t * c1101AnaScreen = lv_obj_create(NULL);  // Create a new screen
    lv_scr_load(c1101AnaScreen);  // Load the new screen, make it active

    /* Disable scrolling for the screen */
    lv_obj_set_scrollbar_mode(c1101AnaScreen, LV_SCROLLBAR_MODE_OFF);
    
    /* Set the layout properties (optional but recommended for consistent positioning) */
    lv_obj_set_flex_flow(c1101AnaScreen, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(c1101AnaScreen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    topLabel_container = lv_obj_create(c1101AnaScreen);
    lv_obj_set_size(topLabel_container, 240, 25); // Set container width to 100% of the parent and height to 50px
    lv_obj_set_flex_flow(topLabel_container, LV_FLEX_FLOW_ROW); // Arrange children in a row
    lv_obj_set_flex_align(topLabel_container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER); // Align buttons in the container
    lv_obj_set_scrollbar_mode(topLabel_container, LV_SCROLLBAR_MODE_OFF);  // Disable scrollbar for this container
    lv_obj_clear_flag(topLabel_container, LV_OBJ_FLAG_SCROLLABLE);  // Make container non-scrollable
    
    fileName_container = lv_obj_create(c1101AnaScreen);
    lv_obj_set_size(fileName_container, 240, 320); // Set container width to 100% of the parent and height to 50px
    lv_obj_set_flex_flow(fileName_container, LV_FLEX_FLOW_COLUMN); // Arrange children in a row
    lv_obj_set_flex_align(fileName_container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER); // Align buttons in the container
    lv_obj_set_scrollbar_mode(fileName_container, LV_SCROLLBAR_MODE_OFF);  // Disable scrollbar for this container
    lv_obj_clear_flag(fileName_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(fileName_container, LV_OBJ_FLAG_HIDDEN);  // Hide the keyboard initially
    
    /* Create the label at the top center */
    label = lv_label_create(topLabel_container);
    lv_label_set_text(label, "RF Scanner");

    label_filename = lv_label_create(fileName_container);
    lv_label_set_text(label_filename, "File name: ");
    
    /* Create the numeric keyboard but hide it initially */
    kb = lv_keyboard_create(c1101AnaScreen);
    lv_obj_set_size(kb, LV_HOR_RES, LV_VER_RES / 2);
    lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUMBER);
    lv_obj_add_event_cb(kb, kb_event_cb, LV_EVENT_ALL, NULL);  // Attach event callback for confirm/cancel
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);  // Hide the keyboard initially
    
    /* Create the frequency input text area */
    ta_= lv_textarea_create(topLabel_container);  // Use global `ta` for frequency input
    lv_textarea_set_password_mode(ta_, true);
    lv_obj_set_size(ta_, 80, 20);
    lv_textarea_set_placeholder_text(ta_, "433.92");
    lv_textarea_set_max_length(ta_, 6);
    lv_textarea_set_one_line(ta_, true);
    lv_textarea_set_text(ta_, frequency_buffer);  // Set the initial frequency from the buffer
    lv_obj_set_style_text_color(ta_, lv_color_hex(0xFF0000), LV_PART_MAIN);

    ta_filename = lv_textarea_create(fileName_container);  // Use global `ta_filename` for file name input
    lv_obj_set_size(ta_filename, 180, 20);
    lv_textarea_set_placeholder_text(ta_filename, "");
    lv_textarea_set_max_length(ta_filename, 32);
    lv_textarea_set_one_line(ta_filename, true);
    lv_textarea_set_text(ta_filename, filename_buffer);  // Set the initial file name from the buffer

    /* Create save confirm button */
    lv_obj_t * save_confirm_btn = lv_btn_create(fileName_container);
    lv_obj_set_size(save_confirm_btn, button_width, button_height);  // Hardcode button size
    lv_obj_t * save_confirm_btn_label = lv_label_create(save_confirm_btn);
    lv_label_set_text(save_confirm_btn_label, "OK");
    lv_obj_center(save_confirm_btn_label);
    //lv_obj_add_event_cb(save_confirm_btn, saveSignalProcess, LV_EVENT_CLICKED, NULL);  // Event callback
    
    /* Create the qwerty keyboard but hide it initially */
    kb_qwert = lv_keyboard_create(fileName_container);
    lv_obj_set_size(kb_qwert, LV_HOR_RES, LV_VER_RES / 2);
    lv_keyboard_set_mode(kb_qwert, LV_KEYBOARD_MODE_TEXT_LOWER);
        lv_obj_add_event_cb(kb_qwert, EVENTS::kb_qwert_event_cb, LV_EVENT_ALL, NULL);  // Attach event callback for confirm/cancel
    lv_obj_add_flag(kb_qwert, LV_OBJ_FLAG_HIDDEN); 
        // Hide the qwerty keyboard initially
    lv_obj_add_flag(kb_qwert, LV_OBJ_FLAG_HIDDEN);

    /* Create the scrollable text area */
    text_are_ = lv_textarea_create(c1101AnaScreen);  // Store reference to text_are_ globally
    lv_obj_set_size(text_are_, 240, 180);  // Manually set width to 240px and height to 180px
    lv_obj_align(text_are_, LV_ALIGN_CENTER, 0, -20);  // Align the text area at the center with vertical offset
    lv_textarea_set_text(text_are_, "Welcome to the RF scanner.\nPlease set frequency.\n");
    lv_obj_set_scrollbar_mode(text_are_, LV_SCROLLBAR_MODE_AUTO);  // Enable scrollbar for the text area

    /* First Button Container */
    button_container1 = lv_obj_create(c1101AnaScreen);
    lv_obj_set_size(button_container1, LV_PCT(100), 40); // Set container width to 100% of the parent and height to 40px
    lv_obj_set_flex_flow(button_container1, LV_FLEX_FLOW_ROW); // Arrange children in a row
    lv_obj_set_flex_align(button_container1, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER); // Align buttons in the container
    lv_obj_set_scrollbar_mode(button_container1, LV_SCROLLBAR_MODE_OFF);  // Disable scrollbar for this container
    lv_obj_clear_flag(button_container1, LV_OBJ_FLAG_SCROLLABLE);  // Make container non-scrollable

    /* First Button: RX-On */
    lv_obj_t * btn_rx_on = lv_btn_create(button_container1);
    lv_obj_set_size(btn_rx_on, button_width, button_height);  // Hardcode button size
    lv_obj_t * label_rx_on = lv_label_create(btn_rx_on);
    lv_label_set_text(label_rx_on, "Listen");
    lv_obj_center(label_rx_on);
    lv_obj_add_event_cb(btn_rx_on, fcnProtAnaRxEn, LV_EVENT_CLICKED, NULL);  // Event callback

    /* Second Button: RX-Off */
    lv_obj_t * btn_rx_off = lv_btn_create(button_container1);
    lv_obj_set_size(btn_rx_off, button_width, button_height);  // Hardcode button size
    lv_obj_t * label_rx_off = lv_label_create(btn_rx_off);
    lv_label_set_text(label_rx_off, "Save");
    lv_obj_center(label_rx_off);
    //lv_obj_add_event_cb(btn_rx_off, saveSignal, LV_EVENT_CLICKED, NULL);  // Event callback

    /* Second Button Container */
    button_container2 = lv_obj_create(c1101AnaScreen);
    lv_obj_set_size(button_container2, LV_PCT(100), 40); // Set container width to 100% of the parent and height to 40px
    lv_obj_set_flex_flow(button_container2, LV_FLEX_FLOW_ROW); // Arrange children in a row
    lv_obj_set_flex_align(button_container2, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER); // Align buttons in the container
    lv_obj_set_scrollbar_mode(button_container2, LV_SCROLLBAR_MODE_OFF);  // Disable scrollbar for this container
    lv_obj_clear_flag(button_container2, LV_OBJ_FLAG_SCROLLABLE);  // Make container non-scrollable

    /* Third Button: Play */
    lv_obj_t * btn_play = lv_btn_create(button_container2);
    lv_obj_set_size(btn_play, button_width, button_height);  // Hardcode button size
    lv_obj_t * label_play = lv_label_create(btn_play);
    lv_label_set_text(label_play, "Play");
    lv_obj_center(label_play);
    //lv_obj_add_event_cb(btn_play, fcnProtAnaPlay, LV_EVENT_CLICKED, NULL);  // Event callback

    /* Fourth Button: Exit */
    lv_obj_t * btn_exit = lv_btn_create(button_container2);
    lv_obj_set_size(btn_exit, button_width, button_height);  // Hardcode button size
    lv_obj_t * label_exit = lv_label_create(btn_exit);
    lv_label_set_text(label_exit, "Exit");
    lv_obj_center(label_exit);
    lv_obj_add_event_cb(btn_exit, EVENTS::btn_event_subGhzTools, LV_EVENT_CLICKED, NULL);  // Event callback

    Serial.println("Created c1101 screen with two-row button layout.");
}

/* Event handler functions with Serial output for debugging */

void fcnProtAnaRxEn(lv_event_t * e) {
    if (text_are_) {
        disconnectSD();
        recievedSubGhz = false;
        digitalWrite(MICRO_SD_IO, HIGH);
        digitalWrite(CC1101_CS, LOW);
        lv_textarea_add_text(text_are_, "Initializing C1101\n");  // Use the global text_are_ reference
        Serial.println("Initializing C1101");
        lv_textarea_set_cursor_pos(text_are_, LV_TEXTAREA_CURSOR_LAST);  // Ensure cursor is at the end
        lv_obj_scroll_to_view_recursive(text_are_, LV_ANIM_ON);  // Ensure the view scrolls to the last line

        lv_event_code_t code = lv_event_get_code(e);
        if (code == LV_EVENT_CLICKED) {
            CC1101_MHZ = atof(frequency_buffer);
            if (initCC1101()) {
                lv_textarea_add_text(text_are_, "CC1101 Initialized\n");
            } else {
                lv_textarea_add_text(text_are_, "CC1101 Connection Error");
            }
            mySwitch.enableReceive(0);
            mySwitch.enableReceive(CCGDO2A);  // Receiver one
            mySwitch.enableTransmit(CCGDO0A); // Transmitter Enabler       
            ELECHOUSE_cc1101.SetRx(CC1101_MHZ);
            CC1101_TX = false;
            ProtAnaRxEn = true;
            lv_textarea_add_text(text_are_, "Status: ready\n");  // Use the global text_are_ reference

            Serial.println("C1101 initialized");
        }
    } else {
        Serial.println("Error: text_are_ is NULL");
    }
}

void ProtAnalyzerloop() {
    if (mySwitch.available() && !recievedSubGhz) {
        unsigned long receivedValue = mySwitch.getReceivedValue();
        receivedBitLength_ = mySwitch.getReceivedBitlength();
        receivedProtocol_ = mySwitch.getReceivedProtocol();
        recievedSubGhz = true;

        if (text_are_) {
            char buffer[32];
            Serial.println("Received Value:");
            Serial.println(receivedValue);
            lv_textarea_add_text(text_are_, "Value: ");
            sprintf(buffer, "%lu", receivedValue);
            lv_textarea_add_text(text_are_, buffer);
            lv_textarea_add_text(text_are_, "\n");

            Serial.println("Received Bit Length:");
            Serial.println(receivedBitLength_);
            lv_textarea_add_text(text_are_, "Length: ");
            sprintf(buffer, "%d", receivedBitLength_);
            lv_textarea_add_text(text_are_, buffer);
            lv_textarea_add_text(text_are_, "\n");

            Serial.println("Received Protocol:");
            Serial.println(receivedProtocol_);
            lv_textarea_add_text(text_are_, "Protocol: ");
            sprintf(buffer, "%d", receivedProtocol_);
            lv_textarea_add_text(text_are_, buffer);
            lv_textarea_add_text(text_are_, "\n");
        }
    }
}

