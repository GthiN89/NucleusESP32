#ifndef FLIPPER_SUB_FILE_H
#define FLIPPER_SUB_FILE_H

#include <SdFat.h>
#include <string>
#include <vector>
#include <map>
#include "globals.h"


class FlipperSubFile {
public:
    /**
     * Generate a Flipper SubGhz RAW file.
     * @param file Reference to the SD card file to write data to.
     * @param presetName The CC1101 preset to be used (e.g., CUSTOM).
     * @param customPresetData Custom data if the preset is set to CUSTOM.
     * @param samples Raw signal data as a string of samples.
     * @param frequency The frequency of the signal in MHz.
     */
    void generateRaw(
File32&, CC1101_PRESET, const std::vector<unsigned char>&, std::ostringstream& , float
    );

private:
    /**
     * Writes the header information to the file.
     * @param file Reference to the SD card file.
     * @param frequency The frequency of the signal in MHz.
     */
    void writeHeader(File32& file, float frequency);

    /**
     * Writes preset information to the file.
     * @param file Reference to the SD card file.
     * @param presetName The CC1101 preset being used.
     * @param customPresetData Custom data for CUSTOM preset.
     */
    void writePresetInfo(File32& file, CC1101_PRESET presetName, const std::vector<byte>& customPresetData);

    /**
     * Writes the raw protocol data to the file.
     * @param file Reference to the SD card file.
     * @param samples String containing the raw signal samples.
     */
    void writeRawProtocolData(File32& file, std::ostringstream& samples);

    /**
     * Retrieves the name of the preset as a string.
     * @param preset The preset enum value.
     * @return A string representing the preset name.
     */
    std::string getPresetName(CC1101_PRESET preset);

    // Mapping from CC1101 preset enums to their string representations
    static const std::map<CC1101_PRESET, std::string> presetMapping;
};

#endif // FLIPPER_SUB_FILE_H
