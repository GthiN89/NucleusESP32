#ifndef GLOBALS_H
#define GLOBALS_H

#include <RCSwitch.h>
#include <lvgl.h>
#include "GUI/menus/MainMenuScreen.h"
#include "GUI/menus/RFMenu.h"
#include "ELECHOUSE_CC1101_SRC_DRV.h"
   //---------------------------------------------------------------------------//
  //-----------------------------SETINGS---------------------------------------//
 //---------------------------------------------------------------------------//
// Pin configuration for CC1101
#define CC1101_CS   21  // Chip Select
#define CC1101_MOSI 4   // Master Out Slave In
#define CC1101_MISO 35  // Master In Slave Out
#define CC1101_SCLK 22  // Serial Clock

#define CCGDO0A 17      // GDO0
#define CCGDO2A 16      // GDO2

#define CC1101_CCGDO0A 17      // GDO0
#define CC1101_CCGDO2A 16      // GDO2

   //---------------------------------------------------------------------------//
  //-----------------------------ENUMBS----------------------------------------//
 //---------------------------------------------------------------------------//
// C1101 Presets

enum CC1101_PRESET {
    AM650,
    AM270,
    FM238,
    FM476,
    FM95,
    FM15k,
    PAGER,
    HND1,
    HND2,
    CUSTOM
};

extern CC1101_PRESET C1101preset;
// C1101 state mashine
enum C1101State
{
  STATE_IDLE,
  STATE_GENERATOR,
  STATE_ANALYZER,
  STATE_SCANNER,
  STATE_CAPTURE,
  STATE_PLAYBACK,
  STATE_TESLA_US,
  STATE_TESLA_EU,
  STATE_AUDIO_TEST,
  STATE_SEND_FLIPPER,
  STATE_WIFI_SCAN,
  STATE_SEND_BLESPAM,
};

// Current State
extern uint8_t C1101CurrentState;

extern bool C1101LoadPreset;


extern bool receiverEnabled;

extern float CC1101_MHZ;

// C1101 Presets


// Function declarations
typedef void (*TouchCallback)();




#endif // GLOBALS_H
