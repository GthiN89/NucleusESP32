#ifndef C1101_SCAN_H
#define C1101_SCAN_H

#include <lvgl.h>              
#include <SD.h>               
//#include "modules/RF/subGhz.h" 

extern lv_obj_t* C1101scanCont;
extern lv_obj_t* c1101scanScreen;
extern lv_obj_t* txtProtscanReceived;

extern int ProtscanRxEn;

void create1101scanScreen();
void CC1101Scanloop();
void output(unsigned long decimal, unsigned int length, unsigned int pktdelay, unsigned int* raw, unsigned int protocol);
void fcnProtscanCancel(lv_event_t * e);
void fcnProtscanRxEn(lv_event_t * e);
void fcnProtscanRxOff(lv_event_t * e);
void fcnProtscanClear(lv_event_t * e);





#endif // SEND_TESLA_H