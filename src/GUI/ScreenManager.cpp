#include "ScreenManager.h"
#include "KeyboardHelper.h"
#include "ContainerHelper.h"
#include "ButtonHelper.h"
#include "globals.h"
#include "events.h"
#include "settingsButton.h"

EVENTS events;

// Singleton instance management
ScreenManager &ScreenManager::getInstance()
{
    static ScreenManager instance;
    return instance;
}

// Constructor
ScreenManager::ScreenManager()
    : ReplayScreen_(nullptr),
      text_area_(nullptr),
      freqInput_(nullptr),
      filenameInput_(nullptr),
      kb_freq_(nullptr),
      kb_qwert_(nullptr),
      fileName_container_(nullptr),
      topLabel_container_(nullptr),
      button_container1_(nullptr),
      button_container2_(nullptr),
      C1101preset_container_(nullptr),
      C1101PTK_container_(nullptr),
      C1101PTK_dropdown_(nullptr),
      C1101SYNC_container_(nullptr)
{
}

// Destructor
ScreenManager::~ScreenManager()
{
    // Cleanup if necessary
}

lv_obj_t *ScreenManager::getFreqInput()
{
    return freqInput_;
}

lv_obj_t *ScreenManager::getTextArea()
{
    return text_area_;
}

lv_obj_t *ScreenManager::getFilenameInput()
{
    return filenameInput_;
}

lv_obj_t *ScreenManager::getKeyboardFreq()
{
    return kb_freq_;
}

lv_obj_t *ScreenManager::getPresetDropdown()
{
    return C1101preset_dropdown_;
}

lv_obj_t *ScreenManager::getSyncDropdown()
{
    return C1101SYNC_dropdown_;
}

lv_obj_t *ScreenManager::getPTKDropdown()
{
    return C1101PTK_dropdown_;
}


void ScreenManager::createReplayScreen()
{
    ContainerHelper containerHelper;

    ReplayScreen_ = lv_obj_create(NULL);
    const char *CC1101_PRESET_STRINGS[] = {
        "AM650",
        "AM270",
        "FM238",
        "FM476",
        "FM95",
        "FM15k",
        "PAGER",
        "HND1",
        "HND2"};

    lv_scr_load(ReplayScreen_);
    lv_obj_set_flex_flow(ReplayScreen_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ReplayScreen_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Create top label container
    containerHelper.createContainer(&topLabel_container_, ReplayScreen_, LV_FLEX_FLOW_ROW, 35, 240);
    lv_obj_set_style_border_width(topLabel_container_, 0, LV_PART_MAIN);

    // Create keyboards
    kb_qwert_ = KeyboardHelper::createKeyboard(ReplayScreen_, LV_KEYBOARD_MODE_TEXT_LOWER);
    kb_freq_ = KeyboardHelper::createKeyboard(ReplayScreen_, LV_KEYBOARD_MODE_NUMBER);

    // Create frequency input
    containerHelper.fillTopContainer(topLabel_container_, "Frequency:", TEXT_AREA, &freqInput_, "433.92", "433.92", 10, kb_freq_, EVENTS::ta_freq_event_cb);
    lv_obj_set_size(freqInput_, 80, 30);
    lv_obj_add_event_cb(freqInput_, EVENTS::ta_freq_event_cb, LV_EVENT_FOCUSED, kb_freq_);

    // CREATE settings icon
    buttonSettings_ = ButtonHelper::createButton(topLabel_container_, "C");
    lv_obj_set_size(buttonSettings_, 30, 30);
     lv_obj_add_event_cb(buttonSettings_, [](lv_event_t * e) {
        ScreenManager::getInstance().createRFSettingsScreen(e);
    }, LV_EVENT_CLICKED, NULL);


    containerHelper.createContainer(&C1101preset_container_, ReplayScreen_, LV_FLEX_FLOW_ROW, 35, 240);
    lv_obj_set_style_border_width(C1101preset_container_, 0, LV_PART_MAIN);
    containerHelper.fillTopContainer(C1101preset_container_, "Preset:", DROPDOWN, &C1101preset_dropdown_, NULL, NULL, 12, NULL, EVENTS::ta_filename_event_cb, CC1101_PRESET_STRINGS, 9);
    lv_obj_set_size(C1101preset_dropdown_, 120, 30);
    lv_obj_add_event_cb(C1101preset_dropdown_, EVENTS::ta_preset_event_cb, LV_EVENT_ALL, NULL);

    // Create filename input container
    containerHelper.createContainer(&fileName_container_, ReplayScreen_, LV_FLEX_FLOW_COLUMN, 80, 240);
    lv_obj_add_flag(fileName_container_, LV_OBJ_FLAG_HIDDEN);

    // Create filename input
    containerHelper.fillTopContainer(fileName_container_, "File:", TEXT_AREA, &filenameInput_, "file name", "file name", 12, kb_qwert_, EVENTS::ta_filename_event_cb);
    lv_obj_set_size(filenameInput_, 200, 30);

    // Create main text area
    text_area_ = lv_textarea_create(ReplayScreen_);
    lv_obj_set_size(text_area_, 240, 140);
    lv_obj_align(text_area_, LV_ALIGN_CENTER, 0, -20);
    lv_textarea_set_text(text_area_, "Welcome to the RF scanner.\n");
    lv_obj_set_scrollbar_mode(text_area_, LV_SCROLLBAR_MODE_OFF); // Disable scrollbar

    // Create button container 1
    containerHelper.createContainer(&button_container1_, ReplayScreen_, LV_FLEX_FLOW_ROW, 35, 240);

    // Create Listen and Save buttons
    lv_obj_t *button_1 = ButtonHelper::createButton(button_container1_, "Listen");
    lv_obj_t *button_2 = ButtonHelper::createButton(button_container1_, "Save");

    // Assign event callbacks to buttons
    lv_obj_add_event_cb(button_1, EVENTS::listenRFEvent, LV_EVENT_CLICKED, text_area_); // Assign event callback for Listen
    lv_obj_add_event_cb(button_2, EVENTS::saveSignal, LV_EVENT_CLICKED, text_area_);    // Assign event callback for Save

    // Create button container 2
    containerHelper.createContainer(&button_container2_, ReplayScreen_, LV_FLEX_FLOW_ROW, 35, 240);

    // Create Play and Exit buttons
    lv_obj_t *button_3 = ButtonHelper::createButton(button_container2_, "Play");
    lv_obj_t *button_4 = ButtonHelper::createButton(button_container2_, "Exit");

    // Assign event callbacks to buttons
    lv_obj_add_event_cb(button_3, EVENTS::replayEvent, LV_EVENT_CLICKED, text_area_); // Assign event callback for Play if needed
    lv_obj_add_event_cb(button_4, EVENTS::exitReplayEvent, LV_EVENT_CLICKED, text_area_); // Assign event callback for Exit if needed
    lv_obj_add_event_cb(button_2, NULL, LV_EVENT_CLICKED, text_area_); // Assign event callback for Play if needed
}

void ScreenManager::createmainMenu()
{
    lv_obj_t *mainMenu = lv_obj_create(NULL);                                            // Create a new screen
    lv_scr_load(mainMenu);                                                               // Load the new screen, make it active
    lv_obj_t *btn_subGhz_main = lv_btn_create(mainMenu);                                 /*Add a button the current screen*/
    lv_obj_set_pos(btn_subGhz_main, 25, 10);                                             /*Set its position*/
    lv_obj_set_size(btn_subGhz_main, 150, 50);                                           /*Set its size*/
    lv_obj_add_event_cb(btn_subGhz_main, EVENTS::btn_event_subGhzTools, LV_EVENT_CLICKED, NULL); /*Assign a callback to the button*/

    lv_obj_t *label_subGhz_main = lv_label_create(btn_subGhz_main); /*Add a label to the button*/
    lv_label_set_text(label_subGhz_main, "RF SubGhz Tools");        /*Set the labels text*/
    lv_obj_center(label_subGhz_main);
}

void ScreenManager::createRFMenu()
{
    lv_obj_t *rfMenu = lv_obj_create(NULL); // Create a new screen
    lv_scr_load(rfMenu);                    // Load the new screen, make it active

    lv_obj_t *btn_playZero_menu = lv_btn_create(rfMenu);
    lv_obj_set_pos(btn_playZero_menu, 25, 10);
    lv_obj_set_size(btn_playZero_menu, 200, 50);
    // lv_obj_add_event_cb(btn_playZero_menu, btn_event_playZero_run, LV_EVENT_ALL, NULL);

    lv_obj_t *label_playZero_menu = lv_label_create(btn_playZero_menu);
    lv_label_set_text(label_playZero_menu, "Transmit Zero codes");
    lv_obj_center(label_playZero_menu);

    lv_obj_t *btn_teslaCharger_menu = lv_btn_create(rfMenu);
    lv_obj_set_pos(btn_teslaCharger_menu, 25, 70);
    lv_obj_set_size(btn_teslaCharger_menu, 200, 50);
    // lv_obj_add_event_cb(btn_teslaCharger_menu, btn_event_teslaCharger_run, LV_EVENT_ALL, NULL);

    lv_obj_t *label_teslaCharger_menu = lv_label_create(btn_teslaCharger_menu);
    lv_label_set_text(label_teslaCharger_menu, "Transmit tesla charger code");
    lv_obj_center(label_teslaCharger_menu);

    lv_obj_t *btn_c1101Alanalyzer_menu = lv_btn_create(lv_scr_act());
    lv_obj_set_pos(btn_c1101Alanalyzer_menu, 25, 130);
    lv_obj_set_size(btn_c1101Alanalyzer_menu, 200, 50);
    lv_obj_add_event_cb(btn_c1101Alanalyzer_menu, EVENTS::btn_event_Replay_run, LV_EVENT_ALL, NULL);

    lv_obj_t *label_c1101Alanalyzer_menu = lv_label_create(btn_c1101Alanalyzer_menu);
    lv_label_set_text(label_c1101Alanalyzer_menu, "subGhz rec/play");
    lv_obj_center(label_c1101Alanalyzer_menu);

    lv_obj_t *btn_c1101Scan_menu = lv_btn_create(lv_scr_act());
    lv_obj_set_pos(btn_c1101Scan_menu, 25, 190);
    lv_obj_set_size(btn_c1101Scan_menu, 200, 50);
    // lv_obj_add_event_cb(btn_c1101Scan_menu, btn_event_c1101Scanner_run, LV_EVENT_ALL, NULL);

    lv_obj_t *label_c1101Scan_menu = lv_label_create(btn_c1101Scan_menu);
    lv_label_set_text(label_c1101Scan_menu, "RF Scanner");
    lv_obj_center(label_c1101Scan_menu);

    lv_obj_t *btn_c1101Others_menu = lv_btn_create(lv_scr_act());
    lv_obj_set_pos(btn_c1101Others_menu, 25, 250);
    lv_obj_set_size(btn_c1101Others_menu, 200, 50);
    // lv_obj_add_event_cb(btn_c1101Others_menu, btn_event_playRecorded_run, LV_EVENT_ALL, NULL);

    lv_obj_t *label_c1101Others_menu = lv_label_create(btn_c1101Others_menu);
    lv_label_set_text(label_c1101Others_menu, "Play Recorded");
    lv_obj_center(label_c1101Others_menu);
}

void ScreenManager::createRFSettingsScreen(lv_event_t * e)
{
    ContainerHelper containerHelper;



    const char *SYNC_STRINGS[] = {
        "No sync",
        "16 bit",
        "16/16 bit",
        "30/32 bit",
        "no sync - carrier-sense +",
        "15/16 - carrier-sense +",
        "16/16 - carrier-sense +",
        "30/32 - carrier-sense +"};

    const char *PTK_STRINGS[] = {
        "Normal",
        "Sync Serial",
        "Send random data",
        "Async Serial"};

    lv_obj_t *RFSettingsScreen = lv_obj_create(NULL);

    lv_scr_load(RFSettingsScreen);
    lv_obj_set_flex_flow(RFSettingsScreen, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(RFSettingsScreen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Create top label container
    containerHelper.createContainer(&C1101SYNC_container_, RFSettingsScreen, LV_FLEX_FLOW_ROW, 35, 240);
    containerHelper.createContainer(&C1101PTK_container_, RFSettingsScreen, LV_FLEX_FLOW_ROW, 35, 240);
    containerHelper.createContainer(&button_container_settings, RFSettingsScreen, LV_FLEX_FLOW_ROW, 35, 240);

    containerHelper.fillTopContainer(C1101SYNC_container_, "Sync:", DROPDOWN, &C1101SYNC_dropdown_, NULL, NULL, 12, NULL, EVENTS::ta_filename_event_cb, SYNC_STRINGS, 8);
    containerHelper.fillTopContainer(C1101PTK_container_, "PTK:", DROPDOWN, &C1101PTK_dropdown_, NULL, NULL, 12, NULL, EVENTS::ta_filename_event_cb, PTK_STRINGS, 4);
    lv_obj_set_size(C1101SYNC_dropdown_, 165, 30);
    lv_obj_set_size(C1101PTK_dropdown_, 165, 30);

    lv_obj_t *SaveSetting = ButtonHelper::createButton(button_container_settings, "Save");
    lv_obj_t *CancelSettings = ButtonHelper::createButton(button_container_settings, "Cancel");

    lv_obj_add_event_cb(SaveSetting, EVENTS::saveRFSettingEvent, LV_EVENT_CLICKED, NULL); // Assign event callback for Play if needed
    lv_obj_add_event_cb(CancelSettings, EVENTS::cancelRFSettingEvent, LV_EVENT_CLICKED, NULL); // Assign event callback for Exit if needed
}