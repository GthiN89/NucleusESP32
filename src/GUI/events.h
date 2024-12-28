// File: src/GUI/events.h

#ifndef EVENTS_H
#define EVENTS_H

#include <lvgl.h>
//#include <RCSwitch.h>
#include "main.h"
extern bool updatetransmitLabel;
extern bool stopTransmit;

#define EVENTS_MAX_PATH_LENGTH 64

extern int SpamDevice;
extern lv_obj_t* label_sub;
extern bool isWarmupStarted;
extern bool stopTransmiting;
class EVENTS {

public:

static lv_obj_t* list;
static lv_obj_t* container;
static char* selected_file;

//subGhz
static char frequency_buffer[10];
static char selected_str[32];
//file explorer
static String cur_path;
static String sel_fn;

static lv_obj_t* selected_item;

static char* fullPath;

// Deklarace event handlerů
static void btn_event_subGhzTools(lv_event_t * e);
static void btn_event_playZero_run(lv_event_t* e);
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
static void btn_event_detectForce_run(lv_event_t* e);
static void btn_event_detect_run(lv_event_t* e);
static void ta_rf_type_event_cb(lv_event_t * e);
static void warmup();
static void dropdown_modulation_event_cb(lv_event_t *e);
static void ok_button_event_cb(lv_event_t *e);
static void sendTesla(lv_event_t *e);
static void btn_event_IR_menu_run(lv_event_t *e);
static void btn_event_UR_BGONE(lv_event_t *e);
static void btn_event_IR_START_READ(lv_event_t * e);
static void btn_event_IR_run(lv_event_t* e);
static void sendCapturedIREvent(lv_event_t* e);
static void btn_event_NFC_menu_run(lv_event_t* e);
static void btn_event_RF24_menu_run(lv_event_t* e);
static void btn_event_IR_replay(lv_event_t* e);

static void ta_freq_event_cb(lv_event_t * e);
static void ta_filename_event_cb(lv_event_t * e);
static void kb_event_cb(lv_event_t * e);
static void kb_qwert_event_cb(lv_event_t * e);
static void saveSignal(lv_event_t * e);
static void replayEvent(lv_event_t * e);
static void exitReplayEvent(lv_event_t * e);
static void sendCapturedEvent(lv_event_t * e);
static void save_RF_to_sd_event(lv_event_t * e);

//file Explorer
static void confirm_delete_event_handler(lv_event_t * e);
static bool deleteFile(const char *path);
static void file_explorer_event_delete_cb(lv_event_t * e);
static void file_explorer_event_handler(lv_event_t * e);
static void confirm__explorer_play_sub_cb(lv_event_t * e);
static void close_explorer_play_sub_cb(lv_event_t * e);
static void file_explorer_play_sub();
static void CC1101TransmitTask(void* pvParameters);
static void initLabelUpdater();
static void labelUpdateTask(void* parameter);
static lv_obj_t * spinbox;

// Define a list of concrete values for the spinbox
static int32_t valuesDrate[];
static int32_t current_indexDrate;
static const int32_t value_countDrate;

static void lv_spinbox_set_value_from_index();
static void lv_spinbox_increment_event_cb(lv_event_t * e);

static void lv_spinbox_decrement_event_cb(lv_event_t * e);

static bool recievedSubGhz;



private:

};

#endif // EVENTS_H


