#ifndef IR_H
#define IR_H

#include <string>
#include <stdint.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>
#include "codes.h"
#include <vector>


#define IR_TX 26
#define IR_RX 34

#define freq_to_timerval(x) (x / 1000)
//Codes captured from Generation 3 TV-B-Gone by Limor Fried & Mitch Altman
//table of POWER codes

// The structure of compressed code entries




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
    uint8_t i;

    uint8_t bitsleft_r = 0;
    uint8_t bits_r = 0;
    uint8_t code_ptr;

    const IrCode * powerCode = EUpowerCodes[i];


    void setupIR();
    void receiveIR();

    void TVbGONE();
    void sendPower();
    void sendVolumeUp();
    void sendVolumeDown();
    void sendMute();
    void sendChannelUp();
    void sendChannelDown();
    

private:
    uint8_t read_bits(uint8_t count);

};

#endif
