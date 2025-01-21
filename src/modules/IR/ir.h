#ifndef IR_H
#define IR_H

#include <string>
#include <stdint.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>

#define IR_TX 26
#define IR_RX 34

enum IRState {
    IR_STATE_IDLE,
    IR_STATE_LISTENING,
    IR_STATE_RECEIVED,
    IR_STATE_SAVED,
    IR_STATE_PLAYBACK,
    IR_STATE_BGONE
};

extern IRState IRCurrentState;

class IR_CLASS {
public:

    struct irSignal {
        std::string signal;
        std::uint16_t protocol;
        std::uint16_t TYPE;
        std::uint16_t address;
        std::uint16_t command;
    };

    struct allSignals {
        std::vector<irSignal> signals;
    };
    void setupIR();
    void receiveIR();
    void sendPower();
    void sendVolumeUp();
    void sendVolumeDown();
    void sendMute();
    void sendChannelUp();
    void sendChannelDown();

private:

    irSignal SamsungPower {
        "Power",
        0x0001, 
        0x007F,  
        0x0000,  
        0x0000,  
    };

    irSignal SIRCPower {
        "Power",
        0x0001, 
        0x0002,  
        0x0000,  
        0x0000,  
    };

    irSignal NECPower {
        "Power",
        0x0001, 
        0x0003,  
        0x0000,  
        0x0000,  
    };

    irSignal NECextPower {
        "Power",
        0x0001, 
        0x0004,  
        0x0000,  
        0x0000,  
    };

    irSignal NEC42Power {
        "Power",
        0x0001, 
        0x0005,  
        0x0000,  
        0x0000,  
    };

    irSignal KaseikyoPower {
        "Power",
        0x0001, 
        0x0006,  
        0x0000,  
        0x0000,  
    };

    irSignal RC5Power {
        "Power",
        0x0001, 
        0x0007,  
        0x0000,  
        0x0000,  
    };

    irSignal RC6Power {
        "Power",
        0x0001, 
        0x0008,  
        0x0000,  
        0x0000,  
    };

    irSignal PioneerPower {
        "Power",
        0x0001, 
        0x0009,  
        0x0000,  
        0x0000,  
    };

    irSignal RCAPower {
        "Power",
        0x0001, 
        0x000A,  
        0x0000,  
        0x0000,  
    };

    irSignal SamsungVolUp {
        "Vol_up",
        0x0001, 
        0x007F,  
        0x0000,  
        0x0000,  
    };

    irSignal SIRCVolUp {
        "Vol_up",
        0x0001, 
        0x0002,  
        0x0000,  
        0x0000,  
    };

    irSignal NECVolUp {
        "Vol_up",
        0x0001, 
        0x0003,  
        0x0000,  
        0x0000,  
    };

    irSignal NECextVolUp {
        "Vol_up",
        0x0001, 
        0x0004,  
        0x0000,  
        0x0000,  
    };

    irSignal RC5VolUp {
        "Vol_up",
        0x0001, 
        0x0007,  
        0x0000,  
        0x0000,  
    };

    irSignal RC6VolUp {
        "Vol_up",
        0x0001, 
        0x0008,  
        0x0000,  
        0x0000,  
    };

    irSignal PioneerVolUp {
        "Vol_up",
        0x0001, 
        0x0009,  
        0x0000,  
        0x0000,  
    };

    irSignal RCARC6VolUp {
        "Vol_up",
        0x0001, 
        0x000A,  
        0x0000,  
        0x0000,  
    };

    irSignal SamsungVolDn {
        "Vol_dn",
        0x0001, 
        0x007F,  
        0x0000,  
        0x0000,  
    };

    irSignal SIRCVolDn {
        "Vol_dn",
        0x0001, 
        0x0002,  
        0x0000,  
        0x0000,  
    };

    irSignal NECVolDn {
        "Vol_dn",
        0x0001, 
        0x0003,  
        0x0000,  
        0x0000,  
    };

    irSignal NECextVolDn {
        "Vol_dn",
        0x0001, 
        0x0004,  
        0x0000,  
        0x0000,  
    };

    irSignal RC5VolDn {
        "Vol_dn",
        0x0001, 
        0x0007,  
        0x0000,  
        0x0000,  
    };

    irSignal RC6VolDn {
        "Vol_dn",
        0x0001, 
        0x0008,  
        0x0000,  
        0x0000,  
    };

    irSignal PioneerVolDn {
        "Vol_dn",
        0x0001, 
        0x0009,  
        0x0000,  
        0x0000,  
    };

    irSignal RCARC6VolDn {
        "Vol_dn",
        0x0001, 
        0x000A,  
        0x0000,  
        0x0000,  
    };

    irSignal SamsungMute {
        "Mute",
        0x0001, 
        0x007F,  
        0x0000,  
        0x0000,  
    };

    irSignal SIRCMute {
        "Mute",
        0x0001, 
        0x0002,  
        0x0000,  
        0x0000,  
    };

    irSignal NECMute {
        "Mute",
        0x0001, 
        0x0003,  
        0x0000,  
        0x0000,  
    };

    irSignal NECextMute {
        "Mute",
        0x0001, 
        0x0004,  
        0x0000,  
        0x0000,  
    };

    irSignal RC5Mute {
        "Mute",
        0x0001, 
        0x0007,  
        0x0000,  
        0x0000,  
    };

    irSignal RC6Mute {
        "Mute",
        0x0001, 
        0x0008,  
        0x0000,  
        0x0000,  
    };

    irSignal PioneerMute {
        "Mute",
        0x0001, 
        0x0009,  
        0x0000,  
        0x0000,  
    };

    irSignal RCARC6Mute {
        "Mute",
        0x0001, 
        0x000A,  
        0x0000,  
        0x0000,  
    };

    irSignal SamsungChNext {
        "Ch_next",
        0x0001, 
        0x007F,  
        0x0000,  
        0x0000,  
    };

    irSignal SIRCChNext {
        "Ch_next",
        0x0001, 
        0x0002,  
        0x0000,  
        0x0000,  
    };

    irSignal NECChNext {
        "Ch_next",
        0x0001, 
        0x0003,  
        0x0000,  
        0x0000,  
    };

    irSignal NECextChNext {
        "Ch_next",
        0x0001, 
        0x0004,  
        0x0000,  
        0x0000,  
    };

    irSignal RC5ChNext {
        "Ch_next",
        0x0001, 
        0x0007,  
        0x0000,  
        0x0000,  
    };

    irSignal RC6ChNext {
        "Ch_next",
        0x0001, 
        0x0008,  
        0x0000,  
        0x0000,  
    };

    irSignal PioneerChNext {
        "Ch_next",
        0x0001, 
        0x0009,  
        0x0000,  
        0x0000,  
    };

    irSignal RCARC6ChNext {
        "Ch_next",
        0x0001, 
        0x000A,  
        0x0000,  
        0x0000,  
    };


    irSignal SamsungChPrev {
        "Ch_prev",
        0x0001, 
        0x007F,  
        0x0000,  
        0x0000,  
    };

    irSignal SIRCChPrev {
        "Ch_prev",
        0x0001, 
        0x0002,  
        0x0000,  
        0x0000,  
    };

    irSignal NECChPrev {
        "Ch_prev",
        0x0001, 
        0x0003,  
        0x0000,  
        0x0000,  
    };

    irSignal NECextChPrev {
        "Ch_prev",
        0x0001, 
        0x0004,  
        0x0000,  
        0x0000,  
    };

    irSignal RC5ChPrev {
        "Ch_prev",
        0x0001, 
        0x0007,  
        0x0000,  
        0x0000,  
    };

    irSignal RC6ChPrev {
        "Ch_prev",
        0x0001, 
        0x0008,  
        0x0000,  
        0x0000,  
    };

    irSignal PioneerChPrev {
        "Ch_prev",
        0x0001, 
        0x0009,  
        0x0000,  
        0x0000,  
    };

    irSignal RCARC6ChPrev {
        "Ch_prev",
        0x0001, 
        0x000A,  
        0x0000,  
        0x0000,  
    };

    std::vector<irSignal> power = {
        {"Power", SAMSUNG, 0x007F, 0x0000, 0x0000}, // SamsungPower
        {"Power", NEC, 0x0003, 0x0000, 0x0000}, // NECPower
        {"Power", RC5, 0x0007, 0x0000, 0x0000}, // RC5Power
        {"Power", RC6, 0x0008, 0x0000, 0x0000}, // RC6Power
        {"Power", NEC, 0x0009, 0x0000, 0x0000}, // PioneerPower
    }; 
    // Vol_up signals
    std::vector<irSignal> volUp = {
        {"Vol_up", 0x0001, 0x007F, 0x0000, 0x0000}, // SamsungVolUp
        {"Vol_up", 0x0001, 0x0002, 0x0000, 0x0000}, // SIRCVolUp
        {"Vol_up", 0x0001, 0x0003, 0x0000, 0x0000}, // NECVolUp
        {"Vol_up", 0x0001, 0x0004, 0x0000, 0x0000}, // NECextVolUp
        {"Vol_up", 0x0001, 0x0007, 0x0000, 0x0000}, // RC5VolUp
        {"Vol_up", 0x0001, 0x0008, 0x0000, 0x0000}, // RC6VolUp
        {"Vol_up", 0x0001, 0x0009, 0x0000, 0x0000}, // PioneerVolUp
        {"Vol_up", 0x0001, 0x000A, 0x0000, 0x0000}, // RCARC6VolUp
    };

    // Vol_dn signals
    std::vector<irSignal> volDn = {
        {"Vol_dn", 0x0001, 0x007F, 0x0000, 0x0000}, // SamsungVolDn
        {"Vol_dn", 0x0001, 0x0002, 0x0000, 0x0000}, // SIRCVolDn
        {"Vol_dn", 0x0001, 0x0003, 0x0000, 0x0000}, // NECVolDn
        {"Vol_dn", 0x0001, 0x0004, 0x0000, 0x0000}, // NECextVolDn
        {"Vol_dn", 0x0001, 0x0007, 0x0000, 0x0000}, // RC5VolDn
        {"Vol_dn", 0x0001, 0x0008, 0x0000, 0x0000}, // RC6VolDn
        {"Vol_dn", 0x0001, 0x0009, 0x0000, 0x0000}, // PioneerVolDn
        {"Vol_dn", 0x0001, 0x000A, 0x0000, 0x0000}, // RCARC6VolDn
    };

    // Mute signals
    std::vector<irSignal> mute = {
        {"Mute", 0x0001, 0x007F, 0x0000, 0x0000}, // SamsungMute
        {"Mute", 0x0001, 0x0002, 0x0000, 0x0000}, // SIRCMute
        {"Mute", 0x0001, 0x0003, 0x0000, 0x0000}, // NECMute
        {"Mute", 0x0001, 0x0004, 0x0000, 0x0000}, // NECextMute
        {"Mute", 0x0001, 0x0007, 0x0000, 0x0000}, // RC5Mute
        {"Mute", 0x0001, 0x0008, 0x0000, 0x0000}, // RC6Mute
        {"Mute", 0x0001, 0x0009, 0x0000, 0x0000}, // PioneerMute
        {"Mute", 0x0001, 0x000A, 0x0000, 0x0000}, // RCARC6Mute
    };

    // Ch_next signals
    std::vector<irSignal> chNext = {
        {"Ch_next", 0x0001, 0x007F, 0x0000, 0x0000}, // SamsungChNext
        {"Ch_next", 0x0001, 0x0002, 0x0000, 0x0000}, // SIRCChNext
        {"Ch_next", 0x0001, 0x0003, 0x0000, 0x0000}, // NECChNext
        {"Ch_next", 0x0001, 0x0004, 0x0000, 0x0000}, // NECextChNext
        {"Ch_next", 0x0001, 0x0007, 0x0000, 0x0000}, // RC5ChNext
        {"Ch_next", 0x0001, 0x0008, 0x0000, 0x0000}, // RC6ChNext
        {"Ch_next", 0x0001, 0x0009, 0x0000, 0x0000}, // PioneerChNext
        {"Ch_next", 0x0001, 0x000A, 0x0000, 0x0000}, // RCARC6ChNext
    };

    // Ch_prev signals
    std::vector<irSignal> chPrev = {
        {"Ch_prev", 0x0001, 0x007F, 0x0000, 0x0000}, // SamsungChPrev
        {"Ch_prev", 0x0001, 0x0002, 0x0000, 0x0000}, // SIRCChPrev
        {"Ch_prev", 0x0001, 0x0003, 0x0000, 0x0000}, // NECChPrev
        {"Ch_prev", 0x0001, 0x0004, 0x0000, 0x0000}, // NECextChPrev
        {"Ch_prev", 0x0001, 0x0007, 0x0000, 0x0000}, // RC5ChPrev
        {"Ch_prev", 0x0001, 0x0008, 0x0000, 0x0000}, // RC6ChPrev
        {"Ch_prev", 0x0001, 0x0009, 0x0000, 0x0000}, // PioneerChPrev
        {"Ch_prev", 0x0001, 0x000A, 0x0000, 0x0000}, // RCARC6ChPrev
    };
};

#endif
