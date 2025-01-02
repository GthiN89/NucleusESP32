#include "SubGHzParser.h"
#include <SDfat.h>
#include <SPI.h>
#include <map>
#include <string>
#include "modules/RF/CC1101.h"
#include "GUI/events.h"
#include "modules/ETC/SDcard.h"

SubGHzParser::SubGHzParser() {}
CC1101_CLASS CC1101;
EVENTS events1;
SubGHzData data;
ELECHOUSE_CC1101 ELECCC1101;
SDcard& SD_SUB = SDcard::getInstance();  

int codesSend = 0;

SubGHzData SubGHzParser::parseContent() {

    String line;
    int index = 0;
    
    while (index < file_content.length()) {
        int endOfLine = file_content.indexOf('\n', index);
        if (endOfLine == -1) endOfLine = file_content.length();
        line = file_content.substring(index, endOfLine);
        line.trim();
        
        if (line.startsWith("Frequency:")) {
            data.frequency = static_cast<Frequency>(line.substring(10).toInt());
            SD_SUB.tempFreq = data.frequency / 1000000.0f;
        } else if (line.startsWith("Preset:")) {
            data.preset = line.substring(7);
            C1101preset =  convert_str_to_enum(data.preset.c_str());
            SubGHzParser::setRegisters();
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
    String text = "Transmitting\n Codes send: " + String(codesSend);
    lv_label_set_text(label_sub, text.c_str());
    return result;
}



  void SubGHzParser::sendRawData(const std::vector<RawDataElement>& rawData) {
    if(stopTransmit) {
        return;
    }

    int tempSampleCount = rawData.size();
    if (tempSampleCount % 2 == 0) {
    } else {
    tempSampleCount++;
    }
   // Serial.print(disconnectSD());
    int samplesClean[tempSampleCount];
    for (int i = 0; i < tempSampleCount; i++) {
    samplesClean[i] = 1;
    }
        int s = 0;
        if(rawData[s] < 0){
            samplesClean[s] = 100;
            tempSampleCount++;
            s++;
        }

        while(s < tempSampleCount) {       
        if (rawData[s]>0)
        {            
            samplesClean[s] = rawData[s];
        } else {            
            if(rawData[s] * -1 > 0) {            
            samplesClean[s] = rawData[s] * -1;
            }
        }
        s++;
    }
    
    

    for (int i = 0; i < tempSampleCount; i++) {        
        Serial.print(String(samplesClean[i]).c_str());
            Serial.print(", ");
        }
    codesSend++;
    Serial.print(SD_SUB.tempFreq);
    CC1101.setFrequency(SD_SUB.tempFreq);
    CC1101.setCC1101Preset(C1101preset);
    CC1101.loadPreset();
    Serial.println(presetToString(C1101preset));   

   
    CC1101.sendSamples(samplesClean, tempSampleCount);

    C1101CurrentState = STATE_IDLE;
    runningModule = MODULE_NONE;

  }

void SubGHzParser::setRegisters(){
    uint8_t addr;
    uint8_t value;
    std::vector<uint8_t> regs; 
    size_t index = 0;

    if (data.preset == "FuriHalSubGhzPresetCustom") {
        regs = data.custom_preset_data;  
    } else {
        const uint8_t* array = presetTobyteArray(convert_str_to_enum(data.preset.c_str()));
    size_t presetArrayLength = 44;
    regs.assign(array, array + presetArrayLength);
    }

    while (index < regs.size()) {
        addr = regs[index++];
        value = regs[index++];
        ELECCC1101.SpiWriteReg(addr, value);

        if (addr == 0x00 && value == 0x00) {
            break;
        }
    }

    if (index + 8 <= regs.size()) {
        std::array<uint8_t, 8> paValue;
        std::copy(regs.begin() + index, regs.begin() + index + paValue.size(), paValue.begin());
        ELECCC1101.SpiWriteBurstReg(CC1101_PATABLE, paValue.data(), paValue.size());
    }
 
    }

     




std::vector<CustomPresetElement> SubGHzParser::parseCustomPresetData(const String& line) {
    std::vector<uint8_t> result;
    int start = 0;

    while (start < line.length()) {
        int end = line.indexOf(' ', start);
        if (end == -1) end = line.length();

        String hex_value = line.substring(start, end);
        
        result.push_back(static_cast<uint8_t>(strtol(hex_value.c_str(), NULL, 16)));
        
        start = end + 1;
    }

    return result; // returns as a vector of uint8_t, a byte array
}



bool SubGHzParser::loadFile(const char* filename) {
  Serial.print("Read Flipper File");
  Serial.print(filename);

    File32* file = SD_SUB.createOrOpenFile(filename, O_RDONLY);
    if (!file)
    {
        Serial.println("Failed to open file: " + String(filename));
        return false;
    }

    String line;
    bool parsingRawData = false;
    std::vector<RawDataElement> raw_data_sequence;

  //  Read the file line-by-line
    while (file->available()) {
        line = file->readStringUntil('\n');
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
            Serial.print(line);
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

    file->close();
    return true;
}



void SubGHzParser::clearData() {
    data = SubGHzData();  // Reset data to a new instance (clears all fields)
    SD_SUB.tempFreq = 0;
    SD_SUB.tempSampleCount = 0;
    C1101CurrentState = STATE_IDLE;
    runningModule = MODULE_NONE;
    codesSend = 0;
    SD_SUB.FlipperFileFlag = false;
}