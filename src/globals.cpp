// everything shared globally + settings

#include "globals.h"

#include "SPI.h"
char* current_dir;
//char* selected_file;

uint8_t C1101CurrentState = STATE_IDLE;

uint8_t BruteCurrentState = BRUTE_IDLE;
EncoderStep encoderState = EncoderStepIddle;
std::vector<int64_t> samplesToSend;
int8_t repeats = 1;

float CC1101_MHZ;

bool teslaSucessFlag;

uint8_t runningModule = MODULE_NONE;
bool isRaw = false;



NFCState nfcState = NFC_IDLE;


CC1101_PRESET  C1101preset = AM650;


    State currentState = IDLE;



void setState(State newState) {
    currentState = newState;
}

CC1101_PRESET convert_str_to_enum(const char * selected_str) {
    if (strcmp(selected_str, "AM650") == 0) return AM650;
    else if (strcmp(selected_str, "AM270") == 0) return AM270;
    else if (strcmp(selected_str, "FM238") == 0) return FM238;
    else if (strcmp(selected_str, "FM476") == 0) return FM476;
    else if (strcmp(selected_str, "FM95") == 0) return FM95;
    else if (strcmp(selected_str, "FSK12k") == 0) return FSK12k;
    else if (strcmp(selected_str, "FM15k") == 0) return FM15k;
    else if (strcmp(selected_str, "FSK25k") == 0) return FSK25k;
    else if (strcmp(selected_str, "FSK31k") == 0) return FSK31k;
    else if (strcmp(selected_str, "PAGER") == 0) return PAGER;
    else if (strcmp(selected_str, "HND1") == 0) return HND1;
    else if (strcmp(selected_str, "HND2") == 0) return HND2;
    return CUSTOM;

}

const char* presetToString(CC1101_PRESET preset) {
    switch (preset) {
        case AM650: return "AM650";
        case AM270: return "AM270";
        case FM238: return "FM238";
        case FM476: return "FM476";
        case FM95:  return "FM95";
        case FSK12k: return "FSK12k";
        case FM15k: return "FM15k";
        case FSK25k: return "FSK25k";
        case FSK31k: return "FSK31k";
        case PAGER: return "PAGER";
        case HND1:  return "HND1";
        case HND2:  return "HND2";
        default:    return "Unknown";
    }
}

const uint8_t* presetTobyteArray(CC1101_PRESET preset) {
    switch (preset) {
        case AM650: return AM650_regs;
        case AM270: return AM270_regs;
        case FM238: return FM238_regs;
        case FM476: return FM476_regs;
        case FM95:  return FM95_regs;
        case FM15k: return FM15k_regs;
        default:    return AM650_regs; 
}
}

