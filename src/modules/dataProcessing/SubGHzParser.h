#ifndef SUBGHZ_PARSER_H
#define SUBGHZ_PARSER_H

#include <Arduino.h>
#include <vector>
#include <string>
#include <cstdint>
#include <SPI.h>
#include <map>
#include <string>
#include "modules/RF/CC1101.h"
#include "GUI/events.h"
#include "modules/ETC/SDcard.h"
#include "SD.h"

using Frequency = uint32_t;  
using PresetString = String; 
using ProtocolString = String; 
using RawDataElement = int16_t; 
extern int codesSend;


using CustomPresetElement = uint8_t;

struct SubGHzData {
    Frequency frequency;
    String preset;
    std::vector<CustomPresetElement> custom_preset_data;
    String protocol;
    String bit;
    String bit_raw;
    String te;
    std::vector<std::vector<RawDataElement>> raw_data_list;
    std::vector<RawDataElement> raw_data;
    std::vector<RawDataElement> key_data;
};


struct RFDetect {
    uint32_t frequencyIN;
    uint32_t PAIn;
    uint32_t frequencyOUT;
    uint32_t PAOut;
    uint32_t Preset;
};

class SubGHzParser {
public:
  //  SubGHzParser();
    bool loadFile(const char* filename); //spfs
    SubGHzData parseContent();
    const String& getFileContent() const { return file_content; }
    void sendRawData(const std::vector<RawDataElement>& rawData);
    void setRegisters();
CC1101_CLASS CC1101;
EVENTS events1;
SubGHzData data;
ELECHOUSE_CC1101 ELECCC1101;
SDcard& SD_SUB = SDcard::getInstance();  

private:
    String file_content;
    std::vector<CustomPresetElement> parseCustomPresetData(const String& line);
    // Helper method to parse a line of raw data
    std::vector<RawDataElement> parseRawData(const String& line);
    void clearData();
};
#endif // SUBGHZ_PARSER_H