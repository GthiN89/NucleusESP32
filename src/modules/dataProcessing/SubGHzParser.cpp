#include "SubGHzParser.h"
#include "modules/ETC/SDcard.h"
#include "globals.h"
#include <SD.h>
#include <SPI.h>
#include <map>
#include <string>
#include "modules/RF/CC1101.h"
#include "GUI/events.h"

SubGHzParser::SubGHzParser() {}
CC1101_CLASS CC1101;
EVENTS events1;

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
            tempFreq = data.frequency / 1000000.0f;
        } else if (line.startsWith("Preset:")) {
            data.preset = line.substring(7);
            C1101preset =  events1.stringToCC1101Preset(data.preset);
        } else if (line.startsWith("Custom_preset_data:")) {
            data.custom_preset_data = parseCustomPresetData(line.substring(19));
        } else if (line.startsWith("Protocol:")) {
            data.protocol = line.substring(9);
        } else if (line.startsWith("RAW_Data:")) {
            std::vector<RawDataElement> raw_data_sequence = parseRawData(line.substring(9));
            index = endOfLine + 1;
            while (index < file_content.length()) {
                endOfLine = file_content.indexOf('\n', index);
                if (endOfLine == -1) endOfLine = file_content.length();
                String nextLine = file_content.substring(index, endOfLine);
                nextLine.trim();
                if (nextLine.length() > 0 && (nextLine[0] == '-' || isDigit(nextLine[0]))) {
                    auto parsed_line = parseRawData(nextLine);
                    raw_data_sequence.insert(raw_data_sequence.end(), parsed_line.begin(), parsed_line.end());
                    index = endOfLine + 1;
                } else {
                    break;
                }
            }
            data.raw_data_list.push_back(raw_data_sequence);
            continue; 
        } else if (line.startsWith("Bit:")) {
            data.bit = line.substring(4);
        } else if (line.startsWith("Key:")) {
            auto parsed_key = parseRawData(line.substring(4));
            data.key_data.insert(data.key_data.end(), parsed_key.begin(), parsed_key.end());
        } else if (line.startsWith("TE:")) {
            data.te = line.substring(3);
        } else if (line.startsWith("Bit_RAW:")) {
            data.bit_raw = line.substring(8);
        } else if (line.startsWith("Data_RAW:")) {
            auto parsed_raw_data = parseRawData(line.startsWith("Data_RAW:") ? line.substring(9) : line);
            data.raw_data.insert(data.raw_data.end(), parsed_raw_data.begin(), parsed_raw_data.end());        }
        
        index = endOfLine + 1;
    }
    
    return data;
}

std::vector<RawDataElement> SubGHzParser::parseRawData(const String& line) {
    std::vector<RawDataElement> result;
    int start = 0;
    while (start < line.length()) {
        int end = line.indexOf(' ', start);
        if (end == -1) end = line.length();
        String value = line.substring(start, end);
        if (value.length() > 0) {
            result.push_back(static_cast<RawDataElement>(value.toInt()));
        }
        start = end + 1;
    }
    return result;
}



  void SubGHzParser::sendRawData(const std::vector<RawDataElement>& rawData) {

    int tempSampleCount = rawData.size();
    if (tempSampleCount % 2 == 0) {
    } else {
    tempSampleCount++;
    }
    Serial.print(disconnectSD());
    int samplesClean[tempSampleCount];
    for (int i = 0; i < tempSampleCount; i++) {
    samplesClean[i] = 1;
    }

        for (int i = 0; i < tempSampleCount; i++) {        
        if (rawData[i]>0)
        {
            Serial.print(String(rawData[i]).c_str());
            samplesClean[i] = rawData[i];
            Serial.print(", ");
        } else {            
            if(rawData[i] * -1 > 0) {
            Serial.print(String(tempSample[i] * -1).c_str());
            samplesClean[i] = rawData[i] * -1;
            }
            Serial.print(", ");
        }
    }

        for (int i = 0; i < tempSampleCount; i++) {        
        Serial.print(String(samplesClean[i]).c_str());
            Serial.print(", ");
        }
    CC1101.setFrequency(tempFreq);
    CC1101.loadPreset();
    CC1101.sendSamples(samplesClean, tempSampleCount);

    C1101CurrentState = STATE_IDLE;

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

    String line;
    bool parsingRawData = false;
    std::vector<RawDataElement> raw_data_sequence;

    // Read the file line-by-line
    while (file.available()) {
        line = file.readStringUntil('\n');
        line.trim();

        if (line.startsWith("RAW_Data:")) {
            if (parsingRawData) {
                sendRawData(raw_data_sequence);  
                raw_data_sequence.clear();       
            }
            // Start a new RAW_Data section
            raw_data_sequence = parseRawData(line.substring(9));
            parsingRawData = true;
        
        } else if (parsingRawData && (line[0] == '-' || isDigit(line[0]))) {
            auto parsed_line = parseRawData(line);
            raw_data_sequence.insert(raw_data_sequence.end(), parsed_line.begin(), parsed_line.end());

        } else {
            if (parsingRawData) {
                sendRawData(raw_data_sequence);
                raw_data_sequence.clear();
                parsingRawData = false;
            }
            if (line.startsWith("Frequency:")) {
                Serial.print("Frequency: ");
                Serial.println(line.substring(10));
            }
        }
    }
    if (parsingRawData) {
        sendRawData(raw_data_sequence);
    }

    file.close();
    return true;
}



