// #include "Radio.h"


// SPISettings SetSPI(2000000, MSBFIRST, SPI_MODE0);


// Module radioModule(CC1101_CS, CC1101_CCGDO0A, -1, CC1101_CCGDO2A, SPI, SetSPI);


// CC1101 radio433(&radioModule);

// void RadioReceiver::setup() {

//     SPI.begin(CC1101_SCLK, CC1101_MISO, CC1101_MOSI, CC1101_CS);


//     Serial.begin(115200);
//     Serial.print(F("CC1101 Initializing ... "));
//     int state = radio433.begin();
//     radio433.setPromiscuousMode(true);  // Set to promiscuous mode for listening to all packets

//     if (state == RADIOLIB_ERR_NONE) {
//         Serial.println(F("CC1101 initialized successfully!"));
//     } else {
//         Serial.print(F("CC1101 initialization failed with error code: "));
//         Serial.println(state);
//     }
// }

// void RadioReceiver::loop() {
//     if (radio433.available()) {
//         // Create a buffer to store the received data (e.g., 255 bytes)
//         uint8_t receivedBuffer[255];  


//         // Check if data was received successfully
//             // Assuming received data is available in the radio's internal buffer
//             // You would need to implement a way to access the data from the buffer
//             // or use the `receive()` method to capture the received bytes.

//             int len = radio433.readData(receivedBuffer, sizeof(receivedBuffer));

//             if (len > 0) {
//                 // Convert the data to a string
//                 String receivedData = "";
//                 for (int i = 0; i < len; i++) {
//                     receivedData += (char)receivedBuffer[i];
//                 }
//                 // Print the received data to Serial
//                 Serial.println(receivedData);
//             }
//         } else {
//             Serial.println("No valid data received.");
//         }
// }
