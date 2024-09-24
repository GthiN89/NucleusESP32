#include "CC1101.h"
#include "Arduino.h"
#include "ELECHOUSE_CC1101_SRC_DRV.h"
#include "globals.h"
#include "FS.h"
#include "GUI/ScreenManager.h"
#include <sstream>
#include <ctime>
#include "FS.h"
#include <LittleFS.h>
#include "SPIFFS.h"
#include <vector>
#include "FlipperSubFile.h"
#include "RCSwitch.h"
#include "modules/ETC/SDcard.h"
#include <string> 

using namespace std;

#define SAMPLE_SIZE 4096

int receiverGPIO;

CC1101_PRESET C1101preset = AM650;
int CC1101_PKT_FORMAT = 0;

int CC1101_MODULATION = 2;

int CC1101_SYNC = 1;

int samplecount;
const int minsample = 30;
static unsigned long lastTime = 0;

int sample[SAMPLE_SIZE];

int error_toleranz = 200;
unsigned long samplesmooth[SAMPLE_SIZE];


String fullPath;   

RCSwitch mySwitch;

void IRAM_ATTR InterruptHandler()
{    

  const long time = micros();
  const unsigned int duration = time - lastTime;

  if (duration > 100000){
    samplecount = 0;
  }

  if (duration >= 100){
    sample[samplecount++] = duration;
  }

  if (samplecount>=SAMPLE_SIZE){
    detachInterrupt(CC1101_CCGDO0A);
//    detachInterrupt(RXPin);
   CC1101_CLASS::CheckReceived();
  }


   if (CC1101_MODULATION == 0) {
    if (samplecount == 1 and digitalRead(CC1101_CCGDO0A) != HIGH){
      samplecount = 0;
    }
  }
  
  
  lastTime = time;


    // if (!receiverEnabled)
    // {
    //     return;
    // }
    
    // const long time = micros();
    // const unsigned int duration = time - lastTime;

    // if (duration > 100000)
    // {
    //     samplecount = 0;
    // }

    // if (duration >= 100)
    // {
    //     sample[samplecount++] = duration;
    // }

    // if (samplecount >= SAMPLE_SIZE)
    // {
    //     return;
    // }

    // if (CC1101_MODULATION == 0)
    // {
    //     if (samplecount == 1 && digitalRead(receiverGPIO) != HIGH)
    //     {
    //         samplecount = 0;
    //     }
    // }

    // lastTime = time;
}

void CC1101_CLASS::setFrequency(float freq)
{
    CC1101_MHZ = freq;
    ELECHOUSE_cc1101.setMHZ(CC1101_MHZ);
}

void CC1101_CLASS::setSync(int sync)
{
    CC1101_SYNC = sync;
}

void CC1101_CLASS::setPTK(int ptk)
{
    CC1101_PKT_FORMAT = ptk;
}


void CC1101_CLASS::enableReceiver()
{
    memset(sample, 0, sizeof(SAMPLE_SIZE));
    samplecount = 0;

    ELECHOUSE_cc1101.Init();

    if (CC1101_MODULATION == 2)
    {
        ELECHOUSE_cc1101.setDcFilterOff(0);
    }

    if (CC1101_MODULATION == 0)
    {
        ELECHOUSE_cc1101.setDcFilterOff(1);
    }

    // ELECHOUSE_cc1101.setDcFilterOff(1);
    ELECHOUSE_cc1101.setSyncMode(CC1101_SYNC);  // Combined sync-word qualifier mode. 0 = No preamble/sync. 1 = 16 sync word bits detected. 2 = 16/16 sync word bits detected. 3 = 30/32 sync word bits detected. 4 = No preamble/sync, carrier-sense above threshold. 5 = 15/16 + carrier-sense above threshold. 6 = 16/16 + carrier-sense above threshold. 7 = 30/32 + carrier-sense above threshold.
    ELECHOUSE_cc1101.setPktFormat(CC1101_PKT_FORMAT); // Format of RX and TX data. 0 = Normal mode, use FIFOs for RX and TX.
                                                      // 1 = Synchronous serial mode, Data in on GDO0 and data out on either of the GDOx pins.
                                                      // 2 = Random TX mode; sends random data using PN9 generator. Used for test. Works as normal mode, setting 0 (00), in RX.
                                                      // 3 = Asynchronous serial mode, Data in on GDO0 and data out on either of the GDOx pins.
                                                      // ELECHOUSE_cc1101.setSyncMode(3);       
    ELECHOUSE_cc1101.setModulation(CC1101_MODULATION); // set modulation mode. 0 = 2-FSK, 1 = GFSK, 2 = ASK/OOK, 3 = 4-FSK, 4 = MSK.
    ELECHOUSE_cc1101.setMHZ(CC1101_MHZ);               // Here you can set your basic frequency. The lib calculates the frequency automatically (default = 433.92).The cc1101 can: 300-348 MHZ, 387-464MHZ and 779-928MHZ. Read More info from datasheet.
    ELECHOUSE_cc1101.setDeviation(CC1101_DEVIATION);
    ELECHOUSE_cc1101.setDRate(CC1101_DRATE); // Set the Data Rate in kBaud. Value from 0.02 to 1621.83. Default is 99.97 kBaud!
    ELECHOUSE_cc1101.setRxBW(CC1101_RX_BW);  // Set the Receive Bandwidth in kHz. Value from 58.03 to 812.50. Default is 812.50 kHz.

  pinMode(CC1101_CCGDO0A,INPUT);
  digitalPinToInterrupt(CC1101_CCGDO0A);
  ELECHOUSE_cc1101.SetRx();
  samplecount = 0;
  attachInterrupt(CC1101_CCGDO0A, InterruptHandler, CHANGE); /// predelat interupt
  
//   pinMode(CC1101_CCGDO2A,INPUT);
//   digitalPinToInterrupt(CC1101_CCGDO2A);
//   ELECHOUSE_cc1101.SetRx();
//   samplecount = 0;
//   attachInterrupt(RXPin, receiver, CHANGE);

    
//     pinMode(CC1101_CCGDO0A, INPUT);
//     receiverGPIO = digitalPinToInterrupt(CC1101_CCGDO0A);    
//     ELECHOUSE_cc1101.SetRx();
// /////////////////////////////
//     receiverEnabled = true;
////////////////////////////////
   // attachInterrupt(receiverGPIO, InterruptHandler, CHANGE);
}

void CC1101_CLASS::setCC1101Preset(CC1101_PRESET preset) {
    C1101preset = preset;
}

bool CC1101_CLASS::init()
{
    ELECHOUSE_cc1101.setSpiPin(CC1101_SCLK, CC1101_MISO, CC1101_MOSI, CC1101_CS);
    ELECHOUSE_cc1101.Init();

    if (ELECHOUSE_cc1101.getCC1101())
    {
        ELECHOUSE_cc1101.setSidle();
        return true;
    }
    else
    {
        ELECHOUSE_cc1101.setSidle();
        return false;
    }
}

void CC1101_CLASS::showResultRecPlay()
{
    ScreenManager& screenMgr = ScreenManager::getInstance();

    lv_obj_t * text_area = screenMgr.getTextArea();
    String rawString = "";

    for (int i = 1; i < samplecount; i++)
    {
        rawString += sample[i];
        rawString += ",";
    }

     lv_textarea_add_text(text_area, "Raw: ");
     lv_textarea_add_text(text_area, String(rawString).c_str());
     lv_textarea_add_text(text_area, String("Capture Complete | Sample: " + String(samplecount)).c_str());
     Serial.print("\n RAW: \n");   
     Serial.print(String(rawString).c_str());
     Serial.print("\n RAW: \n");   
     Serial.print(String("Capture Complete | Sample: " + String(samplecount)).c_str());

    rawString = "";
}


void CC1101_CLASS::disableReceiver()
{
    //detachInterrupt((uint8_t)receiverGPIO);
    receiverEnabled = false;
    ELECHOUSE_cc1101.setSidle();
    ELECHOUSE_cc1101.goSleep();
    digitalWrite(CC1101_CS, HIGH);  
}

void CC1101_CLASS::loadPreset() {
    switch (C1101preset) {
        case AM650:
            CC1101_MODULATION = 2;
            CC1101_DRATE = 3.79372;
            CC1101_RX_BW = 650.00;
            CC1101_DEVIATION = 1.58;
            break;
        case AM270:
            CC1101_MODULATION = 2;
            CC1101_DRATE = 3.79372;
            CC1101_RX_BW = 270.833333;
            CC1101_DEVIATION = 1.58;
            break;
        case FM238:
            CC1101_MODULATION = 0;
            CC1101_DRATE = 4.79794;
            CC1101_RX_BW = 270.833333;
            CC1101_DEVIATION = 2.380371;
            break;
        case FM476:
            CC1101_MODULATION = 0;
            CC1101_DRATE = 4.79794;
            CC1101_RX_BW = 270.833333;
            CC1101_DEVIATION = 47.60742;
            break;
        case FM95:
            CC1101_MODULATION = 0;
            CC1101_DRATE = 4.798;
            CC1101_RX_BW = 270;
            CC1101_DEVIATION = 9.521;
            CC1101_SYNC_MODE = 6;
            break;
        case FM15k:
            CC1101_MODULATION = 0;
            CC1101_DRATE = 3.794;
            CC1101_RX_BW = 135;
            CC1101_DEVIATION = 15.869;
            CC1101_SYNC_MODE = 7;
            break;
        case PAGER:
            CC1101_MODULATION = 0;
            CC1101_DRATE = 0.625;
            CC1101_RX_BW = 270;
            CC1101_DEVIATION = 5.157;
            CC1101_SYNC_MODE = 6;
            break;
        case HND1:
            CC1101_MODULATION = 0;
            CC1101_DRATE = 15.357;
            CC1101_RX_BW = 270;
            CC1101_DEVIATION = 15.869;
            CC1101_SYNC_MODE = 6;
            break;
        case HND2:
            CC1101_MODULATION = 0;
            CC1101_DRATE = 15.357;
            CC1101_RX_BW = 67;
            CC1101_DEVIATION = 15.869;
            CC1101_SYNC_MODE = 7;
            break;
        default:
            break;
    }
}

bool CC1101_CLASS::CheckReceived()
{
    // Serial.print(String(samplecount));
    // Serial.print("\n");
    // delay(1);
    // if (samplecount >= minsample && micros() - lastTime > 100000)
    // {
    //     receiverEnabled = false;
    //     return 1;
    // }
    // else
    // {
    //     return 0;
    // }

   delay(1);
  if (samplecount >= minsample && micros()-lastTime >100000){
    return 1;
  }else{
    return 0;
  }
}

void CC1101_CLASS::signalanalyse(){
  #define signalstorage 10
  bool lastbin=0;

  ScreenManager& screenMgr = ScreenManager::getInstance();
  lv_obj_t* text_area = screenMgr.getTextArea();
  lv_textarea_set_text(text_area, "");
  int signalanz=0;
  int timingdelay[signalstorage];
  float pulse[signalstorage];
  long signaltimings[signalstorage*2];
  int signaltimingscount[signalstorage];
  long signaltimingssum[signalstorage];
  long signalsum=0;

  for (int i = 0; i<signalstorage; i++){
    signaltimings[i*2] = 100000;
    signaltimings[i*2+1] = 0;
    signaltimingscount[i] = 0;
    signaltimingssum[i] = 0;
  }
  for (int i = 1; i<samplecount; i++){
    signalsum+=sample[i];
  }

  for (int p = 0; p<signalstorage; p++){

  for (int i = 1; i<samplecount; i++){
    if (p==0){
      if (sample[i]<signaltimings[p*2]){
        signaltimings[p*2]=sample[i];
      }
    }else{
      if (sample[i]<signaltimings[p*2] && sample[i]>signaltimings[p*2-1]){
        signaltimings[p*2]=sample[i];
      }
    }
  }

  for (int i = 1; i<samplecount; i++){
    if (sample[i]<signaltimings[p*2]+error_toleranz && sample[i]>signaltimings[p*2+1]){
      signaltimings[p*2+1]=sample[i];
    }
  }

  for (int i = 1; i<samplecount; i++){
    if (sample[i]>=signaltimings[p*2] && sample[i]<=signaltimings[p*2+1]){
      signaltimingscount[p]++;
      signaltimingssum[p]+=sample[i];
    }
  }
  }
  
  int firstsample = signaltimings[0];
  
  signalanz=signalstorage;
  for (int i = 0; i<signalstorage; i++){
    if (signaltimingscount[i] == 0){
      signalanz=i;
      i=signalstorage;
    }
  }

  for (int s=1; s<signalanz; s++){
  for (int i=0; i<signalanz-s; i++){
    if (signaltimingscount[i] < signaltimingscount[i+1]){
      int temp1 = signaltimings[i*2];
      int temp2 = signaltimings[i*2+1];
      int temp3 = signaltimingssum[i];
      int temp4 = signaltimingscount[i];
      signaltimings[i*2] = signaltimings[(i+1)*2];
      signaltimings[i*2+1] = signaltimings[(i+1)*2+1];
      signaltimingssum[i] = signaltimingssum[i+1];
      signaltimingscount[i] = signaltimingscount[i+1];
      signaltimings[(i+1)*2] = temp1;
      signaltimings[(i+1)*2+1] = temp2;
      signaltimingssum[i+1] = temp3;
      signaltimingscount[i+1] = temp4;
    }
  }
  }

  for (int i=0; i<signalanz; i++){
    timingdelay[i] = signaltimingssum[i]/signaltimingscount[i];
  }

  if (firstsample == sample[1] and firstsample < timingdelay[0]){
    sample[1] = timingdelay[0];
  }


 
  for (int i=1; i<samplecount; i++){
    float r = (float)sample[i]/timingdelay[0];
    int calculate = r;
    r = r-calculate;
    r*=10;
    if (r>=5){calculate+=1;}
    if (calculate>0){
      if (lastbin==0){
        lastbin=1;
      }else{
      lastbin=0;
    }
      if (lastbin==0 && calculate>8){
        Serial.print(" [Pause: ");
            lv_textarea_add_text(text_area, "");
            string v = to_string(sample[i]);
lv_textarea_add_text(text_area, v.c_str());
         Serial.print(sample[i]);
        // Serial.println(" samples]");
        // appendFile(SD, "/logs.txt",NULL, " [Pause: ");
        // appendFileLong(SD, "/logs.txt", sample[i]);
        // appendFile(SD, "/logs.txt"," samples]", "\n");
      }else{
        for (int b=0; b<calculate; b++){
          Serial.print(lastbin);
         // lv_textarea_add_text(text_area, lastbin);
    //      appendFileLong(SD, "/logs.txt", lastbin);
        }
      }
    }
  }

  Serial.println();
  Serial.print("Samples/Symbol: ");
   lv_textarea_add_text(text_area, "Samples/Symbol: ");
  Serial.println(timingdelay[0]);
std::string timingdelayStr = std::to_string(timingdelay[0]);
const char* timingdelay1 = timingdelayStr.c_str();
   lv_textarea_add_text(text_area, timingdelay1);
  Serial.println();


  int smoothcount=0;
  for (int i=1; i<samplecount; i++){
    float r = (float)sample[i]/timingdelay[0];
    int calculate = r;
    r = r-calculate;
    r*=10;
    if (r>=5){calculate+=1;}
    if (calculate>0){
      samplesmooth[smoothcount] = calculate*timingdelay[0];
      smoothcount++;
    }
  }
  Serial.println("Rawdata corrected:");
  Serial.print("Count=");
  Serial.println(smoothcount+1);
  lv_textarea_add_text(text_area, "Rawdata corrected:");
  lv_textarea_add_text(text_area, "Count=");
  lv_textarea_add_text(text_area, to_string(smoothcount+1).c_str());
  for (int i=0; i<smoothcount; i++){
    Serial.print(samplesmooth[i]);
    Serial.print(",");
  }
  Serial.println();
  Serial.println();
  
  return;
}


bool CC1101_CLASS::captureLoop()
{
    if (C1101preset != AM650 && 
        C1101preset != AM270 && 
        C1101preset != FM238 && 
        C1101preset != FM476) 
    {
        C1101preset = CUSTOM;
    }

     File outputFile;
    // if (CheckReceived())
    // {
    //    CC1101_CLASS::signalanalyse();
       std::stringstream rawSignal;

        for (int i = 0; i < samplecount; i++) {
            rawSignal << (i > 0 ? (i % 2 == 1 ? " -" : " ") : "");
            rawSignal << sample[i];
        }
        
            
            String  filename = generateFilename(CC1101_MHZ, CC1101_MODULATION, CC1101_RX_BW);
                    fullPath = "/"  + filename; 
            outputFile = SPIFFS.open(fullPath.c_str(), "w");
         if (outputFile) {
             std::vector<byte> customPresetData;
             if (C1101preset == CC1101_PRESET::CUSTOM) { 
                    customPresetData.insert(customPresetData.end(), {
                        CC1101_MDMCFG4, ELECHOUSE_cc1101.SpiReadReg(CC1101_MDMCFG4),
                        CC1101_MDMCFG3, ELECHOUSE_cc1101.SpiReadReg(CC1101_MDMCFG3),
                        CC1101_MDMCFG2, ELECHOUSE_cc1101.SpiReadReg(CC1101_MDMCFG2),
                        CC1101_DEVIATN, ELECHOUSE_cc1101.SpiReadReg(CC1101_DEVIATN),
                        CC1101_FREND0,  ELECHOUSE_cc1101.SpiReadReg(CC1101_FREND0),
                        0x00, 0x00
                    });
                std::array<byte,8> paTable;
                 ELECHOUSE_cc1101.SpiReadBurstReg(0x3E, paTable.data(), paTable.size());
                 customPresetData.insert(customPresetData.end(), paTable.begin(), paTable.end());
             }
             FlipperSubFile::generateRaw(outputFile, C1101preset, customPresetData, rawSignal, CC1101_MHZ);
             outputFile.close();
         } else {
             return false;
         }
         signalanalyse();
         return true;
     }
//     return false;
// }
   
String CC1101_CLASS::generateFilename(float frequency, int modulation, float bandwidth)
{
    char filenameBuffer[100];

    sprintf(filenameBuffer, "%d_%s_%d_%s.sub", static_cast<int>(frequency * 100), modulation == 2 ? "AM" : "FM", static_cast<int>(bandwidth),
            generateRandomString(8).c_str());

    return String(filenameBuffer);
}

String CC1101_CLASS::generateRandomString(int length)
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    const std::string characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    std::stringstream ss;
    for (int i = 0; i < length; ++i) {
        int randomIndex = std::rand() % characters.size();
        char randomChar = characters[randomIndex];
        ss << randomChar;
    }

    return String(ss.str().c_str());
}

bool CC1101_CLASS::sendCapture()
{
    CC1101_CLASS::enableTransmit();

    for (int i = 1; i < samplecount; i += 2)
    {
        digitalWrite(CC1101_CCGDO0A, HIGH);
        delayMicroseconds(sample[i]);
        digitalWrite(CC1101_CCGDO0A, LOW);
        delayMicroseconds(sample[i + 1]);
    }

    for (int i = 1; i < samplecount; i += 2)
    {
        digitalWrite(CC1101_CCGDO0A, LOW);
        delayMicroseconds(sample[i]);
        digitalWrite(CC1101_CCGDO0A, HIGH);
        delayMicroseconds(sample[i + 1]);
    }


    CC1101_CLASS::disableTransmit();

    ScreenManager& screenMgr = ScreenManager::getInstance();
    lv_obj_t * text_area = screenMgr.getTextArea();

    lv_textarea_add_text(text_area, String("Playback Complete ! Sample: " + String(samplecount)).c_str());

    return true;
}

void CC1101_CLASS::enableTransmit()
{
    digitalWrite(CC1101_CS, LOW);
    ELECHOUSE_cc1101.Init();
    ELECHOUSE_cc1101.setMHZ(CC1101_MHZ);               // Here you can set your basic frequency. The lib calculates the frequency automatically (default = 433.92).The cc1101 can: 300-348 MHZ, 387-464MHZ and 779-928MHZ. Read More info from datasheet.
    ELECHOUSE_cc1101.setModulation(CC1101_MODULATION); // set modulation mode. 0 = 2-FSK, 1 = GFSK, 2 = ASK/OOK, 3 = 4-FSK, 4 = MSK.
    ELECHOUSE_cc1101.setDeviation(CC1101_DEVIATION);
    ELECHOUSE_cc1101.setDRate(CC1101_DRATE); // Set the Data Rate in kBaud. Value from 0.02 to 1621.83. Default is 99.97 kBaud!
    ELECHOUSE_cc1101.setDcFilterOff(1);
    ELECHOUSE_cc1101.setSyncMode(0);
    ELECHOUSE_cc1101.setPktFormat(CC1101_PKT_FORMAT); // Format of RX and TX data. 0 = Normal mode, use FIFOs for RX and TX.
                                                      // 1 = Synchronous serial mode, Data in on GDO0 and data out on either of the GDOx pins.
                                                      // 2 = Random TX mode; sends random data using PN9 generator. Used for test. Works as normal mode, setting 0 (00), in RX.
                                                      // 3 = Asynchronous serial mode, Data in on GDO0 and data out on either of the GDOx pins.
                                                      // ELECHOUSE_cc1101.setSyncMode(3);        // Combined sync-word qualifier mode. 0 = No preamble/sync. 1 = 16 sync word bits detected. 2 = 16/16 sync word bits detected. 3 = 30/32 sync word bits detected. 4 = No preamble/sync, carrier-sense above threshold. 5 = 15/16 + carrier-sense above threshold. 6 = 16/16 + carrier-sense above threshold. 7 = 30/32 + carrier-sense above threshold.

    ELECHOUSE_cc1101.setPA(12);

    ELECHOUSE_cc1101.SetTx(); // set Transmit on

    mySwitch.enableTransmit(CC1101_CCGDO0A);
}

void CC1101_CLASS::disableTransmit()
{
    digitalWrite(CC1101_CCGDO0A, LOW);
    mySwitch.disableTransmit(); // set Transmit off
    ELECHOUSE_cc1101.setSidle();
    ELECHOUSE_cc1101.goSleep();
    digitalWrite(CC1101_CS, HIGH);
}

void CC1101_CLASS::saveSignal() {
    ScreenManager& scrMng = ScreenManager::getInstance();
    lv_obj_t *text_area = scrMng.getTextArea();
    File fileIntern = SPIFFS.open(fullPath, FILE_READ);
    std::string SDPath = std::string("/recordedRF") + fullPath.c_str();

    Serial.print(fullPath + "\n");

    // Ensure CC1101 is disabled before SD operation
    digitalWrite(CC1101_CS, HIGH);

    // Initialize SD card
    if (!SDInit()) {
        lv_textarea_add_text(text_area, "SD mounting failed!\n");
        Serial.println("SD mounting failed!");
        return;
    } else {
        lv_textarea_add_text(text_area, "SD mounted\n");
        Serial.println("SD mounted");
    }

    // Open file on SD card
    File fileSD = SD.open(SDPath.c_str(), FILE_WRITE);
    if (!fileSD) {
        lv_textarea_add_text(text_area, "Failed to open file on SD card\n");
        Serial.println("Failed to open file on SD card");
        return;
    }

    // Check if SPIFFS file is open
    if (!fileIntern) {
        lv_textarea_add_text(text_area, "Failed to open SPIFFS file\n");
        Serial.println("Failed to open SPIFFS file");
        fileSD.close();
        return;
    }

    // Copy data from SPIFFS file to SD card file
    while (fileIntern.available()) {
        fileSD.write(fileIntern.read());
    }

    lv_textarea_add_text(text_area, "Signal has been saved to SD card\n");
    Serial.println("Signal has been saved to SD card");

    // Close files and end SD transactions
    fileIntern.close();
    fileSD.close();
    SD.end();
    disconnectSD();

    // Remove the original SPIFFS file
    SPIFFS.remove(fullPath);
}






// bool CC1101_CLASS::CheckReceived() {
//   unsigned long previousMillis = millis();
  
//     RCSwitch rcswitch = RCSwitch();
//     RfCodes received;
//         ScreenManager& scrMng = ScreenManager::getInstance();
//     lv_obj_t *text_area = scrMng.getTextArea();
//     File fileIntern = SPIFFS.open(fullPath, FILE_READ);

// //    drawMainBorder();
//     // tft.setCursor(10, 28);
//     // tft.setTextSize(FP);
//     Serial.println("Waiting for signal.");
//     char hexString[64];
    
//     if(!CC1101_MHZ) CC1101_MHZ = CC1101_MHZ; // default from settings
    

//     // init receive
//     pinMode(CC1101_CCGDO0A, INPUT);
//     rcswitch.enableReceive(CC1101_CCGDO0A);

//     while(true) {
//         if(rcswitch.available()) {
//             long value = rcswitch.getReceivedValue();
//             Serial.println("Available");
//             if(value) {
//                 Serial.println("has value");

//                 unsigned int* raw = rcswitch.getReceivedRawdata();
//                 received.frequency=long(CC1101_MHZ*1000000);
//                 received.key=rcswitch.getReceivedValue();
//                 received.protocol="RcSwitch";
//                 received.preset=rcswitch.getReceivedProtocol();
//                 received.te=rcswitch.getReceivedDelay();
//                 received.Bit=rcswitch.getReceivedBitlength();
//                 received.filepath="Last copied";

//                 for(int i=0; i<received.te*2;i++) {
//                     if(i>0) received.data+=" ";
//                     received.data+=raw[i];
//                 }
//                 //Serial.println(received.protocol);
//                 //Serial.println(received.data);
//                 // const char* b = dec2binWzerofill(received.key, received.Bit);
//                 // drawMainBorder();
//                 // tft.setCursor(10, 28);
//                 // tft.setTextSize(FP);
//             //    decimalToHexString(received.key,hexString); // need to remove the extra padding 0s?
//                 Serial.println("Key: " + String(hexString));
//            //     tft.setCursor(10, tft.getCursorY());
//           //      Serial.println("Binary: " + String(b));
//            //     tft.setCursor(10, tft.getCursorY());
//                 Serial.println("Lenght: " + String(received.Bit) + " bits");
//           //      tft.setCursor(10, tft.getCursorY());
//                 Serial.println("PulseLenght: " + String(received.te) + "ms");
//            //     tft.setCursor(10, tft.getCursorY());
//                 Serial.println("Protocol: " + String(received.protocol));
//              //   tft.setCursor(10, tft.getCursorY()+LH*2);
//              //   Serial.println("Press " + String(BTN_ALIAS) + "for options.");
//             }
//             rcswitch.resetAvailable();
//             previousMillis = millis();
//         }
//         if(received.key>0) {
//             String subfile_out = "Filetype: Bruce SubGhz RAW File\nVersion 1\n";
//             subfile_out += "Frequency: " + String(int(CC1101_MHZ*1000000)) + "\n";
//             if(received.preset=="1") received.preset="FuriHalSubGhzPresetOok270Async";
//             else if (received.preset=="2") received.preset="FuriHalSubGhzPresetOok650Async";
//             subfile_out += "Preset: " + String(received.preset) + "\n";
//             subfile_out += "Protocol: RcSwitch\n";
//             subfile_out += "Bit: " + String(received.Bit) + "\n";
//             subfile_out += "Key: " + String(hexString) + "\n";
//             // subfile_out += "RAW_Data: " + received.data; // not in flipper pattern
//             subfile_out += "TE: " + String(received.te) + "\n";
            
//             #ifndef HAS_SCREEN
//                 // headless mode, just print the file on serial and quit
//                 Serial.println(subfile_out);
//                 return true;
//             #endif
            
//             if(true) {
//                 int chosen=0;
//                 // options = {
//                 //     {"Replay signal",   [&]()  { chosen=1; } },
//                 //     {"Save signal",     [&]()  { chosen=2; } },
//                 // };
//                 delay(200);
//               //  loopOptions(options);
//                 if(chosen==1) {
//                     rcswitch.disableReceive();
//                   //  sendRfCommand(received);
//                //     addToRecentCodes(received);
//                 //    displayRedStripe("Waiting Signal",TFT_WHITE, FGCOLOR);

//                 }
//                 else if (chosen==2) {
//                     int i=0;
//                     File file;
//                     String FS="";
//                     if(SD.begin()) {
//                         if (!SD.exists("/BruceRF")) SD.mkdir("/BruceRF");
//                         while(SD.exists("/BruceRF/bruce_" + String(i) + ".sub")) i++;
//                         file = SD.open("/BruceRF/bruce_"+ String(i) +".sub", FILE_WRITE);
//                         FS="SD";
//                     } else if(LittleFS.begin()) {
//                    //     if(!checkLittleFsSize()) goto Exit;
//                         if (!LittleFS.exists("/BruceRF")) LittleFS.mkdir("/BruceRF");
//                         while(LittleFS.exists("/BruceRF/bruce_" + String(i) + ".sub")) i++;
//                         file = LittleFS.open("/BruceRF/bruce_"+ String(i) +".sub", FILE_WRITE);
//                         FS="LittleFS";
//                     }
//                     if(file) {
//                         file.println(subfile_out);
//                   //      displaySuccess(FS + "/bruce_" + String(i) + ".sub");
//                     } else {
//                         Serial.println("Fail saving data to LittleFS");
//                 //        displayError("Error saving file");
//                     }
//                     file.close();
//                     delay(2000);
//                 //    drawMainBorder();
//                 //    tft.setCursor(10, 28);
//               //      tft.setTextSize(FP);
//                     Serial.println("Waiting for signal.");
//                 }
//             }
//         }
//     }

//     delay(1);
//             ELECHOUSE_cc1101.setSidle();

//     #ifdef USE_CC1101_VIA_SPI   
//     if(RfModule==1) 
//         ELECHOUSE_cc1101.setSidle();
//     #endif
        
//     return true;
// }


