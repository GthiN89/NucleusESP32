// File: src/GUI/events.h

#ifndef EVENTS_H
#define EVENTS_H

#include <lvgl.h>
#include <RCSwitch.h>
#include "globals.h"
#include "main.h"

#define EVENTS_MAX_PATH_LENGTH 64

class EVENTS {
public:

static lv_obj_t* list;
static lv_obj_t* selected_label;
static lv_obj_t* selected_btn;
static lv_obj_t* container;
static char selected_file;
static char frequency_buffer[10];
static char selected_str[32]; 



// Deklarace event handlerů
static void btn_event_subGhzTools(lv_event_t * e);
static void btn_event_playZero_run(lv_event_t* e);
static void back_btn_event_cb(lv_event_t* e);
static void file_btn_event_cb(lv_event_t* e);
static void load_btn_event_cb(lv_event_t* e);
static void useSelectedFile(const char* filepath);
static void btn_event_teslaCharger_run(lv_event_t* e);
static void btn_event_Replay_run(lv_event_t* e);
static void btn_event_playRecorded_run(lv_event_t* e);
static void btn_event_c1101Scanner_run(lv_event_t* e);

// Přidání nových event handlerů
static void ta_freq_event_cb(lv_event_t * e);
static void ta_filename_event_cb(lv_event_t * e);
static void kb_event_cb(lv_event_t * e);
static void kb_qwert_event_cb(lv_event_t * e);
static void saveSignal(lv_event_t * e);
static void saveSignalProcess(lv_event_t * e);
static void listenRFEvent(lv_event_t * e);
static void fcnProtAnaPlay(lv_event_t * e);
static void ta_preset_event_cb(lv_event_t * e);
static void saveRFSettingEvent(lv_event_t * e);
static void cancelRFSettingEvent(lv_event_t * e);
static void replayEvent(lv_event_t * e);
static void exitReplayEvent(lv_event_t * e);

static void playFromBuffer();
static void radioHandlerOnChange();
static void ProtAnalyzerloop();
static bool initCC1101();
bool recievedSubGhz;
static CC1101_PRESET stringToCC1101Preset(String presetStr);

private:

};

#endif // EVENTS_H
