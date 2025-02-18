#include "AnsonicProtocol.h"
#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include "lvgl.h"
#include "GUI/ScreenManager.h"
#include "globals.h"


AnsonicProtocol::AnsonicProtocol()
    : te_short(555),
      te_long(1111),
      te_delta(120),
    binaryValue(0),
      min_count_bit(12),
      state(StepReset),
      decodeData(0),
      decodeCountBit(0),
      te_last(0),
      validCodeFound(false),
      finalCode(0),
      finalBitCount(0) {
    Serial.println("AnsonicProtocol: Constructed.");
}

//
// Reset the decoder state.
//
void AnsonicProtocol::reset() {
    state = StepReset;
    decodeData = 0;
    decodeCountBit = 0;
    te_last = 0;
    validCodeFound = false;
    finalCode = 0;
    finalBitCount = 0;
    Serial.println("AnsonicProtocol: Reset state.");
}

void AnsonicProtocol::toBits(unsigned int hexValue) {
    binaryValue = std::bitset<12>(hexValue);
}


//
// Add a bit to the decoded data.
//
void AnsonicProtocol::addBit(uint8_t bit) {
    decodeData = (decodeData << 1) | bit;
    decodeCountBit++;
    Serial.print("Bit added: ");
    Serial.print(bit);
    Serial.print(", decodeData now: 0x");
    Serial.println(decodeData, HEX);
}

void AnsonicProtocol::yield(uint32_t hexValue) {
    Serial.print(F("yield() called with hexValue: "));
    Serial.println(hexValue);
    
    switch (encoderState) {
    case EncoderStepStart:
        Serial.println(F("State: EncoderStepStart"));
        Serial.println(F("Clearing samplesToSend and converting hexValue to 12-bit binary."));
        samplesToSend.clear();
        binaryValue = std::bitset<12>(hexValue);
        Serial.print(F("Binary representation: "));
        Serial.println(binaryValue.to_string().c_str());
        encoderState = EncodeStepStartBit;
        Serial.println(F("Transition to state: EncodeStepStartBit"));
        break;
    
    case EncodeStepStartBit:
        Serial.println(F("State: EncodeStepStartBit"));
        Serial.print(F("Pushing start bit (te_short): "));
        Serial.println(te_short);
        samplesToSend.push_back(te_short);       // Start bit HIGH pulse
        encoderState = EncoderStepDurations;
        Serial.println(F("Transition to state: EncoderStepDurations"));
        break;

    case EncoderStepDurations:
        Serial.println(F("State: EncoderStepDurations"));
        Serial.println(F("Encoding 12-bit binary into durations (first pass):"));
        for (size_t i = 0; i < 12; i++) {
            Serial.print(F("Bit "));
            Serial.print(i);
            Serial.print(F(" ("));
            Serial.print(binaryValue[i] ? "1" : "0");
            Serial.println(F(")"));
            if (binaryValue[i]) {
                Serial.print(F("Pushing te_short: "));
                Serial.println(te_short);
                samplesToSend.push_back(te_short);
                Serial.print(F("Pushing te_long: "));
                Serial.println(te_long);
                samplesToSend.push_back(te_long);
            } else {
                Serial.print(F("Pushing te_long: "));
                Serial.println(te_long);
                samplesToSend.push_back(te_long);
                Serial.print(F("Pushing te_short: "));
                Serial.println(te_short);
                samplesToSend.push_back(te_short);
            }
        }
        Serial.print(F("Pushing end pulse (te_short * 4): "));
        Serial.println(te_short * 4);
        samplesToSend.push_back(te_short * 4);  // End pulse
        Serial.print(F("Pushing header LOW pulse (te_short * 35): "));
        Serial.println(te_short * 35);
        samplesToSend.push_back(te_short * 35);  // Header LOW pulse

        Serial.println(F("Encoding 12-bit binary into durations (second pass):"));
        for (size_t i = 0; i < 12; i++) {
            Serial.print(F("Bit "));
            Serial.print(i);
            Serial.print(F(" ("));
            Serial.print(binaryValue[i] ? "1" : "0");
            Serial.println(F(")"));
            if (binaryValue[i]) {
                Serial.print(F("Pushing te_short: "));
                Serial.println(te_short);
                samplesToSend.push_back(te_short);
                Serial.print(F("Pushing te_long: "));
                Serial.println(te_long);
                samplesToSend.push_back(te_long);
            } else {
                Serial.print(F("Pushing te_long: "));
                Serial.println(te_long);
                samplesToSend.push_back(te_long);
                Serial.print(F("Pushing te_short: "));
                Serial.println(te_short);
                samplesToSend.push_back(te_short);
            }
        }
        Serial.print(F("Pushing second end pulse (te_short * 4): "));
        Serial.println(te_short * 4);
        samplesToSend.push_back(te_short * 4);  // End pulse
        Serial.print(F("Pushing second header LOW pulse (te_short * 35): "));
        Serial.println(te_short * 35);
        samplesToSend.push_back(te_short * 35);  // Header LOW pulse

        encoderState = EncoderStepReady;
        Serial.println(F("Transition to state: EncoderStepReady"));
        break;

    default:
        Serial.println(F("State: Unknown"));
        break;
    }
}



//
// Feed a single pulse to the decoder.
// @param level: true for HIGH pulse, false for LOW pulse.
// @param duration: pulse duration in microseconds.
//
void AnsonicProtocol::feed(bool level, uint32_t duration) {
    Serial.print("Feed: level = ");
    Serial.print(level ? "HIGH" : "LOW");
    Serial.print(", duration = ");
    Serial.println(duration);

    switch(state) {
    case StepReset:
        Serial.println("StepReset: Looking for header...");
        if (!level && DURATION_DIFF(duration, te_short * 35) < te_delta * 35) {
            state = StepFoundStartBit;
            Serial.println("Header detected, moving to StepFoundStartBit.");
        }
        break;

    case StepFoundStartBit:
        if (!level) {
            Serial.println("StepFoundStartBit: Waiting for rising edge.");
            break;
        } else if (DURATION_DIFF(duration, te_short) < te_delta) {
            state = StepSaveDuration;
            decodeData = 0;
            decodeCountBit = 0;
            Serial.println("Start bit detected, moving to StepSaveDuration.");
        } else {
            state = StepReset;
            Serial.println("Invalid start bit duration, resetting state.");
        }
        break;

    case StepSaveDuration:
        if (!level) { // Falling edge: save interval.
            Serial.print("StepSaveDuration: LOW edge detected, duration = ");
            Serial.println(duration);
            if (duration >= (te_short * 4)) {
                state = StepFoundStartBit;
                Serial.println("Long gap detected, returning to StepFoundStartBit.");
                if (decodeCountBit >= min_count_bit) {
                    finalCode = decodeData;
                    finalBitCount = decodeCountBit;
                    validCodeFound = true;
                    Serial.print("Valid code found: 0x");
                    Serial.println(finalCode, HEX);
                }
                break;
            }
            te_last = duration;
            Serial.print("Storing te_last = ");
            Serial.println(te_last);
            state = StepCheckDuration;
        } else {
            state = StepReset;
            Serial.println("StepSaveDuration: Unexpected HIGH level, resetting state.");
        }
        break;

    case StepCheckDuration:
        if (level) {
            Serial.print("StepCheckDuration: HIGH edge detected, duration = ");
            Serial.println(duration);
            uint32_t diff1 = DURATION_DIFF(te_last, te_short);
            uint32_t diff2 = DURATION_DIFF(duration, te_long);
            Serial.print("Diff (te_last vs TE_SHORT): ");
            Serial.println(diff1);
            Serial.print("Diff (duration vs TE_LONG): ");
            Serial.println(diff2);
            if (diff1 < te_delta && diff2 < te_delta) {
                addBit(1);
                Serial.println("Bit '1' detected, adding bit.");
                state = StepSaveDuration;
            } else if (DURATION_DIFF(te_last, te_long) < te_delta &&
                       DURATION_DIFF(duration, te_short) < te_delta) {
                addBit(0);
                Serial.println("Bit '0' detected, adding bit.");
                state = StepSaveDuration;
            } else {
                state = StepReset;
                Serial.println("Duration pattern mismatch, resetting state.");
            }
        } else {
            state = StepReset;
            Serial.println("StepCheckDuration: Unexpected LOW level, resetting state.");
        }
        break;

    default:
        state = StepReset;
        break;
    }
}

//
// Decode an array of raw samples.
// Positive values represent HIGH pulses; negative values represent LOW pulses.
// Returns true if a valid code is found.
//
bool AnsonicProtocol::decode(long long int* samples, size_t sampleCount) {
    reset();
    for (size_t i = 0; i < sampleCount; i++) {
        if (samples[i] > 0) {
            feed(true, samples[i]);
        } else {
            feed(false, (uint32_t)(-samples[i]));
        }
        if (validCodeFound) {
            return true;
        }
    }
    return false;
}


String AnsonicProtocol::getCodeString() const {
    uint32_t data = finalCode;
    uint16_t cnt = data & 0xFFF;
    uint32_t btn = ((data >> 1) & 0x3);
    char buf[256];
    sprintf(buf,
            "%s %dbit\r\n"
            "Key:%03lX\r\n"
            "Btn:%X\r\n"
            "DIP:" DIP_PATTERN "\r\n",
            "Ansonic",
            finalBitCount,
            data,
            btn,
            CNT_TO_DIP(cnt));
    Serial.println("Generated code string:");
    Serial.println(buf);


    ScreenManager& screenMgr = ScreenManager::getInstance();
    lv_obj_t* textarea = screenMgr.getTextArea();
    lv_textarea_set_text(textarea, buf);

    return String(buf);
}

bool AnsonicProtocol::hasValidCode() const {
    return validCodeFound;
}


void AnsonicProtocol::transmit(uint32_t code, uint8_t bitCount) {
    // // Detach interrupts to prevent interference during transmission.
    // detachInterrupt(CC1101_CCGDO0A);
    // detachInterrupt(CC1101_CCGDO2A);

    // Initialize raw transmission.
   


    // Allocate the GPIO configuration structure on the heap.
    gpio_config_t* io_conf = new gpio_config_t;
    io_conf->pin_bit_mask = (1ULL << GPIO_NUM_17);
    io_conf->mode         = GPIO_MODE_OUTPUT;
    gpio_config(io_conf);
    delete io_conf;  // Free the configuration structure

    // Set up the transmit pin (using CC1101_CCGDO0A) as output.
    pinMode(CC1101_CCGDO0A, OUTPUT);
    gpio_set_level(CC1101_CCGDO0A, HIGH);

    // Calculate the number of pulses:
    // Header (1) + Start Bit (1) + (2 pulses per data bit) + Termination (1)
    size_t sampleCount = (2 * bitCount) + 3;
    // Allocate the transmit samples buffer on the heap.
    uint32_t* samplesToSend = new uint32_t[sampleCount];
    size_t index = 0;

    // Header: LOW for te_short * 35 microseconds.
    samplesToSend[index++] = te_short * 35;
    // Start bit: HIGH for te_short microseconds.
    samplesToSend[index++] = te_short;

    // Data bits (MSB first).
    for (int i = bitCount - 1; i >= 0; i--) {
        bool bit = (code >> i) & 1;
        if (bit) {
            // Bit '1': LOW for te_short, then HIGH for te_long.
            samplesToSend[index++] = te_short;
            samplesToSend[index++] = te_long;
        } else {
            // Bit '0': LOW for te_long, then HIGH for te_short.
            samplesToSend[index++] = te_long;
            samplesToSend[index++] = te_short;
        }
    }

    // Termination: LOW for te_short * 4 microseconds.
    samplesToSend[index++] = te_short * 4;

    // Transmit the buffered pulses.
    // The initial signal level is LOW.
    bool levelFlag = false;  // false -> LOW, true -> HIGH
    for (size_t i = 0; i < sampleCount; i++) {
        gpio_set_level(CC1101_CCGDO0A, levelFlag);
        levelFlag = !levelFlag;  // Toggle the signal level for next pulse.
        delayMicroseconds(samplesToSend[i]);
    }

    Serial.println("AnsonicProtocol: Transmission complete.");

    // Free the allocated transmission buffer.
    delete[] samplesToSend;
}

