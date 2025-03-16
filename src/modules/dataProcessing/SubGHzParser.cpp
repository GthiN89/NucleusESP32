#include "SubGHzParser.h"

int codesSend = 0;

SubGHzData SubGHzParser::parseContent(const char* filename) {
    Serial.println(filename);
    
    // First pass: open file and process header.
    File32* file = SD_SUB.createOrOpenFile(filename, O_RDONLY);
    if (!file) {
        return data;
    }
    
    bool radioLoaded = processHeader(file);
    file->close();
    SD_SUB.endSD();
    SD_SUB.initializeSD();
    
    if (!radioLoaded) {
        return data;
    }
    
    
    file = SD_SUB.createOrOpenFile(filename, O_RDONLY);
    if (!file) {
        Serial.println("DEBUG: Failed to open file (second pass)");
        return data;
    }
    
    String line;
    while (file->available()) {
        line = file->readStringUntil('\n');
        line.trim();
        if (line.startsWith("Protocol:")) {
            break;
        }
    }

    processRawDataBlocks(file);
    file->close();
    return data;
}

bool SubGHzParser::processHeader(File32* file) {
    String line;
    bool headerComplete = false;
    
    while (file->available() && !headerComplete) {
        line = file->readStringUntil('\n');
        line.trim();
        if (line.length() == 0)
            continue;
        
        if (line.startsWith("Frequency:")) {
            data.frequency = static_cast<Frequency>(line.substring(10).toInt());
            SD_SUB.tempFreq = data.frequency / 1000000.0f;
            Serial.println(data.frequency);
        } else if (line.startsWith("Preset:")) {
            data.preset = line.substring(7);
            Serial.println(data.preset);
        } else if (line.startsWith("Custom_preset_data:")) {
            data.custom_preset_data = parseCustomPresetData(line.substring(19));
        } else if (line.startsWith("Protocol:")) {
            data.protocol = line.substring(9);
            Serial.println(data.protocol);
            headerComplete = true;
        } else {
            Serial.print("DEBUG: Ignoring header line: ");
            Serial.println(line);
        }
    }
    
    if (headerComplete) {
        if (CC1101.init()) {
            Serial.println("DEBUG: Radio initialized successfully.");
        } else {
            Serial.println("DEBUG: Radio initialization failed.");
            return false;
        }
        setRegisters();
        return true;
    }
    return false;
}

void SubGHzParser::processRawDataBlocks(File32* file) {
    String line;
    
    // For each line that starts with "RAW_Data:" we treat it as a separate block.
    while (file->available()) {
        updatetransmitLabel = true;
        line = file->readStringUntil('\n');
        line.trim();
        
        if (line.startsWith("RAW_Data:")) {
            std::vector<RawDataElement> rawDataBuffer = parseRawData(line.substring(9));
            Serial.println(rawDataBuffer.size());
            data.raw_data_list.push_back(rawDataBuffer);
            sendRawData(rawDataBuffer);
            codesSend++;
        } else if (line.length() > 0) {
            Serial.println(line);
        }
    }
    updatetransmitLabel = false;
}

std::vector<RawDataElement> SubGHzParser::parseRawData(const String& line) {
    std::vector<RawDataElement> result;
    int start = 0;
    while (start < line.length()) {
        int end = line.indexOf(' ', start);
        if (end == -1)
            end = line.length();
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
    if (tempSampleCount % 2 != 0) {
        tempSampleCount++; // Ensure even count.
    }
    int samplesClean[tempSampleCount];
    for (int i = 0; i < tempSampleCount; i++) {
        samplesClean[i] = 1;
    }
    
    int s = 0;
    if (rawData[s] < 0) {
        samplesClean[s] = 100;
        tempSampleCount++;
        s++;
    }
    while (s < tempSampleCount) {
        if (rawData[s] > 0) {
            samplesClean[s] = rawData[s];
        } else {
            samplesClean[s] = (rawData[s] * -1);
        }
        s++;
    }
    Serial.println();    
    codesSend++;
    CC1101.sendSamples(samplesClean, tempSampleCount, 0);    
    C1101CurrentState = STATE_IDLE;
}

void SubGHzParser::setRegisters() {    
    if (data.preset == "FuriHalSubGhzPresetCustom") {
        std::vector<uint8_t> regs = data.custom_preset_data;
        size_t index = 0;
        while (index < regs.size()) {
            uint8_t addr = regs[index++];
            uint8_t value = regs[index++];
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
    } else {
        CC1101_PRESET presetEnum = convert_str_to_enum(data.preset.c_str());
        Serial.println((int)presetEnum);
        CC1101.setFrequency(SD_SUB.tempFreq);
        CC1101.setCC1101Preset(presetEnum);
        CC1101.loadPreset();
        ELECHOUSE_cc1101.setPA(12);
        CC1101.initRaw();

    }
    
    ELECHOUSE_cc1101.SetTx();
    gpio_set_direction(CC1101_CCGDO0A, GPIO_MODE_OUTPUT);
    SPI.end(); 

}

std::vector<CustomPresetElement> SubGHzParser::parseCustomPresetData(const String& line) {
    std::vector<uint8_t> result;
    int start = 0;
    while (start < line.length()) {
        int end = line.indexOf(' ', start);
        if (end == -1)
            end = line.length();
        String hex_value = line.substring(start, end);
        result.push_back(static_cast<uint8_t>(strtol(hex_value.c_str(), NULL, 16)));
        start = end + 1;
    }
    return result;
}
