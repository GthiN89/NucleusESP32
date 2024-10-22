#include "SubGHzParser.h"
#include "modules/ETC/SDcard.h"
#include "globals.h"
#include <SD.h>
#include <SPI.h>
#include <map>
#include <string>

SubGHzParser::SubGHzParser() {}

bool SubGHzParser::loadFile(const char* filename) {
     SDInit();
    if (!SD.begin(true)) {
        Serial.println("An error has occurred while mounting SPIFFS");
        return false;
    }
    
    File file = SD.open(filename, "r");
    if (!file) {
        Serial.println("Failed to open file for reading");
        return false;
    }
    
    file_content = file.readString();
    file.close();
    return true;
}

SubGHzData SubGHzParser::parseContent() {
    SubGHzData data;
    String line;
    int index = 0;
    
    while (index < file_content.length()) {
        int endOfLine = file_content.indexOf('\n', index);
        if (endOfLine == -1) endOfLine = file_content.length();
        line = file_content.substring(index, endOfLine);
        line.trim();
        
        if (line.startsWith("Frequency:")) {
            data.frequency = static_cast<Frequency>(line.substring(10).toInt());
        } else if (line.startsWith("Preset:")) {
            data.preset = line.substring(7);
        } else if (line.startsWith("Custom_preset_data:")) {
            data.custom_preset_data = parseCustomPresetData(line.substring(19));
        } else if (line.startsWith("Protocol:")) {
            data.protocol = line.substring(9);
        } else if (line.startsWith("RAW_Data:") || line[0] == '-' || isdigit(line[0])) {
            auto parsed_raw_data = parseRawData(line.startsWith("RAW_Data:") ? line.substring(9) : line);
            data.raw_data.insert(data.raw_data.end(), parsed_raw_data.begin(), parsed_raw_data.end());
        } else if (line.startsWith("Bit:") ) {
            data.bit = line.substring(5);
        } else if (line.startsWith("Key:") ) {
             auto parsed_key = parseRawData(line.startsWith("Key:") ? line.substring(5) : line);
            data.key_data.insert(data.key_data.end(), parsed_key.begin(), parsed_key.end());
        } else if (line.startsWith("TE:")) {
            data.te = line.substring(4);
        }  else if (line.startsWith("Bit_RAW:")) {
            data.bit_raw = line.substring(9);
        } else if (line.startsWith("Data_RAW:")) {
            auto parsed_raw_data = parseRawData(line.startsWith("Data_RAW:") ? line.substring(9) : line);
            data.raw_data.insert(data.raw_data.end(), parsed_raw_data.begin(), parsed_raw_data.end());
        }

        index = endOfLine + 1;
    }
    
    return data;
}

std::vector<CustomPresetElement> SubGHzParser::parseCustomPresetData(const String& line) {
    std::vector<CustomPresetElement> result;
    int start = 0;
    while (start < line.length()) {
        int end = line.indexOf(' ', start);
        if (end == -1) end = line.length();
        String hex_value = line.substring(start, end);
        result.push_back(static_cast<CustomPresetElement>(strtol(hex_value.c_str(), NULL, 16)));
        start = end + 1;
    }
    return result;
}

std::vector<RawDataElement> SubGHzParser::parseRawData(const String& line) {
    std::vector<RawDataElement> result;
    int start = 0;
    while (start < line.length()) {
        int end = line.indexOf(' ', start);
        if (end == -1) end = line.length();
        String value = line.substring(start, end);
        result.push_back(static_cast<RawDataElement>(value.toInt()));
        start = end + 1;
    }
    return result;
}

void SubGHzParser::printParsedData(const SubGHzData& data) {
    Serial.println("Parsed SubGHz data:");
    Serial.print("Frequency: ");
    Serial.println(data.frequency);
    Serial.print("Preset: ");
    Serial.println(data.preset);
    
    Serial.print("Custom preset data: ");
    for (const auto& value : data.custom_preset_data) {
        Serial.print(value, HEX);
        Serial.print(" ");
    }
    Serial.println();
    
    Serial.print("Protocol: ");
    Serial.println(data.protocol);

    Serial.print("bit: ");
    Serial.println(data.bit);

    Serial.print("bit raw: ");
    Serial.println(data.bit_raw);

    Serial.print("te: ");
    Serial.println(data.te);

    Serial.print("key: ");
    Serial.println(data.te);

    Serial.print("RAW Data (first 10 elements): ");
    for (size_t i = 0; i < std::min(size_t(10), data.raw_data.size()); i++) {
        Serial.print(data.raw_data[i]);
        Serial.print(" ");
    }
    Serial.println("...");
}