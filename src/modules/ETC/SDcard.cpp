#include "SDcard.h"
#include <SdFat.h>
#include <map>
#include <string>
#include "sdios.h"


#define SPI_DRIVER_SELECT 2
#define SD_FAT_TYPE 1

// SoftSPI configuration
SoftSpiDriver<SDCARD_MISO_PIN, SDCARD_MOSI_PIN, SDCARD_SCK_PIN> softSpi;
#define SD_CONFIG SdSpiConfig(SDCARD_CS_PIN, DEDICATED_SPI, SPI_FULL_SPEED, &softSpi)

SdFat32 SD;  

//bool FlipperFileFlag;
//float tempFreq;
int tempSample[MAX_LENGHT_RAW_ARRAY];
//int tempSampleCount;
String line;
char buf[MAX_LENGHT_RAW_ARRAY];
char presetValue[MAX_LENGHT_RAW_ARRAY];

// Preset mapping
const std::map<CC1101_PRESET, std::string> presetMapping = {
    {AM270, "FuriHalSubGhzPresetOok270Async"},
    {AM650, "FuriHalSubGhzPresetOok650Async"},
    {FM238, "FuriHalSubGhzPreset2FSKDev238Async"},
    {FM476, "FuriHalSubGhzPreset2FSKDev476Async"},
    {CUSTOM, "FuriHalSubGhzPresetCustom"}
};

SDcard& SDcard::getInstance() {
    static SDcard instance;
    return instance;
}

bool SDcard::initializeSD() {
    if (!SD.begin(SD_CONFIG)) {
        //Serial.println(F("SD Card MOUNT FAIL"));
        return false;
    }
    //Serial.println(F("SD Card MOUNT SUCCESS"));
    uint64_t cardSize = (uint64_t)SD.card()->sectorCount() * 512 / (1024 * 1024);
    //Serial.print(F("Reported SD Card Size: "));
    //Serial.print(cardSize);
    //Serial.println(F(" MB"));
    return true;
}

bool SDcard::directoryExists(const char* dirPath) {
    if (SD.exists(dirPath)) {
        //Serial.print(F("Directory exists: "));
        //Serial.println(dirPath);
        return true;
    } else {
        //Serial.print(F("Directory does not exist: "));
        //Serial.println(dirPath);
        return false;
    }
}

bool SDcard::createDirectory(const char* dirPath) {
    if (SD.mkdir(dirPath)) {
        //Serial.print(F("Directory created successfully: "));
        //Serial.println(dirPath);
        return true;
    } else {
        //Serial.print(F("Failed to create directory: "));
        //Serial.println(dirPath);
        return false;
    }
}

File32* SDcard::getByPath(const char * path) {
    File32* file = new File32(SD.open(path));  
    if (!file->isOpen()) {  
        delete file;  
        return nullptr;
    }
    return file;
}

File32* SDcard::createOrOpenFile(const char* filePath, oflag_t mode) {
    File32* file = new File32();
    *file = SD.open(filePath, mode);
    if (!*file) {
        //Serial.print(F("Failed to open/create file: "));
        //Serial.println(filePath);
        delete file;
        return nullptr;
    } 
    //Serial.print(F("File opened/created successfully: "));
    //Serial.println(filePath);
    return file;
}

bool SDcard::closeFile(File32* file) {
    if (file) {
        file->close();
        delete file;
        return true;
    }
    //Serial.println(F("Attempted to close a null file handle."));
    return false;
}

bool SDcard::deleteFile(const char* filePath) {
    if (SD.exists(filePath)) {
        if (SD.remove(filePath)) {
            //Serial.println(F("File deleted successfully."));
            return true;
        } else {
            //Serial.println(F("File deletion failed."));
            return false;
        }
    }
    //Serial.println(F("File doesn't exist."));
    return false;
}

bool SDcard::fileExists(const char* filePath) {
    if (SD.exists(filePath)) {
        //Serial.println(F("File exists."));
        return true;
    }
    //Serial.println(F("File does not exist."));
    return false;
}

// void SDcard::writeFile(const char* filePath, const char* data, bool append) {
//     oflag_t mode = append ? (O_RDWR | O_APPEND) : (O_RDWR | O_CREAT | O_TRUNC);
//     File32* file = createOrOpenFile(filePath, mode);
//     if (!file) {
//         //Serial.println(F("Failed to open file for writing."));
//         return;
//     }
//     file->print(data);
//     //Serial.println(F("Data written successfully."));
//     closeFile(file);
// }

size_t SDcard::readFile(File32* file, void* buf, size_t bytesToRead) {
    if (!file || !file->isOpen()) { 
        //Serial.println(F("File is not open."));
        return 0;
    }

    return file->read(static_cast<uint8_t*>(buf), bytesToRead);  
}

bool SDcard::read_sd_card_flipper_file(String filename) {
    if (!initializeSD()) {
        //Serial.println(F("SD mounting failed!"));
        return false;
    }
    //Serial.print(F("sd/Read Flipper File: "));
    //Serial.println(filename);

    File32* file = createOrOpenFile(filename.c_str(), FILE_READ);
    if (!file) {
        return false;
    }

    memset(tempSample, 0, sizeof(tempSample));
    tempSampleCount = 0;
    char *buf = (char *)malloc(MAX_LENGHT_RAW_ARRAY);
    String line = "";

    while (file->available()) {
        line = file->readStringUntil('\n');
        line.toCharArray(buf, MAX_LENGHT_RAW_ARRAY);
        const char sep[2] = ":";
        const char values_sep[2] = " ";

        char *key = strtok(buf, sep);
        char *value;

        if (key != NULL) {
            value = strtok(NULL, sep);
            if (!strcmp(key, "Frequency")) {
                tempFreq = atoi(value) / 1000000.0f;
            }
            if (!strcmp(key, "Preset")) {
                strncpy(presetValue, value, MAX_LENGHT_RAW_ARRAY - 1);
                presetValue[MAX_LENGHT_RAW_ARRAY - 1] = '\0';

                for (const auto& pair : presetMapping) {
                    if (strcmp(pair.second.c_str(), presetValue) == 0) {
                        C1101preset = pair.first;
                        break;
                    }
                }
            }
            if (!strcmp(key, "RAW_Data")) {
                FlipperFileFlag = true;
                char *pulse = strtok(value, values_sep);
                while (pulse != NULL && tempSampleCount < MAX_LENGHT_RAW_ARRAY) {
                    tempSample[tempSampleCount++] = atoi(pulse);
                    pulse = strtok(NULL, values_sep);
                }
            }
        }
    }

    closeFile(file);
    free(buf);
    return true;
}

lv_fs_res_t SDcard::readNextFileInDir(File32* dir, char* fn, size_t fn_len) {
    if (!dir) {
        return LV_FS_RES_FS_ERR;  
    }
   
    File32 entry = dir->openNextFile();
      do {
        entry = dir->openNextFile(); 
        if (!entry) {
            fn[0] = '\0';  
            return LV_FS_RES_OK;
        }

        char name[13];  
        

        entry.getName(name, sizeof(name)); 

        if (entry.isDirectory()) {

            if (fn_len > 1 && fn[0] != '/') {
                snprintf(fn, fn_len, "/%s", name); 
            } else {
                snprintf(fn, fn_len, "%s", name);
            }
        } else {
            snprintf(fn, fn_len, "%s", name);  
        }

        entry.close();  
    } while (strcmp(fn, "/.") == 0 || strcmp(fn, "/..") == 0); 
    return LV_FS_RES_OK;
}


bool SDcard::writeFile(File32* file, const std::vector<uint8_t>& data, unsigned long writeDelay) {
    if (!file || !file->isOpen()) {
        return false;
    }

    // Write the data in chunks if necessary, with optional delay for pacing
    for (size_t i = 0; i < data.size(); i += 512) {  // 512 bytes is typical SD block size
        size_t chunkSize = std::min(data.size() - i, static_cast<size_t>(512));
        file->write(&data[i], chunkSize);

        if (writeDelay > 0) {
            delay(writeDelay);  // Delay between chunks, if specified
        }
    }
    
    file->flush();  // Ensure data is written to SD card
    return true;
}

bool SDcard::restartSD() {
    // Unmount SD card
    SD.end();
    softSpi.end();
    delay(20);
    softSpi.begin();
    delay(20);  // Small delay to allow proper unmounting

    // Attempt to remount the SD card
    if (!initializeSD()) {
        //Serial.println(F("SD Card reinitialization failed."));
        return false;
    }
    //Serial.println(F("SD Card reinitialized successfully."));
    return true;
}