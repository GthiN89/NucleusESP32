#ifndef PN532_EMV_READER_HPP
#define PN532_EMV_READER_HPP

/*
  Copyright (C) 2014 Alexis Guillard, Maxime Marches, Thomas Brunner
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  
  File written for the requirements of our MSc Project at the University of Kent, Canterbury, UK
  
  Retrieves information available from EMV smartcards via an RFID/NFC reader.
  Both tracks are printed then track2 is parsed to retrieve PAN and expiry date.
  The paylog is parsed and showed as well.
  
  All these information are stored in plaintext on the card and available to anyone.
*/

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <vector>
#include <cstring>
#include "globals.h"

#define MAX_FRAME_LEN 256
#define DEBUG 1

struct APDU {
    size_t size;
    uint8_t data[MAX_FRAME_LEN];
};

struct Application {
    uint8_t aid[8];
    uint8_t priority;
    char name[17];
};

class PN532_EMV_Reader {
public:
PN532_EMV_Reader(uint8_t sck = PN532_SCK, uint8_t miso = PN532_MISO, 
                uint8_t mosi = PN532_MOSI, uint8_t ss = PN532_SS);
    
    bool begin();
    void processEMVCard();
    unsigned char* executeCommand(  uint8_t* command, size_t size);
private:
    Adafruit_PN532 nfc;
    uint8_t abtRx[MAX_FRAME_LEN];
    int szRx;
    
    // EMV Commands
    static   uint8_t START_14443A[3];
    static   uint8_t SELECT_APP_HEADER[6];
    static   uint8_t SELECT_PPSE[22];
    static   uint8_t GPO_HEADER[6];
    static   uint8_t READ_RECORD[7];
    static   uint8_t GET_DATA_LOG_FORMAT[7];
    static   uint8_t SELECT_APP[13];
    static   uint8_t GET_PROCESSING_OPTIONS[8];
    
    // Helper methods
    bool checkTrailer();
    std::vector<Application> getAllApplications();
    APDU selectByPriority(  std::vector<Application>& list, uint8_t priority);


    void printApplicationList(  std::vector<Application>& list);

    #include <string>
    void printHex(uint8_t* data, size_t size, const std::string& label = "");
    void printChar(uint8_t* str, size_t size, const std::string& label = "");
};

#endif