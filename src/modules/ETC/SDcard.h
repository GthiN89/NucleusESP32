#ifndef SDCARD_H
#define SDCARD_H

#include <SdFat.h>
#include <SPI.h>
#include <map>
#include <string>
#include "modules/RF/FlipperSubFile.h"

#define SD_FAT_TYPE 1
#define SPI_DRIVER_SELECT 2

const uint8_t SDCARD_CS_PIN = 5;
const uint8_t SDCARD_MISO_PIN = 19;
const uint8_t SDCARD_MOSI_PIN = 23;
const uint8_t SDCARD_SCK_PIN = 18;

#define MAX_LENGHT_RAW_ARRAY 4096


class SDcard {
public:
    static SDcard& getInstance();
    bool FlipperFileFlag;
    int tempSampleCount;
    float tempFreq;

    // Initialization and management
    bool initializeSD();
    bool directoryExists(const char* dirPath);
    bool createDirectory(const char* dirPath);

    // File operations
    File32* createOrOpenFile(const char* filePath, oflag_t mode);
    bool closeFile(File32* file);
    bool deleteFile(const char* filePath);
    bool fileExists(const char* filePath);
    void writeFile(const char* filePath, const char* data, bool append);
    size_t readFile(File32* file, void* buf, size_t bytesToRead);
    bool read_sd_card_flipper_file(String filename);

    // Directory operations
    lv_fs_res_t readNextFileInDir(File32* dir, char* fn, size_t fn_len);
    File32* getByPath(const char * path); 

    //vars

private:
    SDcard() = default;                                   // Private constructor for singleton
    SDcard(const SDcard&) = delete;                       // Delete copy constructor
    SDcard& operator=(const SDcard&) = delete;            // Delete copy assignment
};

#endif  // SDCARD_H
