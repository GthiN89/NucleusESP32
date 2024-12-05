#include "TV-B-Gone.h"
#include "WORLD_IR_CODES.h"
#include "core/globals.h"
#include "ScreenManager.h"
#include "SDcard.h"
#include <IRutils.h>
#include <IRrecv.h>
#include <IRsend.h>

// Globals
static uint8_t region;
static uint8_t num_NAcodes = NUM_ELEM(NApowerCodes);
static uint8_t num_EUcodes = NUM_ELEM(EUpowerCodes);
static IRsend irsend(IR_TX);
static ScreenManager& screenMgr = ScreenManager::getInstance();
static SDcard& sdCard = SDcard::getInstance();

// Forward Declarations
void sendAllPowerCodes();
void displayMessage(const char* message);

void startTvBGone() {
    irsend.begin();

    // Display menu for region selection
    lv_obj_t* textArea = screenMgr.getTextArea();
    lv_textarea_set_text(textArea, "Select Region:\n1. North America\n2. Europe");
    
    // Simple selection logic (can replace with UI callbacks)
    int selectedRegion = NA; // Replace with actual selection logic
    region = (selectedRegion == NA) ? NA : EU;

    // Display message and send power codes
    const char* regionMsg = (region == NA) ? "Sending NA codes..." : "Sending EU codes...";
    displayMessage(regionMsg);
    sendAllPowerCodes();
    displayMessage("All codes sent!");
}

void sendAllPowerCodes() {
    const IrCode* const* codes = (region == NA) ? NApowerCodes : EUpowerCodes;
    uint8_t numCodes = (region == NA) ? num_NAcodes : num_EUcodes;

    for (uint8_t i = 0; i < numCodes; i++) {
        const IrCode* code = codes[i];
        const uint8_t freq = code->timer_val;
        const uint8_t numpairs = code->numpairs;
        const uint8_t bitcompression = code->bitcompression;

        std::vector<uint16_t> rawData;
        rawData.reserve(numpairs * 2);

        uint8_t code_ptr = 0;
        uint8_t bitsleft = 0;
        uint8_t bits = 0;

        for (uint8_t k = 0; k < numpairs; k++) {
            uint16_t ti = 0;

            for (uint8_t b = 0; b < bitcompression; b++) {
                if (bitsleft == 0) {
                    bits = code->codes[code_ptr++];
                    bitsleft = 8;
                }
                bitsleft--;
                ti |= ((bits >> bitsleft) & 1) << (bitcompression - 1 - b);
            }
            ti *= 2;

            rawData.push_back(code->times[ti] * 10);
            rawData.push_back(code->times[ti + 1] * 10);
        }

        // Send the IR signal
        irsend.sendRaw(rawData.data(), rawData.size(), 1000000 / freq);
        delay(50); // Delay between transmissions
    }
}

void sendCustomIRCodes() {
    // Retrieve file path from the SD card using the file explorer
    const char* filePath = "/IR_Custom_Codes.txt"; // Replace with actual file selection logic

    if (!sdCard.fileExists(filePath)) {
        displayMessage("File not found!");
        return;
    }

    File32* file = sdCard.createOrOpenFile(filePath, FILE_READ);
    if (!file) {
        displayMessage("Failed to open file!");
        return;
    }

    std::vector<uint16_t> rawData;
    while (file->available()) {
        String line = file->readStringUntil('\n');
        line.trim();

        if (line.startsWith("frequency:")) {
            uint16_t frequency = line.substring(10).toInt();
            // Handle frequency
        } else if (line.startsWith("data:")) {
            String rawDataStr = line.substring(5);
            rawData.clear();

            while (!rawDataStr.isEmpty()) {
                int spaceIndex = rawDataStr.indexOf(' ');
                if (spaceIndex == -1) {
                    spaceIndex = rawDataStr.length();
                }
                rawData.push_back(rawDataStr.substring(0, spaceIndex).toInt());
                rawDataStr.remove(0, spaceIndex + 1);
            }

            irsend.sendRaw(rawData.data(), rawData.size(), 38000); // Example frequency
            delay(50);
        }
    }

    sdCard.closeFile(file);
    displayMessage("Custom codes sent!");
}

void displayMessage(const char* message) {
    lv_obj_t* textArea = screenMgr.getTextArea();
    lv_textarea_set_text(textArea, message);
}
