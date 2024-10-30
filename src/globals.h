#ifndef GLOBALS_H
#define GLOBALS_H

#include <RCSwitch.h>
#include <lvgl.h>
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

extern bool teslaSucessFlag;

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
  STATE_PULSE_SCAN,
  STATE_BRUTE,
};

// Current State
extern uint8_t C1101CurrentState;

extern bool C1101LoadPreset;
extern bool receiverEnabled;
extern float CC1101_MHZ;

enum BTState
{
  STATE_SOUR_APPLE_IDLE,
  STATE_SOUR_APPLE,
  STATE_BTSPAM,
  STATE_BT_IDDLE,
};

// Current State
extern uint8_t BTCurrentState;

//////////////////////////////////////////////////////////////////////////
//FLAGS
///////////////////
extern bool CC1101_init; 
extern bool CC1101_RCSwitch_init; 
extern bool CC1101_RCSwitch_listen;
extern bool CC1101_RCSwitch_play;

// Function declarations
typedef void (*TouchCallback)();

CC1101_PRESET convert_str_to_enum(const char * selected_str);
const char* presetToString(CC1101_PRESET preset);

extern char* current_dir;
extern char* selected_file;
extern lv_obj_t* selected_label;
extern lv_obj_t* selected_btn;



#endif // GLOBALS_H
