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
    void createRFSettingsScreen(lv_event_t*); 

    lv_obj_t* getFreqInput();  
    lv_obj_t* getTextArea();  
    lv_obj_t* getFilenameInput();
    lv_obj_t* getKeyboardFreq();
    lv_obj_t* getPresetDropdown();
    lv_obj_t* getSyncDropdown();
    lv_obj_t* getPTKDropdown();


private:
    ScreenManager();
    ~ScreenManager();

    lv_obj_t* ReplayScreen_;
    lv_obj_t* text_area_;
    lv_obj_t* freqInput_;
    lv_obj_t* filenameInput_;
    lv_obj_t* kb_freq_;
    lv_obj_t* kb_qwert_;
    lv_obj_t* fileName_container_;
    lv_obj_t* topLabel_container_;
    lv_obj_t* button_container1_;
    lv_obj_t* button_container2_;
    lv_obj_t* button_container_settings;
    lv_obj_t* C1101preset_container_;
    lv_obj_t* C1101preset_dropdown_;
    lv_obj_t* checkbox_container_;
    lv_obj_t* C1101PTK_container_;
    lv_obj_t* C1101PTK_dropdown_;
    lv_obj_t* C1101SYNC_container_;
    lv_obj_t* C1101SYNC_dropdown_;
    lv_obj_t *buttonSettings_;
};

#endif 