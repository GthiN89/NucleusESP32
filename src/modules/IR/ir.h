#ifndef IR_READ_H
#define IR_READ_H

#include <IRrecv.h>
//#include "modules/IR/TV-B-Gone.h"  // Adjust as necessary
#include "globals.h"
#include "GUI/ScreenManager.h"
#include "GUI/events.h"
#include "modules/ETC/SDcard.h"
#include <vector>

// IR TX and RX Pins
#define IR_TX 26
#define IR_RX 34

// IR States
enum IRState {
    IDLE,
    LISTENING,
    SIGNAL_RECEIVED,
    SIGNAL_SAVED,
    SIGNAL_PLAYBACK
};

class IrRead {
public:
    IrRead();

    // Arduino lifecycle
    void setup();
    void loop();

    // Static UI event handlers
    static void btn_event_IR_run(lv_event_t* e);
    static void save_signal_event(lv_event_t* e);
    static void play_signal_event(lv_event_t* e);
    static void exit_event(lv_event_t* e);

private:
    // IR receiver
    IRrecv irrecv;
    decode_results results;

    // State machine
    IRState currentState;

    // Signal storage
    std::vector<uint16_t> signalBuffer;
    String signalData;

    // UI Components
    lv_obj_t* text_area;

    // SD card interface
    SDcard& sdCard;

    // Core functions
    void begin();
    void read_signal();
    void save_signal();
    void play_signal();
    void reset();

    // UI helper functions
    void setTextToTextarea(const char* message);
};

#endif // IR_READ_H
