#ifndef FLIPPER_SUB_FILE_H
#define FLIPPER_SUB_FILE_H

#include "Arduino.h"
#include <SD.h>
#include <sstream>
#include <map>
#include <vector>
#include "globals.h"

class FlipperSubFile {
public:
    static void generateRaw(
        File& file,
        const CC1101_PRESET& presetName,
        const std::vector<byte>& customPresetData,
        std::stringstream& samples,
        float frequency
    );

private:
    static const std::map<CC1101_PRESET, std::string> presetMapping;

    static void writeHeader(File& file, float frequency);
    static void writePresetInfo(File& file, const CC1101_PRESET& presetName, const std::vector<byte>& customPresetData);
    static void writeRawProtocolData(File& file, std::stringstream& samples);
    static std::string getPresetName(const CC1101_PRESET& preset);
};

#endif // FLIPPER_SUB_FILE_H