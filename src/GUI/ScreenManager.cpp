#include "ScreenManager.h"
#include "KeyboardHelper.h"
#include "ContainerHelper.h"
#include "ButtonHelper.h"
#include "globals.h"
#include "events.h"
//#include "settingsButton.h"
#include "FS.h"
#//include "fileBrowserHelper.h"
#include "modules/ETC/SDcard.h"
#include "modules/dataProcessing/SubGHzParser.h"
#include "modules/dataProcessing/dataProcessing.h"
#include "SD.h"
#include "XPT2046_Bitbang.h"
#include "lv_fs_if.h"

#define MAX_PATH_LENGTH 256

EVENTS events;



char** file_list; 
int file_count = 0;
lv_obj_t* list;

const char* pathBUffer;

lv_obj_t* previous_screen = NULL; 

static char selected_file_path[LV_FS_MAX_PATH_LENGTH];
static char selected_file_name[LV_FS_MAX_FN_LENGTH];

ScreenManager &ScreenManager::getInstance()
{
    static ScreenManager instance;
    return instance;
}

ScreenManager::ScreenManager()
    : ReplayScreen_(nullptr),
      SourAppleScreen_(nullptr), 
      BTSpamScreen_(nullptr),   
      text_area_(nullptr),
      freqInput_(nullptr),
      kb_freq_(nullptr),
      kb_qwert_(nullptr),
      fileName_container_(nullptr),
      topLabel_container_(nullptr),
      browserButton_container_(nullptr),
      browserButton2_container_(nullptr),
      button_container1_(nullptr),
      button_container2_(nullptr),
      C1101preset_container_(nullptr),
      C1101PTK_container_(nullptr),
      C1101SYNC_container_(nullptr),
      topLabel_RCSwitchMethod_container_(nullptr),
      secondLabel_container_(nullptr),
      text_area_replay(nullptr),
      button_container_RCSwitchMethod2_(nullptr),
      button_container_RCSwitchMethod1_(nullptr),
      brute_dropdown_(nullptr)
  
{
}


ScreenManager::~ScreenManager()
{

}

lv_obj_t *ScreenManager::getFreqInput()
{
    return freqInput_;
}

lv_obj_t *ScreenManager::getTextArea()
{
    return text_area_replay; 
}

lv_obj_t *ScreenManager::getTextAreaRCSwitchMethod()
{
    return text_area_;
}

lv_obj_t *ScreenManager::getTextAreaSourAple()
{
    return text_area_SourApple;
}

lv_obj_t *ScreenManager::getTextAreaBTSpam()
{
    return text_area_BTSpam;
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


lv_obj_t *ScreenManager::getTextAreaBrute(){
    return text_area__BruteForce;
}

lv_obj_t *ScreenManager::getBruteDropdown(){
    return brute_dropdown_;
}

void ScreenManager::createReplayScreen() {
    ContainerHelper containerHelper;    

    ReplayScreen_ = lv_obj_create(NULL);

    lv_scr_load(ReplayScreen_);

    lv_obj_delete(previous_screen);
    
    previous_screen = ReplayScreen_;
    lv_obj_set_flex_flow(ReplayScreen_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ReplayScreen_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);


    containerHelper.createContainer(&topLabel_container_, ReplayScreen_, LV_FLEX_FLOW_ROW, 35, 240);
    lv_obj_set_style_border_width(topLabel_container_, 0, LV_PART_MAIN);


    kb_qwert_ = KeyboardHelper::createKeyboard(ReplayScreen_, LV_KEYBOARD_MODE_TEXT_LOWER);
    kb_freq_ = KeyboardHelper::createKeyboard(ReplayScreen_, LV_KEYBOARD_MODE_NUMBER);
    lv_keyboard_set_textarea(kb_freq_, freqInput_);


    containerHelper.fillTopContainer(topLabel_container_, "Mhz:  ", TEXT_AREA, &freqInput_, "433.92", "433.92", 10, NULL, NULL);
    lv_obj_set_size(freqInput_, 70, 30);                   
    lv_obj_add_event_cb(freqInput_, EVENTS::ta_freq_event_cb, LV_EVENT_ALL, kb_freq_);


    containerHelper.createContainer(&secondLabel_container_, ReplayScreen_, LV_FLEX_FLOW_ROW, 35, 240);
    lv_obj_set_style_border_width(secondLabel_container_, 0, LV_PART_MAIN);

   C1101preset_dropdown_ = lv_dropdown_create(secondLabel_container_);
    lv_dropdown_set_options(C1101preset_dropdown_, "AM650\n"
                                "AM270\n"
                                "FM238\n"
                                "FM476\n"
                                "FM95\n"
                                "FM15k\n"
                                "PAGER\n"
                                "HND1\n"
                                "HND2\n"
                                );

    lv_obj_add_event_cb(C1101preset_dropdown_, EVENTS::ta_preset_event_cb, LV_EVENT_VALUE_CHANGED, C1101preset_dropdown_);

    C1101type_dropdown_ = lv_dropdown_create(secondLabel_container_);
    lv_dropdown_set_options(C1101type_dropdown_, "Raw\n"
                                "Analyze\n"

                                );

    lv_obj_add_event_cb(C1101type_dropdown_, EVENTS::ta_rf_type_event_cb, LV_EVENT_VALUE_CHANGED, C1101type_dropdown_);


    // Create main text area
    text_area_replay = lv_textarea_create(ReplayScreen_);
    lv_obj_set_size(text_area_replay, 240, 140);
    lv_obj_align(text_area_replay, LV_ALIGN_CENTER, 0, -20);
    lv_textarea_set_text(text_area_replay, "RAW protocol tool.\nDuring radio operation device may not respond.");
    lv_obj_set_scrollbar_mode(text_area_replay, LV_SCROLLBAR_MODE_OFF); 
    lv_textarea_set_cursor_click_pos(text_area_replay, false);


    containerHelper.createContainer(&button_container_RCSwitchMethod1_, ReplayScreen_, LV_FLEX_FLOW_ROW, 35, 240);


    lv_obj_t *listenButton = ButtonHelper::createButton(button_container_RCSwitchMethod1_, "Listen");
    lv_obj_add_event_cb(listenButton, EVENTS::btn_event_RAW_REC_run, LV_EVENT_CLICKED, NULL); 

    lv_obj_t *saveButton = ButtonHelper::createButton(button_container_RCSwitchMethod1_, "Save");
    lv_obj_add_event_cb(saveButton, EVENTS::save_RF_to_sd_event, LV_EVENT_CLICKED, NULL); 


    containerHelper.createContainer(&button_container_RCSwitchMethod2_, ReplayScreen_, LV_FLEX_FLOW_ROW, 35, 240);


    lv_obj_t *playButton = ButtonHelper::createButton(button_container_RCSwitchMethod2_, "Play");
    lv_obj_t *exitButton = ButtonHelper::createButton(button_container_RCSwitchMethod2_, "Exit");


    lv_obj_add_event_cb(playButton, EVENTS::sendCapturedEvent, LV_EVENT_CLICKED, NULL); 
    lv_obj_add_event_cb(exitButton, EVENTS::exitReplayEvent, LV_EVENT_CLICKED, NULL); 
}

void ScreenManager::createBruteForceScreen() {
    ContainerHelper containerHelper;

    BruteForceScreen_ = lv_obj_create(NULL);
    lv_scr_load(BruteForceScreen_);
    lv_obj_delete(previous_screen);
    previous_screen = BruteForceScreen_;
    lv_obj_set_flex_flow(BruteForceScreen_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(BruteForceScreen_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);


    containerHelper.createContainer(&topLabel_BruteForce_container_, BruteForceScreen_, LV_FLEX_FLOW_ROW, 35, 240);
    lv_obj_set_style_border_width(BruteForceScreen_, 0, LV_PART_MAIN);

    kb_freq_ = KeyboardHelper::createKeyboard(BruteForceScreen_, LV_KEYBOARD_MODE_NUMBER);
    lv_keyboard_set_textarea(kb_freq_, freqInput_);


    containerHelper.fillTopContainer(topLabel_BruteForce_container_, "Mhz:  ", TEXT_AREA, &freqInput_, "433.92", "433.92", 10, NULL, NULL);
    lv_obj_set_size(freqInput_, 70, 30);                   
    lv_obj_add_event_cb(freqInput_, EVENTS::ta_freq_event_cb, LV_EVENT_ALL, kb_freq_);


    containerHelper.createContainer(&secondLabel_BruteForce_container_, BruteForceScreen_, LV_FLEX_FLOW_ROW, 35, 240);
    lv_obj_set_style_border_width(secondLabel_BruteForce_container_, 0, LV_PART_MAIN);

    brute_dropdown_ = lv_dropdown_create(secondLabel_BruteForce_container_);
    lv_dropdown_set_options(brute_dropdown_, "Czech Bells\n"
                                "Empty\n"
                                );

    lv_obj_add_event_cb(brute_dropdown_, EVENTS::ta_preset_event_cb, LV_EVENT_VALUE_CHANGED, brute_dropdown_);


    // Create main text area
    text_area__BruteForce = lv_textarea_create(BruteForceScreen_);
    lv_obj_set_size(text_area__BruteForce, 240, 140);
    lv_obj_align(text_area__BruteForce, LV_ALIGN_CENTER, 0, -20);
    lv_textarea_set_text(text_area__BruteForce, "SubGhz BruteForce.\nSet/get frequency,type, and start attack.\nDuring radio operation device may not respond.");
    lv_obj_set_scrollbar_mode(text_area__BruteForce, LV_SCROLLBAR_MODE_OFF); 
    lv_textarea_set_cursor_click_pos(text_area__BruteForce, false);

    containerHelper.createContainer(&button_container_BruteForce1_, BruteForceScreen_, LV_FLEX_FLOW_ROW, 35, 240);

    lv_obj_t *listenButton = ButtonHelper::createButton(button_container_BruteForce1_, "Start");
    lv_obj_add_event_cb(listenButton, EVENTS::btn_event_brute_run, LV_EVENT_CLICKED, NULL); 

    lv_obj_t *saveButton = ButtonHelper::createButton(button_container_BruteForce1_, "Pause");
    lv_obj_add_event_cb(saveButton, EVENTS::save_RF_to_sd_event, LV_EVENT_CLICKED, NULL);

    containerHelper.createContainer(&button_container_BruteForce2_, BruteForceScreen_, LV_FLEX_FLOW_ROW, 35, 240);

    lv_obj_t *playButton = ButtonHelper::createButton(button_container_BruteForce2_, "Save");
    lv_obj_t *exitButton = ButtonHelper::createButton(button_container_BruteForce2_, "Exit");


    lv_obj_add_event_cb(playButton, EVENTS::sendCapturedEvent, LV_EVENT_CLICKED, NULL); 
    lv_obj_add_event_cb(exitButton, EVENTS::exitReplayEvent, LV_EVENT_CLICKED, NULL); 
}

void ScreenManager::createSourAppleScreen() {
    ContainerHelper containerHelper;

    SourAppleScreen_ = lv_obj_create(NULL);

    lv_scr_load(SourAppleScreen_);
    lv_obj_delete(previous_screen);
    previous_screen = SourAppleScreen_;
    lv_obj_set_flex_flow(SourAppleScreen_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(SourAppleScreen_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * topLabel_container_;
    containerHelper.createContainer(&topLabel_container_, SourAppleScreen_, LV_FLEX_FLOW_ROW, 35, 240);
    lv_obj_set_style_border_width(topLabel_container_, 0, LV_PART_MAIN);

    // Create main text area
    text_area_SourApple = lv_textarea_create(SourAppleScreen_);
    lv_obj_set_size(text_area_SourApple, 240, 140);
    lv_obj_align(text_area_SourApple, LV_ALIGN_CENTER, 0, -20);
    lv_textarea_set_text(text_area_SourApple, "Sour Apple\nWill spam BLE devices\nMay cause crash of Apple devices");
    lv_obj_set_scrollbar_mode(text_area_SourApple, LV_SCROLLBAR_MODE_OFF); 
    lv_textarea_set_cursor_click_pos(text_area_SourApple, false);

    lv_obj_t *buttonContainer;
    containerHelper.createContainer(&buttonContainer, SourAppleScreen_, LV_FLEX_FLOW_ROW, 35, 240);


    lv_obj_t *listenButton = ButtonHelper::createButton(buttonContainer, "Start");
    lv_obj_add_event_cb(listenButton, EVENTS::btn_event_SourApple_Start, LV_EVENT_CLICKED, NULL); 

    lv_obj_t *saveButton = ButtonHelper::createButton(buttonContainer, "Stop");
    lv_obj_add_event_cb(saveButton, EVENTS::btn_event_SourApple_Stop, LV_EVENT_CLICKED, NULL); 
}

void ScreenManager::createBTSPamScreen() {
    ContainerHelper containerHelper;

    BTSpamScreen_ = lv_obj_create(NULL);
    lv_scr_load(BTSpamScreen_);
    lv_obj_delete(previous_screen);
    previous_screen = BTSpamScreen_;
    lv_obj_set_flex_flow(BTSpamScreen_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(BTSpamScreen_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * topLabel_container_;
    containerHelper.createContainer(&topLabel_container_, BTSpamScreen_, LV_FLEX_FLOW_ROW, 35, 240);
    lv_obj_set_style_border_width(topLabel_container_, 0, LV_PART_MAIN);

    // Create main text area
    text_area_BTSpam = lv_textarea_create(BTSpamScreen_);
    lv_obj_set_size(text_area_BTSpam, 240, 140);
    lv_obj_align(text_area_BTSpam, LV_ALIGN_CENTER, 0, -20);
    lv_textarea_set_text(text_area_BTSpam, "Will spam BLE devices\nMay cause crash of some");
    lv_obj_set_scrollbar_mode(text_area_BTSpam, LV_SCROLLBAR_MODE_OFF); 
    lv_textarea_set_cursor_click_pos(text_area_BTSpam, false);

    lv_obj_t *buttonContainer;
    containerHelper.createContainer(&buttonContainer, BTSpamScreen_, LV_FLEX_FLOW_ROW_WRAP, 120, 240);
    lv_obj_set_flex_align(buttonContainer, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    int applle = 0;
    int Microsoft = 1;
    int Samsung = 2;
    int Android = 3;
    int ALL = 4;

    lv_obj_t *startButton = ButtonHelper::createButton(buttonContainer, "Start Apple");
    lv_obj_add_event_cb(startButton, EVENTS::btn_event_BTSpam_Start, LV_EVENT_CLICKED, &applle); 

    lv_obj_t *startButton1 = ButtonHelper::createButton(buttonContainer, "Start Microsoft");
    lv_obj_add_event_cb(startButton1, EVENTS::btn_event_BTSpam_Start, LV_EVENT_CLICKED, &Microsoft); 

    lv_obj_t *startButton2 = ButtonHelper::createButton(buttonContainer, "Start Samsung");
    lv_obj_add_event_cb(startButton2, EVENTS::btn_event_BTSpam_Start, LV_EVENT_CLICKED, &Samsung); 

    lv_obj_t *startButton3 = ButtonHelper::createButton(buttonContainer, "Start Android");
    lv_obj_add_event_cb(startButton3, EVENTS::btn_event_BTSpam_Start, LV_EVENT_CLICKED, &Android); 

    lv_obj_t *startButton4 = ButtonHelper::createButton(buttonContainer, "Start ALL");
    lv_obj_add_event_cb(startButton4, EVENTS::btn_event_BTSpam_Start, LV_EVENT_CLICKED, &ALL); 

    lv_obj_t *saveButton = ButtonHelper::createButton(buttonContainer, "Stop");
    lv_obj_add_event_cb(saveButton, EVENTS::btn_event_SourApple_Stop, LV_EVENT_CLICKED, NULL); 
}




void ScreenManager::createmainMenu()
{
    lv_obj_t *mainMenu = lv_obj_create(NULL);                                        
    lv_scr_load(mainMenu);    
    previous_screen = mainMenu;                                                        
    lv_obj_t *btn_subGhz_main = lv_btn_create(mainMenu);                                
    lv_obj_set_pos(btn_subGhz_main, 25, 10);                                             
    lv_obj_set_size(btn_subGhz_main, 150, 50);                                           
    lv_obj_add_event_cb(btn_subGhz_main, EVENTS::btn_event_subGhzTools, LV_EVENT_CLICKED, NULL); 

    lv_obj_t *label_subGhz_main = lv_label_create(btn_subGhz_main);
    lv_label_set_text(label_subGhz_main, "RF SubGhz Tools");       
    lv_obj_center(label_subGhz_main);

    lv_obj_t *btn_BT_main = lv_btn_create(mainMenu);                                
    lv_obj_set_pos(btn_BT_main, 25, 70);                                            
    lv_obj_set_size(btn_BT_main, 150, 50);                                          
    lv_obj_add_event_cb(btn_BT_main, EVENTS::btn_event_BTTools, LV_EVENT_CLICKED, NULL); 

    lv_obj_t *label_BT_main = lv_label_create(btn_BT_main);
    lv_label_set_text(label_BT_main, "BlueTooth");       
    lv_obj_center(label_BT_main);


}

void ScreenManager::createBTMenu()
{
    lv_obj_t *BTMenu = lv_obj_create(NULL);                                            // Create a new screen
    lv_scr_load(BTMenu);    
    lv_obj_delete(previous_screen);
    previous_screen = BTMenu;                                                           // Load the new screen, make it active
    lv_obj_t *btn_BT_main = lv_btn_create(BTMenu);                                 /*Add a button the current screen*/
    lv_obj_set_pos(btn_BT_main, 25, 10);                                             /*Set its position*/
    lv_obj_set_size(btn_BT_main, 150, 50);                                           /*Set its size*/
    lv_obj_add_event_cb(btn_BT_main, EVENTS::btn_event_SourApple, LV_EVENT_CLICKED, NULL); /*Assign a callback to the button*/

    lv_obj_t *label_BLESPAM_main = lv_label_create(btn_BT_main); /*Add a label to the button*/
    lv_label_set_text(label_BLESPAM_main, "Sour Apple");        /*Set the labels text*/
    lv_obj_center(label_BLESPAM_main);

    lv_obj_t *btn_BT_maspam = lv_btn_create(BTMenu);                                 /*Add a button the current screen*/
    lv_obj_set_pos(btn_BT_maspam, 25, 70);                                             /*Set its position*/
    lv_obj_set_size(btn_BT_maspam, 150, 50);
    lv_obj_add_event_cb(btn_BT_maspam, EVENTS::btn_event_BTSpam, LV_EVENT_CLICKED, NULL); /*Assign a callback to the button*/

    lv_obj_t *label_SourApple_main = lv_label_create(btn_BT_maspam); /*Add a label to the button*/
    lv_label_set_text(label_SourApple_main, "BLE spam");        /*Set the labels text*/
    lv_obj_center(label_SourApple_main);




    lv_obj_t *btn_c1101Others_menu = lv_btn_create(lv_scr_act());
    lv_obj_set_pos(btn_c1101Others_menu, 25, 250);
    lv_obj_set_size(btn_c1101Others_menu, 200, 50);

    lv_obj_t *label_c1101Others_menu = lv_label_create(btn_c1101Others_menu);
    lv_label_set_text(label_c1101Others_menu, "Back");
    lv_obj_center(label_c1101Others_menu);
    lv_obj_add_event_cb(btn_c1101Others_menu, EVENTS::btn_event_mainMenu_run, LV_EVENT_CLICKED, NULL);

}

void ScreenManager::createRFMenu()
{
    lv_obj_t *rfMenu = lv_obj_create(NULL);
    lv_scr_load(rfMenu);                   
    lv_obj_delete(previous_screen);
    previous_screen = rfMenu;
    lv_obj_t *btn_playZero_menu = lv_btn_create(rfMenu);
    lv_obj_set_pos(btn_playZero_menu, 25, 10);
    lv_obj_set_size(btn_playZero_menu, 200, 50);
    lv_obj_add_event_cb(btn_playZero_menu, EVENTS::btn_event_playZero_run, LV_EVENT_ALL, NULL);

    lv_obj_t *label_playZero_menu = lv_label_create(btn_playZero_menu);
    lv_label_set_text(label_playZero_menu, "Transmit saved codes");
    lv_obj_center(label_playZero_menu);

    lv_obj_t *btn_teslaCharger_menu = lv_btn_create(rfMenu);
    lv_obj_set_pos(btn_teslaCharger_menu, 25, 70);
    lv_obj_set_size(btn_teslaCharger_menu, 200, 50);
    lv_obj_add_event_cb(btn_teslaCharger_menu, EVENTS::btn_event_teslaCharger_run, LV_EVENT_ALL, NULL);

    lv_obj_t *label_teslaCharger_menu = lv_label_create(btn_teslaCharger_menu);
    lv_label_set_text(label_teslaCharger_menu, "Transmit tesla charger code");
    lv_obj_center(label_teslaCharger_menu);

    lv_obj_t *btn_c1101Alanalyzer_menu = lv_btn_create(lv_scr_act());
    lv_obj_set_pos(btn_c1101Alanalyzer_menu, 25, 130);
    lv_obj_set_size(btn_c1101Alanalyzer_menu, 200, 50);
    lv_obj_add_event_cb(btn_c1101Alanalyzer_menu, EVENTS::btn_event_Replay_run, LV_EVENT_ALL, NULL);

    lv_obj_t *label_c1101Alanalyzer_menu = lv_label_create(btn_c1101Alanalyzer_menu);
    lv_label_set_text(label_c1101Alanalyzer_menu, "rec/play");
    lv_obj_center(label_c1101Alanalyzer_menu);

        lv_obj_t *btn_SubGhzBruteForce_menu = lv_btn_create(lv_scr_act());
    lv_obj_set_pos(btn_SubGhzBruteForce_menu, 25, 190);
    lv_obj_set_size(btn_SubGhzBruteForce_menu, 200, 50);
    lv_obj_add_event_cb(btn_SubGhzBruteForce_menu, EVENTS::btn_event_BruteForce_run, LV_EVENT_ALL, NULL);

    lv_obj_t *label_SubGhzBruteForce_menu = lv_label_create(btn_SubGhzBruteForce_menu);
    lv_label_set_text(label_SubGhzBruteForce_menu, "Brute Force");
    lv_obj_center(label_SubGhzBruteForce_menu);

    lv_obj_t *btn_c1101Others_menu = lv_btn_create(lv_scr_act());
    lv_obj_set_pos(btn_c1101Others_menu, 25, 250);
    lv_obj_set_size(btn_c1101Others_menu, 200, 50);

    lv_obj_t *label_c1101Others_menu = lv_label_create(btn_c1101Others_menu);
    lv_label_set_text(label_c1101Others_menu, "Back");
    lv_obj_center(label_c1101Others_menu);
    lv_obj_add_event_cb(btn_c1101Others_menu, EVENTS::btn_event_mainMenu_run, LV_EVENT_CLICKED, NULL);

}

void ScreenManager::createFileExplorerScreen()
{
    // Create a new screen and load it
    lv_obj_t *fileExplorerScreen = lv_obj_create(NULL);
    lv_scr_load(fileExplorerScreen);

    // Create the file explorer on the new screen
    lv_obj_t *file_explorer = lv_file_explorer_create(fileExplorerScreen);

    // Open the root directory (drive letter 'S')
    lv_file_explorer_open_dir(file_explorer, "S:/");

    // Optional: Sorting dropdown menu
    lv_obj_t *fe_header_obj = lv_file_explorer_get_header(file_explorer);

    static const char *opts = "NONE\nKIND";
    lv_obj_t *dd = lv_dropdown_create(fe_header_obj);
    lv_obj_set_style_radius(dd, 4, 0);
    lv_obj_set_style_pad_all(dd, 0, 0);
    lv_obj_set_size(dd, LV_PCT(30), LV_SIZE_CONTENT);
    lv_dropdown_set_options_static(dd, opts);
    lv_obj_align(dd, LV_ALIGN_RIGHT_MID, 0, 0);

    // Attach event handler for dropdown selection to control sorting
  //  lv_obj_add_event_cb(dd, dd_event_handler, LV_EVENT_VALUE_CHANGED, file_explorer);
}
