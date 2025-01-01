#ifndef IR_CONTROL_H
#define IR_CONTROL_H

#include "WORLD_IR_CODES.h"
#include "ir.h"
#include "TV-B-Gone.h"

#define NUMPIXELS 1 // Number of NeoPixels attached to the Arduino

// Constants for serial output formatting
#define putstring_nl(s) Serial.println(s)
#define putstring(s) Serial.print(s)
#define putnum_ud(n) Serial.print(n, DEC)
#define putnum_uh(n) Serial.print(n, HEX)

#define MAX_WAIT_TIME 65535 // Maximum wait time in microseconds

extern uint8_t num_NAcodes;
extern uint8_t num_EUcodes;

extern uint8_t bitsleft_r;
extern uint8_t bits_r;
extern uint8_t code_ptr;
extern volatile const IrCode * powerCode;

// Function to read bits from the IR code
uint8_t read_bits(uint8_t count);

// IR signal sending functions
void sendRawSignal(const uint16_t* times, uint16_t length, uint16_t frequency);
void sendAllCodes(const IrCode* const* codesArray, uint8_t numCodes);

// Delay function for microseconds
void delay_ten_us(uint16_t us);

#endif // IR_CONTROL_H
