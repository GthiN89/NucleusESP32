// everything shared globally + settings

#include "globals.h"
#include "GUI/menus/MainMenuScreen.h"  // Include before using MainMenuScreen


uint8_t C1101CurrentState = STATE_IDLE;
uint8_t BTCurrentState = STATE_IDLE;

char* current_dir;
char* selected_file;
lv_obj_t* selected_label;
lv_obj_t* selected_btn;



bool C1101LoadPreset = true;

bool receiverEnabled;

float CC1101_MHZ;

bool teslaSucessFlag;

RFMenuScreen rfMenuScreen;


////////////////////////////////////////
//FLAGS
//////////////////
bool CC1101_init  = false;
bool CC1101_RCSwitch_init  = false;
bool CC1101_RCSwitch_listen = false;
bool CC1101_RCSwitch_play    = false;




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