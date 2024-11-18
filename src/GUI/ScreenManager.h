#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include "lvgl.h"
#include <functional>
#include "XPT2046_Bitbang.h"

class ScreenManager {
public:

    lv_obj_t* C1101preset_dropdown_;
    lv_obj_t* C1101type_dropdown_;
    lv_obj_t*  detect_dropdown_;
    lv_obj_t* ReplayScreen_;
    lv_obj_t* topLabel_container_;
    lv_obj_t *mbox_container;
    static ScreenManager& getInstance();

    ScreenManager(const ScreenManager&) = delete;
    ScreenManager& operator=(const ScreenManager&) = delete;
    void createReplayScreen();
    void createmainMenu();
    void createRFMenu();
    void createBTMenu();
    void createSourAppleScreen();
    void createRFdetectScreen();
    void createFileExplorerScreen();


    lv_obj_t* customPreset;
    lv_obj_t *input_datarate;
    lv_obj_t *input_bandwidth;
    lv_obj_t *input_deviation;
    lv_obj_t *dropdown_modulation;

    lv_obj_t* getFreqInput();  
    lv_obj_t* getTextArea();  
    lv_obj_t* getdetectLabel();  
    lv_obj_t* getTextAreaRCSwitchMethod(); 
    lv_obj_t* getTextAreaSourAple();
    lv_obj_t* getFilenameInput();
    lv_obj_t* getKeyboardFreq();
    lv_obj_t* getPresetDropdown();
    lv_obj_t* getSyncDropdown();
    lv_obj_t* getDetectDropdown();
    lv_obj_t* getSquareLineContainer();

    void createBTSPamScreen();
    lv_obj_t*  getTextAreaBTSpam();

    static char selected_file_path[];
    static char selected_file_name[];
private:
    ScreenManager();
    ~ScreenManager();
    
    
    lv_obj_t* detectScreen_;
    lv_obj_t* SourAppleScreen_;
    lv_obj_t* BTSpamScreen_;
    lv_obj_t* fileExplorerScreen;
    lv_obj_t* text_area_;

    lv_obj_t* filenameInput_;
    lv_obj_t* kb_freq_;
    lv_obj_t* kb_qwert_;
    lv_obj_t* fileName_container_;
    
    lv_obj_t* browserButton_container_;
    lv_obj_t* browserButton2_container_;
    lv_obj_t* text_area_replay;
    lv_obj_t* detectLabel;

    lv_obj_t* text_area_SourApple;
    lv_obj_t* text_area_BTSpam;
    lv_obj_t* button_container1_;
    lv_obj_t* button_container2_;
    lv_obj_t* button_container_settings;
    lv_obj_t* C1101preset_container_;
    lv_obj_t* button_container_Detec1_;
    lv_obj_t* button_container_Detec2_;

    lv_obj_t* checkbox_container_;
    lv_obj_t* C1101PTK_container_;
    lv_obj_t* C1101SYNC_container_;
    lv_obj_t* C1101SYNC_dropdown_;
    lv_obj_t *buttonSettings_;
    lv_obj_t * C1101pulseLenght_container_;
    lv_obj_t * pulseLenghInput_;
    lv_obj_t * pulseGet_;
    lv_obj_t* SubPlayerScreen_;
    lv_obj_t* topLabel_RCSwitchMethod_container_;
    lv_obj_t* button_container_RCSwitchMethod1_;
    lv_obj_t* button_container_RCSwitchMethod2_; 
    lv_obj_t* quareLine_container;
    lv_obj_t* button_container_detectForce1_;
    lv_obj_t* button_container_detectForce2_;
    lv_obj_t* topLabel_detectForce_container_;
    lv_obj_t* settingsButton_; 
    lv_obj_t* secondLabel_container_;
    lv_obj_t* secondLabel_detectForce_container_;

    lv_obj_t* file_explorer_;
};

#endif 