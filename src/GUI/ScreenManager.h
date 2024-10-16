#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include "lvgl.h"
#include "settingsButton.h" 


class ScreenManager {
public:
    static ScreenManager& getInstance();

    ScreenManager(const ScreenManager&) = delete;
    ScreenManager& operator=(const ScreenManager&) = delete;

    void createReplayScreen();
    void createmainMenu();
    void createRFMenu();
    void createRFSettingsScreen(); 
    void createSubPlayerScreen();
    void createFileBrowser(lv_obj_t* parent);
    void createBTMenu();
    void createSourAppleScreen();

    static void btn_event_RFSettings_show(lv_event_t* e);


    lv_obj_t* getFreqInput();  
    lv_obj_t* getTextArea();  
    lv_obj_t* getTextAreaRCSwitchMethod(); 
    lv_obj_t* getTextAreaSourAple();
    lv_obj_t* getFilenameInput();
    lv_obj_t* getKeyboardFreq();
    lv_obj_t* getPresetDropdown();
    lv_obj_t* getSyncDropdown();
    lv_obj_t* getPTKDropdown();
    lv_obj_t* getPulseLenghtInput();


private:
    ScreenManager();
    ~ScreenManager();

    lv_obj_t* ReplayScreen_;
    lv_obj_t* SourAppleScreen_;
    lv_obj_t* text_area_;
    lv_obj_t* freqInput_;
    lv_obj_t* filenameInput_;
    lv_obj_t* kb_freq_;
    lv_obj_t* kb_qwert_;
    lv_obj_t* fileName_container_;
    lv_obj_t* topLabel_container_;
    lv_obj_t* text_area_replay;
    lv_obj_t* text_area_SourApple;
    lv_obj_t* button_container1_;
    lv_obj_t* button_container2_;
    lv_obj_t* button_container_settings;
    lv_obj_t* C1101preset_container_;
    lv_obj_t* C1101buffer_dropdown_;
    lv_obj_t* checkbox_container_;
    lv_obj_t* C1101PTK_container_;
    lv_obj_t* C1101PTK_dropdown_;
    lv_obj_t* C1101SYNC_container_;
    lv_obj_t* C1101SYNC_dropdown_;
    lv_obj_t *buttonSettings_;
    lv_obj_t * C1101pulseLenght_container_;
    lv_obj_t * pulseLenghInput_;
    lv_obj_t * pulseGet_;
    lv_obj_t* SubPlayerScreen_;
    lv_obj_t* topLabel_RCSwitchMethod_container_;
    lv_obj_t* RCSwitchMethodScreen_;
    lv_obj_t* button_container_RCSwitchMethod1_;
    lv_obj_t* button_container_RCSwitchMethod2_; 
    lv_obj_t* settingsButton_; 
    lv_obj_t* secondLabel_container_;
    
    char* current_dir;
    char* selected_file;
};

#endif 