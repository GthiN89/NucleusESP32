#include "ChamberlainCodeProtocol.h"
#include <stdio.h>

ChamberlainCodeProtocol::ChamberlainCodeProtocol()
    : te_short(1000),
      te_long(3000),
      te_delta(200),
      min_count_bit(10)
      
{
    binaryValue.reset();
    samplesToSend.clear();
}

void ChamberlainCodeProtocol::reset() {
    encoderState = EncoderStepStart;
    samplesToSend.clear();
    // (Reset any additional decoder state if needed)
}

void ChamberlainCodeProtocol::toBits(uint32_t hexValue, uint8_t bitCount) {
    // Clear and fill the 9-bit bitset using only lower 'bitCount' bits.
    binaryValue.reset();
    for (uint8_t i = 0; i < bitCount; i++) {
        bool bit = ((hexValue >> i) & 1) != 0;
        binaryValue.set(i, bit);
    }
}

void ChamberlainCodeProtocol::yield(uint32_t hexValue, uint8_t bitCount) {
    // Mimic the Came protocol: yield() is called repeatedly until encoderState becomes EncoderStepReady.
    // We use a simple two‑state approach here.
    if (encoderState == EncoderStepStart) {
        samplesToSend.clear();
        toBits(hexValue, bitCount);
        // Debug output:
        //Serial.print("Chamberlain yield: code 0x");
        //Serial.print(hexValue, HEX);
        //Serial.print(" as bits: ");
        //Serial.println(binaryValue.to_string().c_str());
        encoderState = EncoderStepDurations;
    }
    if (encoderState == EncoderStepDurations) {
        // Build the pulse sequence:
        // 1. Start with a HIGH pulse (te_short)
        samplesToSend.push_back(te_short);
        // 2. For each bit, from MSB to LSB:
        for (int i = bitCount - 1; i >= 0; i--) {
            bool bit = binaryValue.test(i);
            if (bit) {
                // For bit '1': HIGH pulse = te_short * 3, then LOW pulse = te_short
                samplesToSend.push_back(te_short * 3);
                samplesToSend.push_back(te_short);
            } else {
                // For bit '0': HIGH pulse = te_short, then LOW pulse = te_short * 3
                samplesToSend.push_back(te_short);
                samplesToSend.push_back(te_short * 3);
            }
        }
        // 3. Append an end (guard) pulse.
        // Use the LSB of the code (binaryValue[0]) to decide which end pulse to use.
        if (binaryValue.test(0)) {
            samplesToSend.push_back(END_HIGH_FOR_1);
            samplesToSend.push_back(END_LOW_FOR_1);
        } else {
            samplesToSend.push_back(END_HIGH_FOR_0);
            samplesToSend.push_back(END_LOW_FOR_0);
        }
        delay(5);
        encoderState = EncoderStepReady;
        //Serial.println("Chamberlain yield: EncoderStepReady");
    }
}

String ChamberlainCodeProtocol::getCodeString(uint32_t hexValue, uint8_t bitCount) const {
    char buf[64];
    snprintf(buf, sizeof(buf), "Chamberlain %d-bit code: 0x%X", bitCount, hexValue);
    return String(buf);
}
