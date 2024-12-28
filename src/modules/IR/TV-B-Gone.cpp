#include "WORLD_IR_CODES.h"
#include "TV-B-Gone.h"
#include "Arduino.h"
#include "ir.h"

// Define the number of codes for each region
// uint8_t num_NAcodes = NUM_ELEM(NApowerCodes);
// uint8_t num_EUcodes = NUM_ELEM(EUpowerCodes);

// IR code reading variables
uint8_t bitsleft_r = 0;
uint8_t bits_r = 0;
uint8_t code_ptr = 0;
volatile const IrCode* powerCode = nullptr;

// Function to read bits from IR code
uint8_t read_bits(uint8_t count) {
    uint8_t tmp = 0;

    for (uint8_t i = 0; i < count; i++) {
        if (bitsleft_r == 0) {
            bits_r = powerCode->codes[code_ptr++];
            bitsleft_r = 8;
        }
        bitsleft_r--;
        tmp |= (((bits_r >> bitsleft_r) & 1) << (count - 1 - i));
    }
    return tmp;
}

// Function to send raw IR signal
void sendRawSignal(const uint16_t* times, uint16_t length, uint16_t frequency) {
    uint32_t period = 1000000 / frequency; // Period in microseconds
    uint32_t pulseWidth = period / 2;      // 50% duty cycle

    for (uint16_t i = 0; i < length; i++) {
        if (i % 2 == 0) {
            uint32_t endTime = micros() + times[i];
            while (micros() < endTime) {
                digitalWrite(IR_TX, HIGH);
                delayMicroseconds(pulseWidth);
                digitalWrite(IR_TX, LOW);
                delayMicroseconds(pulseWidth);
            }
        } else {
            delayMicroseconds(times[i]);
        }
    }
}

// Function to send all IR codes
void sendAllCodes(const IrCode* const* codesArray, uint8_t numCodes) {
    for (uint8_t i = 0; i < numCodes; i++) {
        const IrCode* code = codesArray[i];
        const uint8_t numpairs = code->numpairs;
        const uint8_t bitcompression = code->bitcompression;
        const uint16_t* times = code->times;
        const uint16_t* codes = &code->codes[i]; 
        uint8_t bitsleft = 0;
        uint8_t bits = 0;

        uint16_t rawData[numpairs * 2];

        for (uint8_t k = 0; k < numpairs; k++) {
            uint16_t ti = 0;

            for (uint8_t b = 0; b < bitcompression; b++) {
                if (bitsleft == 0) {
                    bits = codes[code_ptr++];
                    bitsleft = 8;
                }
                bitsleft--;
                ti |= ((bits >> bitsleft) & 1) << (bitcompression - 1 - b);
            }
            ti *= 2;

            rawData[k * 2] = times[ti] * 10;       // OnTime
            rawData[k * 2 + 1] = times[ti + 1] * 10; // OffTime
        }

        sendRawSignal(rawData, numpairs * 2, 1000000 / code->timer_val);
        delay(50);  
    }
}

// Delay function for microseconds
void delay_ten_us(uint16_t us) {
    delayMicroseconds(us * 10);
}
