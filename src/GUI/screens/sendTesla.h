#ifndef SEND_TESLA_H
#define SEND_TESLA_H

#include <lvgl.h>              
#include <SDfat.h>               
//#include "modules/RF/subGhz.h" 

// External declarations for global variables
extern lv_obj_t* tesla_screen;
extern bool teslaRunningFlag;
extern lv_obj_t* teslaCont;
extern int teslaTextLine;
extern lv_obj_t** labels;

// Tesla button configuration constants
const uint16_t pulseWidth = 400;
const uint16_t messageDistance = 23;
const uint8_t transmtesla = 5;
const uint8_t messageLength = 43;
extern void btn_event_teslaCharger_run(lv_event_t* e);


const uint8_t sequence[messageLength] = { 
  0x02,0xAA,0xAA,0xAA,  // Preamble of 26 bits by repeating 1010
  0x2B,                 // Sync byte
  0x2C,0xCB,0x33,0x33,0x2D,0x34,0xB5,0x2B,0x4D,0x32,0xAD,0x2C,0x56,0x59,0x96,0x66,
  0x66,0x5A,0x69,0x6A,0x56,0x9A,0x65,0x5A,0x58,0xAC,0xB3,0x2C,0xCC,0xCC,0xB4,0xD2,
  0xD4,0xAD,0x34,0xCA,0xB4,0xA0
};

// Function declarations
void teslaScreen();
void createTeslaScreenContainer(lv_obj_t* tesla_screen);
void addLineToTeslaContainer(lv_obj_t* teslaCont, String text);
void createTeslaButtons(lv_obj_t* teslaCont);
void sendByte(uint8_t dataByte);
void sendBits(int *buff, int length, int gdo0);
bool sendTeslaSignal(lv_obj_t* teslaCont, float freq);
void play_tesla_btn_event_cb(lv_event_t* e);  

#endif // SEND_TESLA_H
