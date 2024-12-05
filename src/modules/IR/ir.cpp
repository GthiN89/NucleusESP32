#include "ir.h"

ScreenManager& screenMgr = ScreenManager::getInstance();


// Constructor
IrRead::IrRead()
    : irrecv(IR_RX, SAFE_STACK_BUFFER_SIZE / 2, 50),  // Use IR_RX pin
      currentState(IDLE),
      text_area(nullptr),
      sdCard(SDcard::getInstance()) {}

// Setup method
void IrRead::setup() {
    if (!sdCard.initializeSD()) {
        Serial.println(F("SD Card initialization failed."));
    }

    irrecv.enableIRIn();  // Start the IR receiver
    ScreenManager::getInstance().createIRRecScreen();
    text_area = ScreenManager::getInstance().getTextArea();
    setTextToTextarea("Press Listen to start.");
}

// Main loop
void IrRead::loop() {
    if (currentState == LISTENING && irrecv.decode(&results)) {
        currentState = SIGNAL_RECEIVED;
        read_signal();
        irrecv.resume();  // Prepare for the next signal
    }
}

// Start listening for signals
void IrRead::begin() {
    if (!_read_signal) {
        irrecv.enableIRIn();
        _read_signal = true;
    }
}
// Read signal
void IrRead::read_signal() {
    if (results.decode_type != UNKNOWN) {
        signalBuffer.clear();
        signalData = "Signal Detected:\n";

        for (uint16_t i = 0; i < results.rawlen; i++) {
            signalBuffer.push_back(results.rawbuf[i]);
            signalData += String(results.rawbuf[i]) + " ";
        }

        setTextToTextarea(signalData.c_str());
    } else {
        setTextToTextarea("Unknown signal received.");
    }
}

// Save signal to SD card
bool IrRead::save_signal() {
    if (!_read_signal) return false;
    if (decodeIRSignal()) {
        return saveSignalToSD(); // Custom method for saving to SD
    }
    return false;
}

// Play the recorded signal
bool IrRead::save_signal() {
    if (!_read_signal) return false;
    if (decodeIRSignal()) {
        return saveSignalToSD(); // Custom method for saving to SD
    }
    return false;
}
// Reset the state machine and UI
void IrRead::reset() {
    if (_read_signal) {
        irrecv.disableIRIn();
        _read_signal = false;
    }
}
// Update text area in the UI
void IrRead::setTextToTextarea(const char* message) {
    if (text_area) {
        lv_textarea_set_text(text_area, message);
    }
}


void IrRead::btn_event_IR_run(lv_event_t* e) {
    // Start listening for IR signals
    irRead.begin();
    ScreenManager::getInstance().updateLabel("Listening for IR signals...");
}

void IrRead::save_signal_event(lv_event_t* e) {
    // Save the captured IR signal
    if (irRead.save_signal()) {
        ScreenManager::getInstance().updateLabel("Signal saved successfully.");
    } else {
        ScreenManager::getInstance().updateLabel("No signal to save.");
    }
}

void IrRead::play_signal_event(lv_event_t* e) {
    // Play the captured or saved IR signal
    if (irRead.play_signal()) {
        ScreenManager::getInstance().updateLabel("Signal played successfully.");
    } else {
        ScreenManager::getInstance().updateLabel("No signal to play.");
    }
}

void IrRead::exit_event(lv_event_t* e) {
    // Reset and return to the RF Menu
    irRead.reset();
    ScreenManager::getInstance().updateLabel("Exiting IR receiver...");
    ScreenManager::getInstance().createRFMenu();
}