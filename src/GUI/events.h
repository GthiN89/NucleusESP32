// File: src/GUI/events.h

#ifndef EVENTS_H
#define EVENTS_H

#include <lvgl.h>
//#include <RCSwitch.h>
#include "globals.h"
#include "main.h"

#define EVENTS_MAX_PATH_LENGTH 64

extern int SpamDevice;

class EVENTS {

public:

static lv_obj_t* list;
static lv_obj_t* container;
static char frequency_buffer[10];
static char selected_str[32]; 
static char selected_file_path[LV_FS_MAX_PATH_LENGTH];


// Deklarace event handlerů
static void btn_event_subGhzTools(lv_event_t * e);
static void btn_event_playZero_run(lv_event_t* e);
static void useSelectedFile(const char* filepath);
static void btn_event_teslaCharger_run(lv_event_t* e);
static void btn_event_Replay_run(lv_event_t* e);
static void btn_event_mainMenu_run(lv_event_t* e);
static void btn_event_RAW_REC_run(lv_event_t* e);
static void ta_preset_event_cb(lv_event_t * e);
static void btn_event_BTTools(lv_event_t * e);
static void btn_event_SourApple(lv_event_t * e);
static void btn_event_SourApple_Start(lv_event_t * e);
static void btn_event_SourApple_Stop(lv_event_t * e);
static void btn_event_BTSpam(lv_event_t * e);
static void btn_event_BTSpam_Start(lv_event_t * e);
static void btn_event_BTSpam_Stop(lv_event_t * e);
static void btn_event_BruteForce_run(lv_event_t* e);
static void btn_event_brute_run(lv_event_t* e);
static void ta_rf_type_event_cb(lv_event_t * e);

static void load_btn_event_cb_sub(lv_event_t* e);
static void back_btn_event_cb_sub(lv_event_t* e);
static void file_btn_event_cb_sub(lv_event_t* e);


// Přidání nových event handlerů
static void ta_freq_event_cb(lv_event_t * e);
static void ta_filename_event_cb(lv_event_t * e);
static void kb_event_cb(lv_event_t * e);
static void kb_qwert_event_cb(lv_event_t * e);
static void saveSignal(lv_event_t * e);
static void replayEvent(lv_event_t * e);
static void exitReplayEvent(lv_event_t * e);
static void sendCapturedEvent(lv_event_t * e);
static void save_RF_to_sd_event(lv_event_t * e); 
static void delete_btn_event_cb_sub(lv_event_t* e);
static void confirm_delete_event_handler(lv_event_t * e);
void btn_event_handler(lv_event_t *e);
void dd_event_handler(lv_event_t *e);
static bool deleteFile(const char *path);

void file_explorer_event_handler(lv_event_t * e);



static void ProtAnalyzerloop();
static bool recievedSubGhz;

static CC1101_PRESET stringToCC1101Preset(String presetStr);

private:

};

#endif // EVENTS_H
