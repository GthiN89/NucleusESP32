#include "ScreenManager.h"
#include "KeyboardHelper.h"
#include "ContainerHelper.h"
#include "ButtonHelper.h"
#include "events.h"
//#include "FS.h"
#include "modules/dataProcessing/SubGHzParser.h"
#include "modules/dataProcessing/dataProcessing.h"
#include "XPT2046_Bitbang.h"
#include "modules/nfc/nfc.h"
#include "modules/IR/ir.h"
#include "GUI/spinbox.h"

#define MAX_PATH_LENGTH 256

EVENTS events;



char** file_list; 
int file_count = 0;
lv_obj_t* list;

const char* pathBUffer;

lv_obj_t* previous_screen = NULL; 


ScreenManager &ScreenManager::getInstance()
{
    static ScreenManager instance;
    return instance;
}

ScreenManager::ScreenManager()
    : ReplayScreen_(nullptr),
      IRRecScreen_(nullptr),
      SubGHzCustomScreen_(nullptr),
      SourAppleScreen_(nullptr), 
      BTSpamScreen_(nullptr),
      fileExplorerScreen(nullptr),   
      text_area_(nullptr),
      customPreset(nullptr),
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
      button_container_IR_REC1_(nullptr),
      button_container_IR_REC2_(nullptr),
      quareLine_container(nullptr),
      detect_dropdown_(nullptr),
      teslaScreen_(nullptr),
      BruteScreen_(nullptr),
      BruteCounterLabel(nullptr)
      
  
{
}


ScreenManager::~ScreenManager()
{

}

lv_obj_t *ScreenManager::getFreqInput()
{
    return customPreset;
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


lv_obj_t *ScreenManager::getdetectLabel(){
    return detectLabel;
}

lv_obj_t *ScreenManager::getDetectDropdown(){
    return detect_dropdown_;
}

lv_obj_t *ScreenManager::getSquareLineContainer(){
    return quareLine_container;
}

void ScreenManager::createReplayScreen() {
    ContainerHelper containerHelper;    

    ReplayScreen_ = lv_obj_create(NULL);

    lv_scr_load(ReplayScreen_);

  //  lv_obj_delete(previous_screen);
    
    previous_screen = ReplayScreen_;
    lv_obj_set_flex_flow(ReplayScreen_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ReplayScreen_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);


    lv_obj_set_style_pad_column(ReplayScreen_, 1, LV_PART_MAIN);
    lv_obj_set_style_pad_row(ReplayScreen_, 1, LV_PART_MAIN);

    containerHelper.createContainer(&topLabel_container_, ReplayScreen_, LV_FLEX_FLOW_ROW, 35, 240);
    lv_obj_set_style_border_width(topLabel_container_, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_left(topLabel_container_, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_right(topLabel_container_, 0, LV_PART_MAIN);
    containerHelper.fillTopContainer(topLabel_container_, "MHz:  ", TEXT_AREA, &customPreset, "433.92", "433.92", 10, NULL, NULL);
    lv_obj_set_size(customPreset, 90, 30);                   
    lv_obj_add_event_cb(customPreset, EVENTS::ta_freq_event_cb, LV_EVENT_ALL, (void *)"freq");

    containerHelper.createContainer(&secondLabel_container_, ReplayScreen_, LV_FLEX_FLOW_ROW, 35, 240);
    lv_obj_set_style_border_width(secondLabel_container_, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_left(secondLabel_container_, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_right(secondLabel_container_, 0, LV_PART_MAIN);


   C1101preset_dropdown_ = lv_dropdown_create(secondLabel_container_);
    lv_dropdown_set_options(C1101preset_dropdown_, "AM650\n"
                                "AM270\n"
                                "FM238\n"
                                "FM476\n"
                                "FM95\n"
                                "FSK12k\n"
                                "FM15k\n"
                                "FSK25k\n"
                                "FSK31k\n"
                                "PAGER\n"
                                "HND1\n"
                                "HND2\n"
                                "CSTM"
                                );

    lv_obj_add_event_cb(C1101preset_dropdown_, EVENTS::ta_preset_event_cb, LV_EVENT_VALUE_CHANGED, C1101preset_dropdown_);
    lv_obj_set_width(C1101preset_dropdown_, 120);  
    C1101type_dropdown_ = lv_dropdown_create(secondLabel_container_);
    lv_dropdown_set_options(C1101type_dropdown_, "Raw\n"
                                "RC-Switch\n"
                                "ESPiLight\n"
                                "RTL_433\n"
                                );
    lv_obj_set_width(C1101type_dropdown_, 120);  
    lv_obj_add_event_cb(C1101type_dropdown_, EVENTS::ta_rf_type_event_cb, LV_EVENT_VALUE_CHANGED, C1101type_dropdown_);


    // Create main text area
    text_area_replay = lv_textarea_create(ReplayScreen_);
    lv_obj_set_size(text_area_replay, 240, 120);
    lv_obj_align(text_area_replay, LV_ALIGN_CENTER, 0, -20);
    lv_textarea_set_text(text_area_replay, "RAW protocol tool.\nPress listen to start.");
    lv_obj_set_scrollbar_mode(text_area_replay, LV_SCROLLBAR_MODE_OFF); 
    lv_textarea_set_cursor_click_pos(text_area_replay, false);
    containerHelper.createContainer(&quareLine_container, ReplayScreen_, LV_FLEX_FLOW_ROW, 50, 240);

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

void ScreenManager::createCustomSubghzScreen(){
    ContainerHelper containerHelper;  
    CC1101_CLASS CC1101SM;
    
    SubGHzCustomScreen_ = new SubGHzScreen;
    SubGHzCustomScreen_->screen = lv_obj_create(NULL);

    
    lv_scr_load(SubGHzCustomScreen_->screen);
    lv_obj_delete(previous_screen);
    
    previous_screen = SubGHzCustomScreen_->screen;
    lv_obj_set_flex_flow(SubGHzCustomScreen_->screen, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(SubGHzCustomScreen_->screen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);


    lv_obj_set_style_pad_column(SubGHzCustomScreen_->screen, 1, LV_PART_MAIN);
    lv_obj_set_style_pad_row(SubGHzCustomScreen_->screen, 1, LV_PART_MAIN);
 
    text_area_SubGHzCustom = lv_textarea_create(SubGHzCustomScreen_->screen);
    lv_obj_set_size(text_area_SubGHzCustom, 240, 75);
    lv_obj_align(text_area_SubGHzCustom, LV_ALIGN_CENTER, 0, -20);
    lv_textarea_set_text(text_area_SubGHzCustom, "Custom SubGhz tool.\nHelp On Long Touch\nIllegal settings can kill the CC1101.");
    lv_obj_set_scrollbar_mode(text_area_SubGHzCustom, LV_SCROLLBAR_MODE_OFF); 
    lv_textarea_set_cursor_click_pos(text_area_SubGHzCustom, false);

    lv_obj_t *settingContainer;
    containerHelper.createContainer(&settingContainer, SubGHzCustomScreen_->screen, LV_FLEX_FLOW_ROW, 25, 240);
    lv_obj_t* label1 = lv_label_create(settingContainer);	
    lv_label_set_text(label1, "Bandwith ");
    Serial.println("Bandwith ");
    Serial.println(CC1101SM.CC1101_RX_BW);
    int32_t valueBW = CC1101SM.CC1101_RX_BW;
    SubGHzCustomScreen_->spinbox = SpinBox::createSpinbox(settingContainer, 0, valueBW, 812, 10, EVENTS::CustomSubGhzHelp_CB, "Bandwith");

    lv_obj_t *settingContainer1;
    containerHelper.createContainer(&settingContainer1, SubGHzCustomScreen_->screen, LV_FLEX_FLOW_ROW, 25, 240);
    lv_obj_t* label2 = lv_label_create(settingContainer1);	
    lv_label_set_text(label2, "Threshold ");
    SubGHzCustomScreen_->spinbox1 = SpinBox::createSpinbox(settingContainer1, -50, -80 , 80, 10, EVENTS::CustomSubGhzHelp_CB, "Threshold");
 

    lv_obj_t *settingContainer2;
    containerHelper.createContainer(&settingContainer2, SubGHzCustomScreen_->screen, LV_FLEX_FLOW_ROW, 25, 240);
    lv_obj_t* label3 = lv_label_create(settingContainer2);	
    lv_label_set_text(label3, "SyncMode ");
    Serial.println("SyncMode ");    
    Serial.println(CC1101SM.CC1101_SYNC);
     int32_t valueSync = CC1101SM.CC1101_SYNC;
    SubGHzCustomScreen_->spinbox2 = SpinBox::createSpinbox(settingContainer2, valueSync, 0, 5, 1, EVENTS::CustomSubGhzHelp_CB, "SYNC");
   
    lv_obj_add_event_cb(settingContainer2, EVENTS::CustomSubGhzHelp_CB, LV_EVENT_LONG_PRESSED, (void *)"SYNC");

    lv_obj_t *settingContainer3;
    containerHelper.createContainer(&settingContainer3, SubGHzCustomScreen_->screen, LV_FLEX_FLOW_ROW, 25, 240);
    lv_obj_t* label4 = lv_label_create(settingContainer3);	
    lv_label_set_text(label4, "PktFormat ");
    Serial.println("PktFormat ");
    Serial.println(CC1101SM.CC1101_PKT_FORMAT);
    int32_t valuePTK = CC1101SM.CC1101_PKT_FORMAT;
    SubGHzCustomScreen_->spinbox3 = SpinBox::createSpinbox(settingContainer3, valuePTK, 0, 3, 1, EVENTS::CustomSubGhzHelp_CB, "PktFormat");
    lv_obj_add_event_cb(settingContainer3, EVENTS::CustomSubGhzHelp_CB, LV_EVENT_LONG_PRESSED, (void *)"PktFormat");

    lv_obj_t *settingContainer4;
    containerHelper.createContainer(&settingContainer4, SubGHzCustomScreen_->screen, LV_FLEX_FLOW_ROW, 25, 240);
    lv_obj_t* label5 = lv_label_create(settingContainer4);	
    lv_label_set_text(label5, "Modulatio ");
    Serial.println("Modulatio ");   
    Serial.println(CC1101SM.CC1101_MODULATION);
    int32_t valueMOD = CC1101SM.CC1101_MODULATION;
    SubGHzCustomScreen_->spinbox4 = SpinBox::createSpinbox(settingContainer4, valueMOD, 1, 4, 1, EVENTS::CustomSubGhzHelp_CB, "Modulatio");
    lv_obj_add_event_cb(settingContainer4, EVENTS::CustomSubGhzHelp_CB, LV_EVENT_LONG_PRESSED, (void *)"Modulatio");

    lv_obj_t *settingContainer5;
    containerHelper.createContainer(&settingContainer5, SubGHzCustomScreen_->screen, LV_FLEX_FLOW_ROW, 25, 240);
    lv_obj_t* label6 = lv_label_create(settingContainer5);	
    lv_label_set_text(label6, "Deviation:  ");
    Serial.println("Deviation:  ");
    Serial.println(CC1101SM.CC1101_DEVIATION);
    int32_t valueDEV = CC1101SM.CC1101_DEVIATION;
    SubGHzCustomScreen_->spinbox5 = SpinBox::createSpinbox(settingContainer5, valueDEV, 0, 200, 5, EVENTS::CustomSubGhzHelp_CB, "Deviation");
    lv_obj_add_event_cb(settingContainer5, EVENTS::CustomSubGhzHelp_CB, LV_EVENT_LONG_PRESSED, (void *)"Deviation");


    lv_obj_t *settingContainer6;
    containerHelper.createContainer(&settingContainer6, SubGHzCustomScreen_->screen, LV_FLEX_FLOW_ROW, 25, 240);
    lv_obj_t* label7 = lv_label_create(settingContainer6);	
    lv_label_set_text(label7, "Data  rate:");
    Serial.println("Data  rate:");
    Serial.println(CC1101SM.CC1101_DRATE);
    int32_t DRATE = CC1101SM.CC1101_DRATE;
    SubGHzCustomScreen_->spinbox6 = SpinBox::createSpinbox(settingContainer6, DRATE, 1, 250, 5, EVENTS::CustomSubGhzHelp_CB, "DataRate");
    lv_obj_add_event_cb(settingContainer6, EVENTS::CustomSubGhzHelp_CB, LV_EVENT_LONG_PRESSED, (void *)"DataRate");

    lv_obj_t *button_container;

    containerHelper.createContainer(&button_container, SubGHzCustomScreen_->screen, LV_FLEX_FLOW_ROW, 30, 240);

    lv_obj_t* listenButton = ButtonHelper::createButton(button_container, "Listen");
    lv_obj_add_event_cb(listenButton, EVENTS::btn_event_CUSTOM_REC_run, LV_EVENT_CLICKED, nullptr);

    
    lv_obj_t* saveButton = ButtonHelper::createButton(button_container, "Save");
   // lv_obj_add_event_cb(saveButton, EVENTS::save_signal_event, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *button_container2;
    containerHelper.createContainer(&button_container2, SubGHzCustomScreen_->screen, LV_FLEX_FLOW_ROW, 30, 240);

    
    lv_obj_t* playButton = ButtonHelper::createButton(button_container2, "Play");
    lv_obj_add_event_cb(playButton, EVENTS::sendCapturedEvent, LV_EVENT_CLICKED, nullptr);

    lv_obj_t* exitButton = ButtonHelper::createButton(button_container2, "Exit");
    lv_obj_add_event_cb(exitButton, EVENTS::btn_event_Replay_run, LV_EVENT_CLICKED, nullptr);

}

void ScreenManager::createIRRecScreen() {
    ContainerHelper containerHelper;    

    IRRecScreen_ = lv_obj_create(NULL);

    lv_scr_load(IRRecScreen_);

    lv_obj_delete(previous_screen);
    
    previous_screen = IRRecScreen_;
    lv_obj_set_flex_flow(IRRecScreen_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(IRRecScreen_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);


    lv_obj_set_style_pad_column(IRRecScreen_, 1, LV_PART_MAIN);
    lv_obj_set_style_pad_row(IRRecScreen_, 1, LV_PART_MAIN);

    
    // Create main text area
    text_area_IR = lv_textarea_create(IRRecScreen_);
    lv_obj_set_size(text_area_IR, 240, 240);
    lv_obj_align(text_area_IR, LV_ALIGN_CENTER, 0, -20);
    lv_textarea_set_text(text_area_IR, "Press listen to start.");
    lv_obj_set_scrollbar_mode(text_area_IR, LV_SCROLLBAR_MODE_OFF); 
    lv_textarea_set_cursor_click_pos(text_area_IR, false);
    containerHelper.createContainer(&button_container_IR_REC1_, IRRecScreen_, LV_FLEX_FLOW_ROW, 35, 240);


     // Listen Button
    lv_obj_t* listenButton = ButtonHelper::createButton(button_container_IR_REC1_, "Listen");
    lv_obj_add_event_cb(listenButton, EVENTS::btn_event_IR_run, LV_EVENT_CLICKED, nullptr);

    // Save Button
    lv_obj_t* saveButton = ButtonHelper::createButton(button_container_IR_REC1_, "Save");
   // lv_obj_add_event_cb(saveButton, EVENTS::save_signal_event, LV_EVENT_CLICKED, nullptr);

    containerHelper.createContainer(&button_container_IR_REC2_, IRRecScreen_, LV_FLEX_FLOW_ROW, 35, 240);

    // Play Button
    lv_obj_t* playButton = ButtonHelper::createButton(button_container_IR_REC2_, "Play");
    lv_obj_add_event_cb(playButton, EVENTS::btn_event_IR_run, LV_EVENT_CLICKED, nullptr);

    // Exit Button
    lv_obj_t* exitButton = ButtonHelper::createButton(button_container_IR_REC2_, "Exit");
//lv_obj_add_event_cb(exitButton, EVENTS::exit_event, LV_EVENT_CLICKED, nullptr);
}

void ScreenManager::createRFdetectScreen() {
    ContainerHelper containerHelper;

    detectScreen_ = lv_obj_create(NULL);
    lv_scr_load(detectScreen_);
    lv_obj_delete(previous_screen);

    previous_screen = detectScreen_;
    lv_obj_set_flex_flow(detectScreen_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(detectScreen_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);


    containerHelper.createContainer(&secondLabel_detectForce_container_, detectScreen_, LV_FLEX_FLOW_COLUMN, 270, 240);
   // lv_obj_set_style_border_width(topLabel_container_, 0, LV_PART_MAIN);


    detectLabel = lv_label_create(secondLabel_detectForce_container_);
    lv_obj_set_size(detectLabel, 210, 180);
    lv_obj_align(detectLabel, LV_ALIGN_LEFT_MID, 0, 0);
    lv_label_set_text(detectLabel, "Press start");


   containerHelper.createContainer(&button_container_Detec1_, detectScreen_, LV_FLEX_FLOW_ROW, 35, 240);

   lv_obj_t *startButton = ButtonHelper::createButton(button_container_Detec1_, "Start");
    lv_obj_add_event_cb(startButton, EVENTS::btn_event_detect_run, LV_EVENT_CLICKED, NULL);

   lv_obj_t *stopButton = ButtonHelper::createButton(button_container_Detec1_, "Exit");
    lv_obj_add_event_cb(stopButton, EVENTS::closeCC1101scanner, LV_EVENT_CLICKED, NULL);
}

void ScreenManager::draw_image() {
  LV_IMAGE_DECLARE(logo_dsc);
  lv_obj_t * img1 = lv_image_create(lv_screen_active());
  lv_image_set_src(img1, &logo_dsc);
  lv_obj_align(img1, LV_ALIGN_CENTER, 0, 0);
}

void ScreenManager::createSourAppleScreen() {
    // ContainerHelper containerHelper;

    // SourAppleScreen_ = lv_obj_create(NULL);

    // lv_scr_load(SourAppleScreen_);
    // lv_obj_delete(previous_screen);
    // previous_screen = SourAppleScreen_;
    // lv_obj_set_flex_flow(SourAppleScreen_, LV_FLEX_FLOW_COLUMN);
    // lv_obj_set_flex_align(SourAppleScreen_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // lv_obj_t * topLabel_container_;
    // containerHelper.createContainer(&topLabel_container_, SourAppleScreen_, LV_FLEX_FLOW_ROW, 35, 240);
    // lv_obj_set_style_border_width(topLabel_container_, 0, LV_PART_MAIN);

    // // Create main text area
    // text_area_SourApple = lv_textarea_create(SourAppleScreen_);
    // lv_obj_set_size(text_area_SourApple, 240, 140);
    // lv_obj_align(text_area_SourApple, LV_ALIGN_CENTER, 0, -20);
    // lv_textarea_set_text(text_area_SourApple, "Sour Apple\nWill spam BLE devices\nMay cause crash of Apple devices");
    // lv_obj_set_scrollbar_mode(text_area_SourApple, LV_SCROLLBAR_MODE_OFF); 
    // lv_textarea_set_cursor_click_pos(text_area_SourApple, false);

    // lv_obj_t *buttonContainer;
    // containerHelper.createContainer(&buttonContainer, SourAppleScreen_, LV_FLEX_FLOW_ROW, 35, 240);


    // lv_obj_t *listenButton = ButtonHelper::createButton(buttonContainer, "Start");
    // lv_obj_add_event_cb(listenButton, EVENTS::btn_event_SourApple_Start, LV_EVENT_CLICKED, NULL); 

    // lv_obj_t *saveButton = ButtonHelper::createButton(buttonContainer, "Stop");
    // lv_obj_add_event_cb(saveButton, EVENTS::btn_event_SourApple_Stop, LV_EVENT_CLICKED, NULL); 
}

void ScreenManager::createBTSPamScreen() {
    // ContainerHelper containerHelper;

    // BTSpamScreen_ = lv_obj_create(NULL);
    // lv_scr_load(BTSpamScreen_);
    // lv_obj_delete(previous_screen);
    // previous_screen = BTSpamScreen_;
    // lv_obj_set_flex_flow(BTSpamScreen_, LV_FLEX_FLOW_COLUMN);
    // lv_obj_set_flex_align(BTSpamScreen_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // lv_obj_t * topLabel_container_;
    // containerHelper.createContainer(&topLabel_container_, BTSpamScreen_, LV_FLEX_FLOW_ROW, 35, 240);
    // lv_obj_set_style_border_width(topLabel_container_, 0, LV_PART_MAIN);

    // // Create main text area
    // text_area_BTSpam = lv_textarea_create(BTSpamScreen_);
    // lv_obj_set_size(text_area_BTSpam, 240, 140);
    // lv_obj_align(text_area_BTSpam, LV_ALIGN_CENTER, 0, -20);
    // lv_textarea_set_text(text_area_BTSpam, "Will spam BLE devices\nMay cause crash of some");
    // lv_obj_set_scrollbar_mode(text_area_BTSpam, LV_SCROLLBAR_MODE_OFF); 
    // lv_textarea_set_cursor_click_pos(text_area_BTSpam, false);

    // lv_obj_t *buttonContainer;
    // containerHelper.createContainer(&buttonContainer, BTSpamScreen_, LV_FLEX_FLOW_ROW_WRAP, 120, 240);
    // lv_obj_set_flex_align(buttonContainer, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    // int applle = 0;
    // int Microsoft = 1;
    // int Samsung = 2;
    // int Android = 3;
    // int ALL = 4;

    // lv_obj_t *startButton = ButtonHelper::createButton(buttonContainer, "Start Apple");
    // lv_obj_add_event_cb(startButton, EVENTS::btn_event_BTSpam_Start, LV_EVENT_CLICKED, &applle); 

    // lv_obj_t *startButton1 = ButtonHelper::createButton(buttonContainer, "Start Microsoft");
    // lv_obj_add_event_cb(startButton1, EVENTS::btn_event_BTSpam_Start, LV_EVENT_CLICKED, &Microsoft); 

    // lv_obj_t *startButton2 = ButtonHelper::createButton(buttonContainer, "Start Samsung");
    // lv_obj_add_event_cb(startButton2, EVENTS::btn_event_BTSpam_Start, LV_EVENT_CLICKED, &Samsung); 

    // lv_obj_t *startButton3 = ButtonHelper::createButton(buttonContainer, "Start Android");
    // lv_obj_add_event_cb(startButton3, EVENTS::btn_event_BTSpam_Start, LV_EVENT_CLICKED, &Android); 

    // lv_obj_t *startButton4 = ButtonHelper::createButton(buttonContainer, "Start ALL");
    // lv_obj_add_event_cb(startButton4, EVENTS::btn_event_BTSpam_Start, LV_EVENT_CLICKED, &ALL); 

    // lv_obj_t *saveButton = ButtonHelper::createButton(buttonContainer, "Stop");
    // lv_obj_add_event_cb(saveButton, EVENTS::btn_event_SourApple_Stop, LV_EVENT_CLICKED, NULL); 
}

void ScreenManager::createIRMenuScreen() {
    lv_obj_t *IRMenu = lv_obj_create(NULL);                                        
    lv_scr_load(IRMenu);
    lv_obj_delete(previous_screen);
    ScreenManager::apply_neon_theme(IRMenu);
    previous_screen = IRMenu;                                                        
    
    lv_obj_t *btn_IR_BGONE = lv_btn_create(IRMenu);                                
    lv_obj_set_pos(btn_IR_BGONE, 25, 10);                                             
    lv_obj_set_size(btn_IR_BGONE, 200, 50);                                           
    lv_obj_add_event_cb(btn_IR_BGONE, EVENTS::btn_event_UR_BGONE, LV_EVENT_CLICKED, NULL); 
    apply_neon_theme_button(btn_IR_BGONE); 

    lv_obj_t *label_IR_BGONE = lv_label_create(btn_IR_BGONE);
    lv_label_set_text(label_IR_BGONE, "Turn the TV's OFF!!!");       
    lv_obj_center(label_IR_BGONE);

    lv_obj_t *btn_IR_READ = lv_btn_create(IRMenu);                                
    lv_obj_set_pos(btn_IR_READ, 25, 70);                                             
    lv_obj_set_size(btn_IR_READ, 200, 50);                                           
    lv_obj_add_event_cb(btn_IR_READ, EVENTS::btn_event_IR_START_READ, LV_EVENT_CLICKED, NULL); 
    apply_neon_theme_button(btn_IR_READ); 

    lv_obj_t *label_IR_READ = lv_label_create(btn_IR_READ);
    lv_label_set_text(label_IR_READ, "Read IR signals");       
    lv_obj_center(label_IR_READ);
}


void ScreenManager::createmainMenu()
{
    lv_obj_t *mainMenu = lv_obj_create(NULL); 
    ScreenManager::apply_neon_theme(mainMenu);                                       
    lv_scr_load(mainMenu);    
    previous_screen = mainMenu;                                                        

 
    lv_obj_t *btn_subGhz_main = lv_btn_create(mainMenu);                                
    lv_obj_set_pos(btn_subGhz_main, 25, 10);                                             
    lv_obj_set_size(btn_subGhz_main, 200, 50);                                           
    lv_obj_add_event_cb(btn_subGhz_main, EVENTS::btn_event_subGhzTools, LV_EVENT_CLICKED, NULL); 
    ScreenManager::apply_neon_theme_button(btn_subGhz_main); 

    lv_obj_t *label_subGhz_main = lv_label_create(btn_subGhz_main);
    lv_label_set_text(label_subGhz_main, "RF SubGhz Tools");       
    lv_obj_center(label_subGhz_main);


    lv_obj_t *btn_teslaCharger_menu = lv_btn_create(mainMenu);
    lv_obj_set_pos(btn_teslaCharger_menu, 25, 70);
    lv_obj_set_size(btn_teslaCharger_menu, 200, 50);
    lv_obj_add_event_cb(btn_teslaCharger_menu, EVENTS::btn_event_teslaCharger_run, LV_EVENT_ALL, NULL);
    ScreenManager::apply_neon_theme_button(btn_teslaCharger_menu); 

    lv_obj_t *label_teslaCharger_menu = lv_label_create(btn_teslaCharger_menu);
    lv_label_set_text(label_teslaCharger_menu, "Transmit tesla charger code");
    lv_obj_center(label_teslaCharger_menu);

    lv_obj_t *btn_IR_menu = lv_btn_create(mainMenu);
    lv_obj_set_pos(btn_IR_menu, 25, 130);
    lv_obj_set_size(btn_IR_menu, 200, 50);
    lv_obj_add_event_cb(btn_IR_menu, EVENTS::btn_event_IR_menu_run, LV_EVENT_ALL, NULL);
    ScreenManager::apply_neon_theme_button(btn_IR_menu); 

    lv_obj_t *label_IR_menu = lv_label_create(btn_IR_menu);
    lv_label_set_text(label_IR_menu, "IR Tools");
    lv_obj_center(label_IR_menu);

    lv_obj_t *btn_NFC_menu = lv_btn_create(mainMenu);
    lv_obj_set_pos(btn_NFC_menu, 25, 190);
    lv_obj_set_size(btn_NFC_menu, 200, 50);
    lv_obj_add_event_cb(btn_NFC_menu, EVENTS::btn_event_NFC_menu_run, LV_EVENT_CLICKED, NULL);
    ScreenManager::apply_neon_theme_button(btn_NFC_menu); 

    lv_obj_t *label_NFC_menu = lv_label_create(btn_NFC_menu);
    lv_label_set_text(label_NFC_menu, "TEST IR TX");
    lv_obj_center(label_NFC_menu);

    lv_obj_t *btn_RF24_menu = lv_btn_create(mainMenu);
    lv_obj_set_pos(btn_RF24_menu, 25, 260);
    lv_obj_set_size(btn_RF24_menu, 200, 50);
    lv_obj_add_event_cb(btn_RF24_menu, EVENTS::btn_event_RF24_menu_run, LV_EVENT_CLICKED, NULL);
    ScreenManager::apply_neon_theme_button(btn_RF24_menu); 

    lv_obj_t *label_RF24_menu = lv_label_create(btn_RF24_menu);
    lv_label_set_text(label_RF24_menu, "2.4 Ghz Jammer");
    lv_obj_center(label_RF24_menu);
}


void ScreenManager::createJammerMenu() {
    lv_obj_t *jammerMenu = lv_obj_create(NULL);
    ScreenManager::apply_neon_theme(jammerMenu);
    lv_scr_load(jammerMenu);
    lv_obj_delete(previous_screen);
    previous_screen = jammerMenu;

    // Bluetooth Jam Button
    lv_obj_t *btn_BT_main = lv_btn_create(jammerMenu);
    lv_obj_set_pos(btn_BT_main, 25, 10);
    lv_obj_set_size(btn_BT_main, 200, 50);
    lv_obj_add_event_cb(btn_BT_main, [](lv_event_t *e) {
        setState(BLUETOOTH_JAM);
    }, LV_EVENT_CLICKED, NULL);
    lv_obj_t *label_BTJAM_main = lv_label_create(btn_BT_main);
    lv_label_set_text(label_BTJAM_main, "BT Jam");
    lv_obj_center(label_BTJAM_main);
    apply_neon_theme_button(btn_BT_main); 

    // Drone Jam Button
    lv_obj_t *btn_DroneJam_menu = lv_btn_create(jammerMenu);
    lv_obj_set_pos(btn_DroneJam_menu, 25, 70);
    lv_obj_set_size(btn_DroneJam_menu, 200, 50);
    lv_obj_add_event_cb(btn_DroneJam_menu, [](lv_event_t *e) {
        setState(DRONE_JAM);
    }, LV_EVENT_CLICKED, NULL);
    lv_obj_t *label_DroneJam_menu = lv_label_create(btn_DroneJam_menu);
    lv_label_set_text(label_DroneJam_menu, "Drone Jam");
    lv_obj_center(label_DroneJam_menu);
    apply_neon_theme_button(btn_DroneJam_menu); 

    // BLE Jam Button
    lv_obj_t *btn_BLE_menu = lv_btn_create(jammerMenu);
    lv_obj_set_pos(btn_BLE_menu, 25, 130);
    lv_obj_set_size(btn_BLE_menu, 200, 50);
    lv_obj_add_event_cb(btn_BLE_menu, [](lv_event_t *e) {
        setState(BLE_JAM);
    }, LV_EVENT_CLICKED, NULL);
    lv_obj_t *label_BLEJAM_menu = lv_label_create(btn_BLE_menu);
    lv_label_set_text(label_BLEJAM_menu, "BLE Jam");
    lv_obj_center(label_BLEJAM_menu);
    apply_neon_theme_button(btn_BLE_menu); 

    // WiFi Jam Button
    lv_obj_t *btn_WiFi_menu = lv_btn_create(jammerMenu);
    lv_obj_set_pos(btn_WiFi_menu, 25, 190);
    lv_obj_set_size(btn_WiFi_menu, 200, 50);
    lv_obj_add_event_cb(btn_WiFi_menu, [](lv_event_t *e) {
        setState(WIFI_JAM);
    }, LV_EVENT_CLICKED, NULL);
    lv_obj_t *label_WiFi_menu = lv_label_create(btn_WiFi_menu);
    lv_label_set_text(label_WiFi_menu, "WiFi Jam");
    lv_obj_center(label_WiFi_menu);
    apply_neon_theme_button(btn_WiFi_menu); 

    // ZigBee Jam Button
    lv_obj_t *btn_ZigBee_menu = lv_btn_create(jammerMenu);
    lv_obj_set_pos(btn_ZigBee_menu, 25, 250);
    lv_obj_set_size(btn_ZigBee_menu, 200, 50);
    lv_obj_add_event_cb(btn_ZigBee_menu, [](lv_event_t *e) {
        setState(ZIGBEE_JAM);
    }, LV_EVENT_CLICKED, NULL);
    lv_obj_t *label_ZigBee_menu = lv_label_create(btn_ZigBee_menu);
    lv_label_set_text(label_ZigBee_menu, "ZigBee Jam");
    lv_obj_center(label_ZigBee_menu);
    apply_neon_theme_button(btn_ZigBee_menu); 

    // Misc Jam Button
    lv_obj_t *btn_Misc_menu = lv_btn_create(jammerMenu);
    lv_obj_set_pos(btn_Misc_menu, 25, 310);
    lv_obj_set_size(btn_Misc_menu, 200, 50);
    lv_obj_add_event_cb(btn_Misc_menu, [](lv_event_t *e) {
        setState(MISC_JAM);
    }, LV_EVENT_CLICKED, NULL);
    lv_obj_t *label_Misc_menu = lv_label_create(btn_Misc_menu);
    lv_label_set_text(label_Misc_menu, "Misc Jam");
    lv_obj_center(label_Misc_menu);
    apply_neon_theme_button(btn_Misc_menu); 

    // Stop Button
    lv_obj_t *btn_Stop_menu = lv_btn_create(jammerMenu);
    lv_obj_set_pos(btn_Stop_menu, 25, 370);
    lv_obj_set_size(btn_Stop_menu, 200, 50);
    lv_obj_add_event_cb(btn_Stop_menu, [](lv_event_t *e) {
        setState(IDLE);
        lv_obj_t *parent = lv_obj_get_parent((lv_obj_t *)lv_event_get_target(e));
        lv_obj_t *child = lv_obj_get_child(parent, NULL);
        while (child) {
            lv_obj_set_style_bg_color(child, lv_color_hex(0x00FF00), 0); // Reset all button colors to green

        }
    }, LV_EVENT_CLICKED, NULL);
    lv_obj_t *label_Stop_menu = lv_label_create(btn_Stop_menu);
    lv_label_set_text(label_Stop_menu, "Stop");
    lv_obj_center(label_Stop_menu);
    apply_neon_theme_button(btn_Stop_menu); 
}

void ScreenManager::createTeslaScreen() {
    ContainerHelper containerHelper;
    teslaScreen_ = lv_obj_create(NULL);
    lv_scr_load(teslaScreen_);
    lv_obj_delete(previous_screen);
    previous_screen = teslaScreen_;
    lv_obj_set_flex_flow(teslaScreen_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(teslaScreen_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * topLabel_container_;
    containerHelper.createContainer(&topLabel_container_, teslaScreen_, LV_FLEX_FLOW_ROW, 35, 240);
    lv_obj_set_style_border_width(topLabel_container_, 0, LV_PART_MAIN);

    lv_obj_t * topLabel = lv_label_create(topLabel_container_);
    lv_obj_align(topLabel, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(topLabel, "Tesla charger signal\n Push the button.");

    lv_obj_t* sendButton = lv_button_create(teslaScreen_);
    lv_obj_t * sendBtnLbl = lv_label_create(sendButton);
    lv_label_set_text(sendBtnLbl, "Send");
    lv_obj_align_to(sendButton, teslaScreen_, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_align_to(sendBtnLbl, sendButton, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(sendButton, 120, 60);

    lv_obj_add_event_cb(sendButton, EVENTS::sendTesla, LV_EVENT_CLICKED, NULL);
    apply_neon_theme_button(sendButton);  
}

void ScreenManager::createBruteScreen() {


    ContainerHelper containerHelper;
    teslaScreen_ = lv_obj_create(NULL);
    lv_scr_load(teslaScreen_);
    lv_obj_delete(previous_screen);
    previous_screen = teslaScreen_;
    lv_obj_set_flex_flow(teslaScreen_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(teslaScreen_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * topLabel_container_;
    containerHelper.createContainer(&topLabel_container_, teslaScreen_, LV_FLEX_FLOW_ROW, 35, 240);
    lv_obj_set_style_border_width(topLabel_container_, 0, LV_PART_MAIN);

    lv_obj_t * topLabel = lv_label_create(topLabel_container_);
    lv_obj_align(topLabel, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(topLabel, "Came + NICE brute forcer\n Push the button.");
    text_area_ = lv_label_create(topLabel_container_);
    lv_obj_align(text_area_, LV_ALIGN_CENTER, 30, 0);
    lv_label_set_text(text_area_, "0/4096");

    lv_obj_t* cameButton = lv_button_create(teslaScreen_);
    lv_obj_t * cameButtonlbl = lv_label_create(cameButton);
    lv_label_set_text(cameButtonlbl, "CAME");
    lv_obj_align_to(cameButton, teslaScreen_, LV_ALIGN_OUT_LEFT_MID, 0, 0);
    lv_obj_align_to(cameButtonlbl, cameButton, LV_ALIGN_OUT_LEFT_MID, 0, 0);
    lv_obj_set_size(cameButton, 120, 60);
     lv_obj_t* niceButton = lv_button_create(teslaScreen_);
    lv_obj_t * niceButtonlbl = lv_label_create(niceButton);
    lv_label_set_text(niceButtonlbl, "NICE");
    lv_obj_align_to(niceButton, teslaScreen_, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
    lv_obj_align_to(niceButtonlbl, niceButton, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
    lv_obj_set_size(niceButton, 120, 60);


    lv_obj_add_event_cb(cameButton, EVENTS::btn_event_Brute_CAME, LV_EVENT_CLICKED, NULL);
    apply_neon_theme_button(cameButton); 
     lv_obj_add_event_cb(niceButton, EVENTS::btn_event_Brute_NICE, LV_EVENT_CLICKED, NULL);
    apply_neon_theme_button(niceButton); 
}


void ScreenManager::createBTMenu() {
    lv_obj_t *BTMenu = lv_obj_create(NULL);
    ScreenManager::apply_neon_theme(BTMenu);
    lv_scr_load(BTMenu);    
    lv_obj_delete(previous_screen);
    previous_screen = BTMenu;

    lv_obj_t *btn_BT_main = lv_btn_create(BTMenu);
    lv_obj_set_pos(btn_BT_main, 25, 10);
    lv_obj_set_size(btn_BT_main, 150, 50);
    lv_obj_add_event_cb(btn_BT_main, EVENTS::btn_event_SourApple, LV_EVENT_CLICKED, NULL);
    lv_obj_t *label_BLESPAM_main = lv_label_create(btn_BT_main);
    lv_label_set_text(label_BLESPAM_main, "Sour Apple");
    lv_obj_center(label_BLESPAM_main);
    apply_neon_theme_button(btn_BT_main); 

    lv_obj_t *btn_BT_maspam = lv_btn_create(BTMenu);
    lv_obj_set_pos(btn_BT_maspam, 25, 70);
    lv_obj_set_size(btn_BT_maspam, 150, 50);
    lv_obj_add_event_cb(btn_BT_maspam, EVENTS::btn_event_BTSpam, LV_EVENT_CLICKED, NULL);
    lv_obj_t *label_SourApple_main = lv_label_create(btn_BT_maspam);
    lv_label_set_text(label_SourApple_main, "BLE spam");
    lv_obj_center(label_SourApple_main);
    apply_neon_theme_button(btn_BT_maspam); 

    lv_obj_t *btn_c1101Others_menu = lv_btn_create(lv_scr_act());
    lv_obj_set_pos(btn_c1101Others_menu, 25, 250);
    lv_obj_set_size(btn_c1101Others_menu, 200, 50);
    lv_obj_t *label_c1101Others_menu = lv_label_create(btn_c1101Others_menu);
    lv_label_set_text(label_c1101Others_menu, "Back");
    lv_obj_center(label_c1101Others_menu);
    lv_obj_add_event_cb(btn_c1101Others_menu, EVENTS::btn_event_mainMenu_run, LV_EVENT_CLICKED, NULL);
    apply_neon_theme_button(btn_c1101Others_menu); 
}

void ScreenManager::createRFMenu() {
    lv_obj_t *rfMenu = lv_obj_create(NULL);
    ScreenManager::apply_neon_theme(rfMenu);
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
    apply_neon_theme_button(btn_playZero_menu); 

    lv_obj_t *btn_Brute_menu = lv_btn_create(lv_scr_act());
    lv_obj_set_pos(btn_Brute_menu, 25, 70);
    lv_obj_set_size(btn_Brute_menu, 200, 50);
    lv_obj_add_event_cb(btn_Brute_menu, EVENTS::btn_event_Brute_run, LV_EVENT_ALL, NULL);
    lv_obj_t *label_Brute_menu = lv_label_create(btn_Brute_menu);
    lv_label_set_text(label_Brute_menu, "BruteForce");
    lv_obj_center(label_Brute_menu);
    apply_neon_theme_button(btn_Brute_menu); 

    lv_obj_t *btn_c1101Alanalyzer_menu = lv_btn_create(lv_scr_act());
    lv_obj_set_pos(btn_c1101Alanalyzer_menu, 25, 130);
    lv_obj_set_size(btn_c1101Alanalyzer_menu, 200, 50);
    lv_obj_add_event_cb(btn_c1101Alanalyzer_menu, EVENTS::btn_event_Replay_run, LV_EVENT_ALL, NULL);
    lv_obj_t *label_c1101Alanalyzer_menu = lv_label_create(btn_c1101Alanalyzer_menu);
    lv_label_set_text(label_c1101Alanalyzer_menu, "rec/play");
    lv_obj_center(label_c1101Alanalyzer_menu);
    apply_neon_theme_button(btn_c1101Alanalyzer_menu); 

    lv_obj_t *btn_SubGhzdetectForce_menu = lv_btn_create(lv_scr_act());
    lv_obj_set_pos(btn_SubGhzdetectForce_menu, 25, 190);
    lv_obj_set_size(btn_SubGhzdetectForce_menu, 200, 50);
    lv_obj_add_event_cb(btn_SubGhzdetectForce_menu, EVENTS::btn_event_detectForce_run, LV_EVENT_ALL, NULL);
    lv_obj_t *label_SubGhzdetectForce_menu = lv_label_create(btn_SubGhzdetectForce_menu);
    lv_label_set_text(label_SubGhzdetectForce_menu, "detect signal");
    lv_obj_center(label_SubGhzdetectForce_menu);
    apply_neon_theme_button(btn_SubGhzdetectForce_menu); 

    lv_obj_t *btn_c1101Others_menu = lv_btn_create(lv_scr_act());
    lv_obj_set_pos(btn_c1101Others_menu, 25, 250);
    lv_obj_set_size(btn_c1101Others_menu, 200, 50);
    lv_obj_t *label_c1101Others_menu = lv_label_create(btn_c1101Others_menu);
    lv_label_set_text(label_c1101Others_menu, "Back");
    lv_obj_center(label_c1101Others_menu);
    lv_obj_add_event_cb(btn_c1101Others_menu, EVENTS::btn_event_mainMenu_run, LV_EVENT_CLICKED, NULL);
    apply_neon_theme_button(btn_c1101Others_menu); 
}

void ScreenManager::createFileExplorerScreen()
{
    ContainerHelper containerHelper;
    fileExplorerScreen = lv_obj_create(NULL);
    ScreenManager::apply_neon_theme(fileExplorerScreen);
    lv_scr_load(fileExplorerScreen);
    lv_obj_delete(previous_screen);

    lv_obj_set_style_bg_color(fileExplorerScreen, lv_color_black(), 0);

    lv_obj_t *file_explorer = lv_file_explorer_create(fileExplorerScreen);
    lv_file_explorer_open_dir(file_explorer, "S:/");

    lv_obj_set_style_bg_color(file_explorer, lv_color_black(), 0);

    lv_obj_t *footer;
    containerHelper.createContainer(&footer, fileExplorerScreen, LV_FLEX_FLOW_ROW, 35, 240);
    
    lv_obj_t *explorerDelelete_btn = lv_btn_create(footer);
    lv_obj_t *explorerRename_btn = lv_btn_create(footer);
    lv_obj_t *explorerMove_btn = lv_btn_create(footer);

    lv_obj_set_size(explorerDelelete_btn, 70, 30);
    lv_obj_set_size(explorerRename_btn, 70, 30);
    lv_obj_set_size(explorerMove_btn, 70, 30);

    lv_obj_set_align(explorerDelelete_btn, LV_ALIGN_BOTTOM_LEFT);
    lv_obj_set_align(explorerRename_btn, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_align(explorerMove_btn, LV_ALIGN_BOTTOM_RIGHT);
    
    ScreenManager::apply_neon_theme_button(explorerDelelete_btn);
    ScreenManager::apply_neon_theme_button(explorerRename_btn);
    ScreenManager::apply_neon_theme_button(explorerMove_btn);

    lv_obj_t * explorerDelelete_lbl = lv_label_create(explorerDelelete_btn);
    lv_obj_t * explorerRename_lbl = lv_label_create(explorerRename_btn);
    lv_obj_t * explorerMove_lbl = lv_label_create(explorerMove_btn);

    lv_obj_set_align(explorerDelelete_lbl, LV_ALIGN_CENTER);
    lv_obj_set_align(explorerRename_lbl, LV_ALIGN_CENTER);
    lv_obj_set_align(explorerMove_lbl, LV_ALIGN_CENTER);

    lv_label_set_text(explorerDelelete_lbl, "DEL");
    lv_label_set_text(explorerRename_lbl, "RNM");
    lv_label_set_text(explorerMove_lbl, "MOV");

    lv_obj_add_event_cb(explorerDelelete_btn, EVENTS::file_explorer_event_delete_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(file_explorer, EVENTS::file_explorer_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
}


lv_style_t style_bg;
lv_style_t style_text;
lv_theme_t * neon_theme;
lv_style_t style_button_bg;

void ScreenManager::apply_neon_theme(lv_obj_t * obj) {
   if (!obj) return;
    lv_obj_add_style(obj, &style_bg, LV_PART_MAIN);  
    lv_obj_add_style(obj, &style_text, LV_PART_MAIN); 
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x000000), LV_PART_MAIN); 
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN); 
    lv_obj_set_style_text_color(obj, lv_color_hex(0x000000), LV_PART_MAIN); 
    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN); 
    lv_style_init(&style_button_bg);
    lv_style_set_bg_color(&style_button_bg, lv_color_hex(0x39FF14)); 
    lv_style_set_bg_opa(&style_button_bg, LV_OPA_COVER);
    lv_obj_add_style(obj, &style_button_bg, LV_PART_MAIN);  
}

void ScreenManager::apply_neon_theme_button(lv_obj_t * obj) {
   if (!obj) return;
    lv_obj_add_style(obj, &style_bg, LV_PART_MAIN);  
    lv_obj_add_style(obj, &style_text, LV_PART_MAIN); 
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x39FF14), LV_PART_MAIN); 
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN); 
    lv_obj_set_style_text_color(obj, lv_color_hex(0x000000), LV_PART_MAIN); 
    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN); 
    lv_style_init(&style_button_bg); 
}



void ScreenManager::create_neon_theme(void) {
    lv_style_init(&style_bg);
    lv_style_set_bg_color(&style_bg, lv_color_black());
    lv_style_set_bg_opa(&style_bg, LV_OPA_COVER);
    lv_style_init(&style_text);
    lv_style_set_text_color(&style_text, lv_color_black()); 
    lv_style_set_text_font(&style_text, &lv_font_montserrat_14);  
}


void ScreenManager::init_neon_theme(void) {
    create_neon_theme();
    lv_obj_t * screen = lv_scr_act();
    apply_neon_theme(screen);
}
