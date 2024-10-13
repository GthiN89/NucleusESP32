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
    void generateRaw(
        File& file,                               // File reference for output
        const CC1101_PRESET& presetName,          // The preset used (e.g., CUSTOM)
        const std::vector<byte>& customPresetData,// Custom preset data (if any)
        String& samples,                          // Raw signal data (as a String)
        float frequency                           // Frequency of the signal
    );

private:
    static const std::map<CC1101_PRESET, std::string> presetMapping;

    static void writeHeader(File& file, float frequency);
    static void writePresetInfo(File& file, const CC1101_PRESET& presetName, const std::vector<byte>& customPresetData);
    static void writeRawProtocolData(File& file, String& samples);
    static std::string getPresetName(const CC1101_PRESET& preset);
};

#endif // FLIPPER_SUB_FILE_H