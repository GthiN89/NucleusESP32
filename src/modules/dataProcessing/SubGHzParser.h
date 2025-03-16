#ifndef SUBGHZ_PARSER_H
#define SUBGHZ_PARSER_H

#include <Arduino.h>
#include <vector>
#include <cstdint>
#include <SPI.h>
#include <array>
#include <map>
#include "modules/RF/CC1101.h"
#include "GUI/events.h"
#include "modules/ETC/SDcard.h"
#include "SD.h"


using Frequency = uint32_t;
using RawDataElement = int16_t;
using CustomPresetElement = uint8_t;

extern int codesSend;


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

class SubGHzParser {
public:
    SubGHzParser() = default;

    SubGHzData parseContent(const char* filename);
    

    void sendRawData(const std::vector<RawDataElement>& rawData);
    
    void setRegisters();

    ELECHOUSE_CC1101 ELECCC1101;
    SDcard& SD_SUB = SDcard::getInstance();  
    
    
private:

    SubGHzData data;
    
    std::vector<RawDataElement> parseRawData(const String& line);
    
    std::vector<CustomPresetElement> parseCustomPresetData(const String& line);
    

    bool processHeader(File32* file);
    

    void processRawDataBlocks(File32* file);
};

#endif // SUBGHZ_PARSER_H
