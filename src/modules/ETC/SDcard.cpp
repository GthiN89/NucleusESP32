#include "SDcard.h"
#include "globals.h"
#include <SD.h>
#include <SPI.h>
#include <map>
#include <string>

SPIClass sd(SDCARD_SPI_HOST);

bool SDInit() {
    // Begin SPI with specific pins
        // Ensure CC1101 is disabled
    
  
    SPI.begin(SDCARD_SCK, SDCARD_MISO, SDCARD_MOSI, SDCARD_CS);

    
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
    return "SD card disconnected.\n";
}

#define MAX_LENGHT_RAW_ARRAY 4096

float tempFreq;
int tempSample[MAX_LENGHT_RAW_ARRAY];
int tempSampleCount;
const std::map<CC1101_PRESET, std::string> presetMapping = {
    {AM270, "FuriHalSubGhzPresetOok270Async"},
    {AM650, "FuriHalSubGhzPresetOok650Async"},
    {FM238, "FuriHalSubGhzPreset2FSKDev238Async"},
    {FM476, "FuriHalSubGhzPreset2FSKDev476Async"},
    {CUSTOM, "FuriHalSubGhzPresetCustom"}
};

String line;
char buf[MAX_LENGHT_RAW_ARRAY];
char presetValue[MAX_LENGHT_RAW_ARRAY]; // To store the preset value



bool read_sd_card_flipper_file(String filename)
{
        if (!SDInit()) {
        Serial.println("SD mounting failed!");
        return false;
    } else {
        Serial.println("SD mounted");
    }
  Serial.print("Read Flipper File");

    File file = SD.open(filename, FILE_READ);
    if (!file)
    {
        Serial.println("Failed to open file: " + String(filename));
        return false;
    }

    // Reset Current
    memset(tempSample, 0, sizeof(MAX_LENGHT_RAW_ARRAY));       
    tempSampleCount = 0;

    char *buf = (char *) malloc(MAX_LENGHT_RAW_ARRAY);
    String line = "";

    while (file.available())
    {
        line = file.readStringUntil('\n');
        line.toCharArray(buf, MAX_LENGHT_RAW_ARRAY);
        const char sep[2] = ":";
        const char values_sep[2] = " ";

        char *key = strtok(buf, sep);
        char *value;

        if (key != NULL)
        {
            value = strtok(NULL, sep);

            if (!strcmp(key, "Frequency"))
            {
                tempFreq = atoi(value) / 1000000.0f;
            }

        if (!strcmp(key, "Preset")) {
            strncpy(presetValue, value, MAX_LENGHT_RAW_ARRAY - 1); // Store the preset value
            presetValue[MAX_LENGHT_RAW_ARRAY - 1] = '\0'; // Ensure null termination
            
            // Find matching preset in the map
            for (const auto& pair : presetMapping) {
                if (strcmp(pair.second.c_str(), presetValue) == 0) {
                    C1101preset = pair.first;
                    break;
                }
            }
        }

            if (!strcmp(key, "RAW_Data"))
            {
                char *pulse = strtok(value, values_sep);

                while (pulse != NULL && tempSampleCount < MAX_LENGHT_RAW_ARRAY)
                {
                    tempSample[tempSampleCount] = atoi(pulse);
                    pulse = strtok(NULL, values_sep);
                    tempSampleCount++;
                }
            }
        }
    }

    file.close();

    free(buf);

  return true;
}

bool  deleteFile(const char *path) {
    if (SD.exists(path)) {
        if (SD.remove(path)) {
            Serial.println("File deleted successfully");
            return true;
        } else {
            Serial.println("File deletion failed");
            return false;
        }
    } else {
        Serial.println("File doesn't exist");
        return false;
    }
}
