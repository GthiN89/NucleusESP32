#ifndef SUBGHZ_H
#define SUBGHZ_H

#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <RCSwitch.h>
#include "globals.h"
#include "../../GUI/screens/sendTesla.h"

// CC1101 Modules - GDO Pins  (CSN are in the VSPI area)


// External declarations for global variables
extern volatile long last_RXmillis; // CC1101 Receive timer
extern volatile long last_micros;
extern String RXbuffer; // RX buffer
extern ELECHOUSE_CC1101 ELECHOUSE_cc1101;

// CC1101 parameters
extern float CC1101_MHZ_;
extern bool CC1101_TX;
extern int CC1101_MODULATION_;
extern int CC1101_DRATE;
extern float CC1101_RX_BW;
extern int CC1101_PKT_FORMAT_;
extern float CC1101_LAST_AVG_LQI;
extern float CC1101_LAST_AVG_RSSI;
extern int CC1101ScanEn;

// Buffers for Flipper Sub Player
extern char filebuffer[32];
extern char folderbuffer[32];

//tesla stuff
extern bool teslaMenuIsOpen;
extern bool teslaSucessFlag_;
//extern lv_obj_t* cont;



// Function declarations
bool initCC1101();
bool transmitFlipperFile(String filename, bool transmit);
bool sendSamples(int samples[], int samplesLength);
void handleFlipperCommandLine(String command, String value);
bool setZeroName(char* current_dir, char* selected_file);

//void addLineToTeslaContainer(lv_obj_t **cont, lv_obj_t **labels, int teslaTextLine, String text);
//extern void addLineToTeslaContainer(lv_obj_t *teslaCont, lv_obj_t **labels, int teslaTextLine, String text);



#endif // SUBGHZ_H
