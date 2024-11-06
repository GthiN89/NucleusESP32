// everything shared globally + settings

#include "globals.h"

char* current_dir;
//char* selected_file;

uint8_t C1101CurrentState = STATE_IDLE;
uint8_t BTCurrentState = STATE_IDLE;



CC1101_PRESET C1101preset = AM650;



bool C1101LoadPreset = true;

bool receiverEnabled;

float CC1101_MHZ;

bool teslaSucessFlag;




////////////////////////////////////////
//FLAGS
//////////////////
bool CC1101_init  = false;
bool CC1101_RCSwitch_init  = false;
bool CC1101_RCSwitch_listen = false;
bool CC1101_RCSwitch_play    = false;
extern bool bruteIsRunning = false;

extern uint16_t bruteCounter = 0;




CC1101_PRESET convert_str_to_enum(const char * selected_str) {
    if (strcmp(selected_str, "AM650") == 0) return AM650;
    else if (strcmp(selected_str, "AM270") == 0) return AM270;
    else if (strcmp(selected_str, "FM238") == 0) return FM238;
    else if (strcmp(selected_str, "FM476") == 0) return FM476;
    else if (strcmp(selected_str, "FM95") == 0) return FM95;
    else if (strcmp(selected_str, "FM15k") == 0) return FM15k;
    else if (strcmp(selected_str, "PAGER") == 0) return PAGER;
    else if (strcmp(selected_str, "HND1") == 0) return HND1;
    else if (strcmp(selected_str, "HND2") == 0) return HND2;
    else return CUSTOM;  // Default to CUSTOM if no match is found
}

const char* presetToString(CC1101_PRESET preset) {
    switch (preset) {
        case AM650: return "AM650";
        case AM270: return "AM270";
        case FM238: return "FM238";
        case FM476: return "FM476";
        case FM95:  return "FM95";
        case FM15k: return "FM15k";
        case PAGER: return "PAGER";
        case HND1:  return "HND1";
        case HND2:  return "HND2";
        default:    return "Unknown";
    }
}