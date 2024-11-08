#ifndef SUBGHZ_PARSER_H
#define SUBGHZ_PARSER_H

#include <Arduino.h>
#include <vector>
#include <string>
#include <cstdint>

// Define types for clarity and potential future flexibility
using Frequency = uint32_t;  // Typically in Hz, unlikely to exceed 4.29 GHz
using PresetString = String; // Arduino String for preset
using ProtocolString = String; // Arduino String for protocol
using RawDataElement = int16_t; // For raw data, allowing negative values
extern int codesSend;

// Custom preset data is typically small values, often representing register settings
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


class SubGHzParser {
public:
    SubGHzParser();
    bool loadFile(const char* filename); //spfs
    SubGHzData parseContent();
    const String& getFileContent() const { return file_content; }
    void sendRawData(const std::vector<RawDataElement>& rawData);
    void setRegisters();

private:
    String file_content;
    std::vector<CustomPresetElement> parseCustomPresetData(const String& line);
    // Helper method to parse a line of raw data
    std::vector<RawDataElement> parseRawData(const String& line);
};

#endif // SUBGHZ_PARSER_H