#include "SDcard.h"
#include "globals.h"
#include <SD.h>
#include <SPI.h>

SPIClass sd(SDCARD_SPI_HOST);

bool SDInit() {
    // Begin SPI with specific pins
    SPI.begin(SDCARD_SCK, SDCARD_MISO, SDCARD_MOSI, SDCARD_CS);
    
    // Ensure CC1101 is disabled
    digitalWrite(CC1101_CS, HIGH);  
    
    // Begin SD card initialization
    if (!SD.begin(SDCARD_CS)) {
        Serial.println("SD Card MOUNT FAIL");
        return false;
    } else {
        Serial.println("SD Card MOUNT SUCCESS");
        uint32_t cardSize = SD.cardSize() / (1024 * 1024);
        Serial.println("SDCard Size: " + String(cardSize) + "MB");
        return true;
    }
}

String disconnectSD() {
    SPI.endTransaction();
    digitalWrite(SDCARD_CS, HIGH);  
    return "SD card disconnected.\n";
}
