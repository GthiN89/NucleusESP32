#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include "lvgl.h"
#include <functional>
#include "XPT2046_Bitbang.h"
#include "logo.h"
struct SubGHzScreen {
    lv_obj_t* screen;
    lv_obj_t* spinbox;
    lv_obj_t* spinbox1;
    lv_obj_t* spinbox2;
    lv_obj_t* spinbox3;
    lv_obj_t* spinbox4;
    lv_obj_t* spinbox5;
    lv_obj_t* spinbox6;
    lv_obj_t* spinbox7;
};

class ScreenManager {
public:

    int16_t bitLenght;
    int16_t spinbox_frequency_value;
    int16_t repeats;
    lv_obj_t* dropdown_1;
    lv_obj_t* dropdown_2;
    lv_obj_t* Encoder_dropdown_;
    SubGHzScreen* SubGHzCustomScreen_;
    lv_obj_t* topLabel_container_;
    lv_obj_t *mbox_container;
    lv_obj_t* appScreen_;
    lv_obj_t* detectLabel;
    lv_obj_t * BruteCounterLabel;
    lv_obj_t * spinbox_bitLenght;
    lv_obj_t * spinbox_frequency;
    lv_obj_t * spinbox_repeats; 

    static ScreenManager& getInstance();

    ScreenManager(const ScreenManager&) = delete;
    ScreenManager& operator=(const ScreenManager&) = delete;
    void apply_neon_theme_button(lv_obj_t * obj);
    void createReplayScreen();
    void createCustomSubghzScreen();
    void createmainMenu();
    void createRFMenu();
    void createRFdetectScreen();
    void createFileExplorerScreen();
    void createTeslaScreen();
    void createBruteScreen();
    void createIRMenuScreen();
    void createIRRecScreen();
    void init_neon_theme(void);
    void create_neon_theme(void);
    static void apply_neon_theme(lv_obj_t * obj);
    void draw_image();
    void createRFRemotesMenu();
    void createEncoderSreen();
    static void ta_event_cb(lv_event_t * e);

    lv_obj_t* customPreset;
    lv_obj_t* BitLengh;
    lv_obj_t *input_datarate;
    lv_obj_t *input_bandwidth;
    lv_obj_t *input_deviation;
    lv_obj_t *dropdown_modulation;
    lv_obj_t *text_area_IR;
    lv_obj_t *text_area_SubGHzCustom;
    lv_obj_t *textarea_encoder;
    lv_obj_t *frequency_spinbox;
    lv_obj_t* secondLabel_container_;
    lv_obj_t* quareLine_container;
    lv_obj_t* text_area_;

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



    static char selected_file_path[];
    static char selected_file_name[];
private:
    ScreenManager();
    ~ScreenManager();
    
    
    lv_obj_t* detectScreen_;
    lv_obj_t* fileExplorerScreen;


    lv_obj_t* filenameInput_;
    lv_obj_t* kb_freq_;
    lv_obj_t* kb_qwert_;
    lv_obj_t* fileName_container_;
    lv_obj_t * bitLenght_container_;
    lv_obj_t * repeats_container_;
    lv_obj_t * frequency_container_;

    
    lv_obj_t* browserButton_container_;
    lv_obj_t* browserButton2_container_;
    




    lv_obj_t* button_container1_;
    lv_obj_t* button_container2_;
    lv_obj_t* button_container_settings;
    lv_obj_t* C1101preset_container_;

    
    lv_obj_t* checkbox_container_;
    lv_obj_t* C1101PTK_container_;
    lv_obj_t* C1101SYNC_container_;
    lv_obj_t* C1101SYNC_dropdown_;
    lv_obj_t *buttonSettings_;
    lv_obj_t * C1101pulseLenght_container_;
    lv_obj_t * pulseLenghInput_;
    lv_obj_t * pulseGet_;
    lv_obj_t* SubPlayerScreen_;
    lv_obj_t* button_container_RCSwitchMethod1_;
    lv_obj_t* button_container_RCSwitchMethod2_; 

    lv_obj_t* topLabel_detectForce_container_;
    lv_obj_t* settingsButton_; 

    lv_obj_t* secondLabel_detectForce_container_;

    lv_obj_t* file_explorer_;

};

#endif 