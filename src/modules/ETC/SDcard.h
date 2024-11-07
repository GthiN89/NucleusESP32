#ifndef SDCARD_H
#define SDCARD_H

#include <FS.h>
#include <SDfat.h>
#include "SPI.h"
#include "driver/sdspi_host.h"   // Include for SD card SPI host definitions
#include "driver/spi_common.h"   // Include for common SPI host definitions

// SPI Host configuration
#define SDCARD_SPI_HOST VSPI  // Using HSPI for SD card communication

// Define SD card pins
#define SDCARD_CS 5    // Chip Select pin for the SD card
#define SDCARD_MISO 19 // MISO pin for SD card
#define SDCARD_MOSI 23 // MOSI pin for SD card
#define SDCARD_SCK 18  // SCK pin for SD card

// Define CS pin for SPI transaction handling
#define MICRO_SD_IO SDCARD_CS // Use SDCARD_CS for SD card control

// Declare the SPIClass object for SD card
extern SPIClass sd; 

extern float tempFreq;
extern int tempSample[];
extern int tempSampleCount;
extern bool FlipperFileFlag;

// Function prototypes
bool SDInit();        // Function to initialize the SD card
String disconnectSD();// Function to disconnect the SD card
bool read_sd_card_flipper_file(String filename);
bool  deleteFile(const char *path);

#endif // SDCARD_H
