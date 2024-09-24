#ifndef REPLAY_H
#define REPLAY_H

#include "lvgl.h"

// Externální buffery
extern char frequency_buffer[10];
extern char filename_buffer[32];

// Flags
extern bool ProtAnaRxEn;


// RF Modules
// extern RCSwitch mySwitch;
// extern CC1101 cc1101;


/* Function to create the c1101 analysis screen */
void create_c1101_screen(void);

/* Event handler functions for the buttons */
//void fcnProtAnaRxEn(lv_event_t * e);
void saveSignal(lv_event_t * e);
void fcnProtAnaPlay(lv_event_t * e);
//void btn_event_subGhzTools(lv_event_t * e);
void saveSignalProcess(lv_event_t * e);

extern bool ProtAnaRxEn;




 void ProtAnalyzerloop();


#endif /* REPLAY_H */

