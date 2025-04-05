#include "EMVGrabber.hpp"
#include "modules/nfc/nfc.h"
// Command Definitions
  uint8_t PN532_EMV_Reader::START_14443A[3] = {0x4A, 0x01, 0x00};
  uint8_t PN532_EMV_Reader::SELECT_APP_HEADER[6] = {0x40, 0x01, 0x00, 0xA4, 0x04, 0x00};
  uint8_t PN532_EMV_Reader::SELECT_PPSE[22] = {
    0x40, 0x01, 0x00, 0xA4, 0x04, 0x00, 0x0E,
    0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00};
  uint8_t PN532_EMV_Reader::GPO_HEADER[6] = {0x40, 0x01, 0x80, 0xA8, 0x00, 0x00};
  uint8_t PN532_EMV_Reader::READ_RECORD[7] = {0x40, 0x01, 0x00, 0xB2, 0x00, 0x00, 0x00};
  uint8_t PN532_EMV_Reader::GET_DATA_LOG_FORMAT[7] = {0x40, 0x01, 0x80, 0xCA, 0x9F, 0x4F, 0x00};
  uint8_t PN532_EMV_Reader::SELECT_APP[13] = 
    {0x00, 0xA4, 0x04, 0x00, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x03, 0x10, 0x10, 0x00};
  uint8_t PN532_EMV_Reader::GET_PROCESSING_OPTIONS[8] = 
    {0x80, 0xA8, 0x00, 0x00, 0x02, 0x83, 0x00, 0x00};

PN532_EMV_Reader::PN532_EMV_Reader(uint8_t sck, uint8_t miso, uint8_t mosi, uint8_t ss) 
    : nfc(sck, miso, mosi, ss) {}

bool PN532_EMV_Reader::begin() {

    digitalWrite(CC1101_CS, HIGH);
    digitalWrite(SD_CS, HIGH);
    digitalWrite(PN532_SS, LOW);
    delay(5);
    SPI.end();
    delay(5);
    // has to be fast to dump the entire memory contents!
    SPI.begin(CYD_SCLK, CYD_MISO, CYD_MOSI, PN532_SS);
    //SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);  
    SPI.setFrequency(30000000);   

    delay(5);

    nfc.begin();
    
    uint32_t version = nfc.getFirmwareVersion();
    if (!version) {
        Serial.println("PN532 not found!");
        return false;
    }
    
    nfc.SAMConfig();
    Serial.println("Waiting for EMV card...");
    return true;
}

bool PN532_EMV_Reader::checkTrailer() {
    if (szRx < 2) return true;
    return !(abtRx[szRx - 2] == 0x90 && abtRx[szRx - 1] == 0);
}

std::vector<Application> PN532_EMV_Reader::getAllApplications() {
    // std::vector<Application> list;
    // APDU res = executeCommand(SELECT_PPSE, sizeof(SELECT_PPSE), "SELECT PPSE");
    
    // if (res.size == 0) return list;

    // for (size_t i = 0; i < szRx; ++i) {
    //     if (abtRx[i] == 0x61) { // Application template
    //         Application app;
    //         ++i;
    //         while (i < szRx && abtRx[i] != 0x61) {
    //             if (abtRx[i] == 0x4F) { // Application ID
    //                 uint8_t len = abtRx[++i];
    //                 i++;
    //                 if (len != 7) {
    //                     Serial.println("Application id larger than 7 bytes");
    //                 }
    //                 memcpy(app.aid, &abtRx[i], len);
    //                 i += len - 1;
    //             }

    //             if (abtRx[i] == 0x87) { // Application Priority
    //                 i += 2;
    //                 app.priority = abtRx[i];
    //             }
    //             ++i;

    //             if (abtRx[i] == 0x50) { // Application label
    //                 uint8_t len = abtRx[++i];
    //                 i++;
    //                 memcpy(app.name, &abtRx[i], len);
    //                 app.name[len] = 0;
    //                 i += len - 1;
    //             }
    //         }
    //         list.push_back(app);
    //         --i;
    //     }
    // }
    // return list;
}

// APDU PN532_EMV_Reader::selectByPriority(  std::vector<Application>& list, uint8_t priority) {
//     Application app;
//     for (  auto& a : list) {
//         if (a.priority == priority) {
//             app = a;
//             break;
//         }
//     }
    
//     uint8_t select_app[256] = {0};
//     uint8_t size = 0;
    
//     memcpy(select_app, SELECT_APP_HEADER, sizeof(SELECT_APP_HEADER));
//     size = sizeof(SELECT_APP_HEADER);
    
//     select_app[size] = sizeof(app.aid);
//     size += 1;
    
//     memcpy(select_app + size, app.aid, sizeof(app.aid));
//     size += sizeof(app.aid);
//     size += 1;
    
//     return executeCommand(select_app, size, "SELECT APP");
// }

// Update executeCommand to use the new sendAPDU
unsigned char* PN532_EMV_Reader::executeCommand(  uint8_t* command, size_t size){

uint8_t response[256];
uint8_t responseLength = sizeof(response);

// Temporary non-  buffer for sending
uint8_t sendBuffer[256];
memcpy(sendBuffer, command, size);

nfc.writecommand(command, size);
nfc.waitready(1000);
nfc.readdata(response, responseLength);

return response;
}



void PN532_EMV_Reader::printApplicationList(  std::vector<Application>& list) {
    Serial.print(list.size());
    Serial.println(" Application(s) found:");
    Serial.println("-----------------");

    for (  auto& a : list) {
        Serial.print("Name: ");
        Serial.println(a.name);
        Serial.print("Priority: ");
        Serial.println((char)('0' + a.priority));
        printHex(a.aid, sizeof(a.aid), "AID");
        Serial.println("\n-----------------");
    }
}

void PN532_EMV_Reader::processEMVCard() {
    uint8_t uid[7];
    uint8_t uidLength;

    if (!nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
        delay(500);
        return;
    }

    Serial.println("\n===== CARD DETECTED =====");
    printHex(uid, uidLength, "UID");

    // Get all applications
    auto apps = getAllApplications();
    if (apps.empty()) {
        Serial.println("No applications found");
        return;
    }

#ifdef DEBUG
    printApplicationList(apps);
#endif

    // Process each application
    for (  auto& app : apps) {
        APDU res = selectByPriority(apps, app.priority);
        if (res.size == 0) {
            Serial.print("Failed to select app ");
            Serial.println(app.name);
            continue;
        }

        // Get Processing Options
        uint8_t gpoResponse[256];
        uint8_t gpoResponseLen = sizeof(gpoResponse);
        if (!nfc.inDataExchange(GET_PROCESSING_OPTIONS, sizeof(GET_PROCESSING_OPTIONS), 
                              gpoResponse, &gpoResponseLen)) {
            Serial.println("Failed to get Processing Options");
            continue;
        }
        printHex(gpoResponse, gpoResponseLen, "GPO Response");
    }
    delay(2000);
}