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
#include "../../../../NucleusESP32/src/GUI/events.h"
#include "../ETC/SDcard.h"
#include "SPI.h"



using namespace std;
int receiverGPIO;

String rawString;

int CC1101_MODULATION = 2;

static unsigned long lastTime = 0;

int sample[SAMPLE_SIZE];

int error_toleranz = 200;

int samplecount = 0;


bool CC1101_is_initialized = false;
bool CC1101_recieve_is_running = false;
bool CC1101_transmit_is_running = false;
bool CC1101_isiddle = true;
bool CC1101_interup_attached = false;

int smoothcount;
unsigned long samplesmooth[SAMPLE_SIZE];


String fullPath;   

RCSwitch mySwitch;

RCSwitch CC1101_CLASS::getRCSwitch() {
 return mySwitch;
}

//SPIClass CC1101SPI;

void IRAM_ATTR InterruptHandler()
{    

    if (!receiverEnabled)
    {
        return;
    }
    
    const long time = micros();
    const unsigned int duration = time - lastTime;

    if (duration > 100000)
    {
        samplecount = 0;
    }

    if (duration >= 100)
    {
        sample[samplecount++] = duration;
    }

    if (samplecount >= SAMPLE_SIZE)
    {
        return;
    }

    if (CC1101_MODULATION == 0)
    {
        if (samplecount == 1 && digitalRead(receiverGPIO) != HIGH)
        {
            samplecount = 0;
        }
    }

    lastTime = time;
}

bool CC1101_CLASS::init()
{
//    digitalWrite(SDCARD_CS, HIGH);

  //  digitalWrite(CC1101_CS, LOW);

    ELECHOUSE_cc1101.setSpiPin(CC1101_SCLK, CC1101_MISO, CC1101_MOSI, CC1101_CS);
    ELECHOUSE_cc1101.Init();

    if (ELECHOUSE_cc1101.getCC1101())
    {
        ELECHOUSE_cc1101.setGDO0(CCGDO0A);
        ELECHOUSE_cc1101.setSidle();
        CC1101_isiddle = true;
        CC1101_is_initialized = true;
        return true;
    }
    else
    {
        ELECHOUSE_cc1101.setSidle();
        CC1101_isiddle = true;
        return false;
    }
    
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
    if (!CC1101_is_initialized) {
        CC1101_CLASS::init();
    }
    CC1101_CLASS::loadPreset();

    memset(sample, 0, sizeof(SAMPLE_SIZE));
    samplecount = 0;

   // ELECHOUSE_cc1101.Init();

    if (CC1101_MODULATION == 2)
    {
        ELECHOUSE_cc1101.setDcFilterOff(0);
    }

    if (CC1101_MODULATION == 0)
    {
        ELECHOUSE_cc1101.setDcFilterOff(1);
    }

     ELECHOUSE_cc1101.setDcFilterOff(1);
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
    
    pinMode(CC1101_CCGDO0A, INPUT);
    receiverGPIO = digitalPinToInterrupt(CC1101_CCGDO0A);    
    ELECHOUSE_cc1101.SetRx();
/////////////////////////////
    receiverEnabled = true;
//////////////////////////////
   attachInterrupt(CC1101_CCGDO0A, InterruptHandler, CHANGE);
}

void CC1101_CLASS::setCC1101Preset(CC1101_PRESET preset) {
    C1101preset = preset;
}

void CC1101_CLASS::disableReceiver()
{
    detachInterrupt((uint8_t)receiverGPIO);
    receiverEnabled = false;
    CC1101_is_initialized = false;
    ELECHOUSE_cc1101.setSidle();
    ELECHOUSE_cc1101.goSleep();
    ELECHOUSE_cc1101.SpiEnd();
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
            break;
        case FM95:
            CC1101_MODULATION = 0;
            CC1101_DRATE = 4.798;
            CC1101_RX_BW = 270;
            CC1101_DEVIATION = 9.521;
   //         CC1101_SYNC = 6;
            break;
        case FM15k:
            CC1101_MODULATION = 0;
            CC1101_DRATE = 3.794;
            CC1101_RX_BW = 135;
            CC1101_DEVIATION = 15.869;
    //        CC1101_SYNC = 7;
            break;
        case PAGER:
            CC1101_MODULATION = 0;
            CC1101_DRATE = 0.625;
            CC1101_RX_BW = 270;
            CC1101_DEVIATION = 5.157;
//            CC1101_SYNC = 6;
            break;
        case HND1:
            CC1101_MODULATION = 0;
            CC1101_DRATE = 15.357;
            CC1101_RX_BW = 270;
            CC1101_DEVIATION = 15.869;
 //           CC1101_SYNC = 6;
            break;
        case HND2:
            CC1101_MODULATION = 0;
            CC1101_DRATE = 15.357;
            CC1101_RX_BW = 67;
            CC1101_DEVIATION = 15.869;
 //           CC1101_SYNC = 7;
            break;
        default:
            CC1101_MODULATION = 2;
            CC1101_DRATE = 3.79372;
            CC1101_RX_BW = 650.00;
            CC1101_DEVIATION = 1.58;
  //          CC1101_SYNC = 2;
            break;
    }

    // ELECHOUSE_cc1101.setModulation(CC1101_MODULATION);
    // ELECHOUSE_cc1101.setDRate(CC1101_DRATE);
    // ELECHOUSE_cc1101.setRxBW(CC1101_RX_BW);
    // ELECHOUSE_cc1101.setDeviation(CC1101_DEVIATION);
    Serial.print("preset loaded");
}

bool CC1101_CLASS::CheckReceived()
{
    delay(1);
    if (samplecount >= minsample && micros() - lastTime > 100000)
    {
        receiverEnabled = false;
        return 1;
    }
    else
    {
        return 0;
    }
}

void CC1101_CLASS::signalanalyse(){
 #define signalstorage 10

  int signalanz=0;
  int timingdelay[signalstorage];
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

    smoothcount=0;
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

      ScreenManager& screenMgr = ScreenManager::getInstance();
    lv_obj_t * textareaRC = screenMgr.getTextArea();

    File outputFile;

    lv_textarea_set_text(textareaRC, "New RAW signal, Count: ");
    lv_textarea_add_text(textareaRC, String(smoothcount).c_str());
    lv_textarea_add_text(textareaRC,"\n");
    String rawString = "";

    for (int i = 0; i < smoothcount; i++) {
            rawString += (i > 0 ? (i % 2 == 1 ? " -" : " ") : "");
            rawString += samplesmooth[i];
        }

    lv_textarea_add_text(textareaRC, "Capture Complete | Sample: ");
    lv_textarea_add_text(textareaRC, rawString.c_str());





    FlipperSubFile subFile;
    CC1101_CLASS::disableReceiver();
    if (SDInit()) {
    if (!SD.exists("/recordedRF/" )) {
        SD.mkdir("/recordedRF/" );
    }  
        String filename = CC1101_CLASS::generateFilename(CC1101_MHZ, CC1101_MODULATION, CC1101_RX_BW);
        String fullPath = "/recordedRF/" + filename; 
        outputFile = SD.open(fullPath.c_str(), "w");
        if (outputFile) {
            std::vector<byte> customPresetData;
            if (C1101preset == CUSTOM) {
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
            subFile.generateRaw(outputFile, C1101preset, customPresetData, rawString, CC1101_MHZ);
            Serial.print(rawString);
            outputFile.close();
        } else {

        }
        SD.end();
        digitalWrite(CC1101_CS, LOW);
        digitalWrite(SDCARD_CS, HIGH);
        CC1101_CLASS::enableReceiver();     

    }
}

void CC1101_CLASS::sendRaw() {
    detachInterrupt(CC1101_CCGDO0A);
    disconnectSD();
    digitalWrite(SDCARD_CS, HIGH);
    CC1101_CLASS::initrRaw();
    Serial.print(F("\r\nReplaying RAW data from the buffer...\r\n"));

    Serial.print("Transmitting\n");
    Serial.print(smoothcount);
    Serial.print("\n----------------\n");
    for (int i = 1; i < smoothcount - 1; i += 2)
    {
        unsigned long highTime = max((unsigned long)(samplesmooth[i]), 0UL);
        unsigned long lowTime = max((unsigned long)(samplesmooth[i + 1]), 0UL);
        digitalWrite(CC1101_CCGDO0A, HIGH);
        delayMicroseconds(highTime);
        digitalWrite(CC1101_CCGDO0A, LOW);
        delayMicroseconds(lowTime);
    }
    Serial.print("Transmitted\n");
    delay(20);
    digitalWrite(CC1101_CCGDO0A, LOW); 

    Serial.print(F("\r\nReplaying RAW data complete.\r\n\r\n"));
}

void CC1101_CLASS::initrRaw() {
  Serial.print("Init CC1101 raw");

    if(!CC1101_is_initialized) {
        CC1101_CLASS::init();
        CC1101_is_initialized = true;
    }

    ELECHOUSE_cc1101.Init();                // must be set to initialize the cc1101!
    ELECHOUSE_cc1101.setGDO0(CC1101_CCGDO0A);         // set lib internal gdo pin (gdo0). Gdo2 not use for this example.
    ELECHOUSE_cc1101.setCCMode(0);          // set config for internal transmission mode. value 0 is for RAW recording/replaying
    ELECHOUSE_cc1101.setModulation(CC1101_MODULATION);      // set modulation mode. 0 = 2-FSK, 1 = GFSK, 2 = ASK/OOK, 3 = 4-FSK, 4 = MSK.
    ELECHOUSE_cc1101.setDeviation(CC1101_DEVIATION);   // Set the Frequency deviation in kHz. Value from 1.58 to 380.85. Default is 47.60 kHz.
    ELECHOUSE_cc1101.setDRate(CC1101_DRATE);
    ELECHOUSE_cc1101.setPktFormat(3);
    ELECHOUSE_cc1101.SetTx();
    pinMode(CC1101_CCGDO0A, OUTPUT);  
}
   
String CC1101_CLASS::generateFilename(float frequency, int modulation, float bandwidth)
{
    char filenameBuffer[32];

    sprintf(filenameBuffer, "%d_%s_%s.sub", static_cast<int>(frequency * 100), presetToString(C1101preset),
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

void CC1101_CLASS::sendSamples(int samples[], int samplesLength)
{
    disconnectSD();
    digitalWrite(SDCARD_CS, HIGH);
    CC1101_CLASS::initrRaw();

    pinMode(CC1101_CCGDO0A, OUTPUT);
    Serial.print(F("\r\nReplaying RAW data from the buffer...\r\n"));

    Serial.print("Transmitting\n");
    Serial.print(samplesLength);
    Serial.print("\n----------------\n");
    for (int i = 1; i < samplesLength - 1; i += 2)
    {
        unsigned long highTime = max((unsigned long)(samples[i]), 0UL);
        unsigned long lowTime = max((unsigned long)(samples[i + 1]), 0UL);
        digitalWrite(CC1101_CCGDO0A, HIGH);
        delayMicroseconds(highTime);
        digitalWrite(CC1101_CCGDO0A, LOW);
        delayMicroseconds(lowTime);
    }
    Serial.print("Transmitted\n");
    delay(20);
    digitalWrite(CC1101_CCGDO0A, LOW); 

    Serial.print(F("\r\nReplaying RAW data complete.\r\n\r\n"));
    ELECHOUSE_cc1101.setSidle(); 
    ELECHOUSE_cc1101.goSleep();  
    disableTransmit();
    SDInit();
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
    ELECHOUSE_cc1101.SetRx();
    pinMode(CC1101_CCGDO0A, OUTPUT);  

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
//;
}

void CC1101_CLASS::sendBrute(int type) {
    bruteIsRunning = true;
 //initializing library with custom pins selected
     #include "../RFBrute/came12bit.h"
    #include "../RFBrute/czech_bell_8bit.h"
    CC1101_CLASS::initrRaw();
    CC1101_CLASS::loadPreset();   
    // types:
    // 1 - czech bell (6-bit combinations)
    // 2 - came 12-bit combinations
    int count6 = 64;  
    int count12 = 4096;  
    int i = 0;
    int sc = 0;
    uint16_t sr = 0;
    
    switch (type) {
        case 1:
 
            while (i < count6)  
            {
            int times[26];
            times[1] = 420;
            int index = 1;
                int j = 0;
                while (j < 6) {
                if (((binary_combinations_czech_bell[i] >> j) & 1) == 1) {
                    times[index] = 420;
                    index++; 
                    times[index] = 840;
                    index++;
                } else {
                    times[index] = 840;
                    index++; 
                    times[index++] = 420;
                    index++; 
                }
                j++;                 
                }
                    times[index] = 840;
                    index++; 
                    times[index++] = 420;
                    times[index] = 840;
                    index++; 
                    times[index++] = 420;
                    times[index] = 420;
                    index++; 
                    times[index] = 840;
                    index++;
                    times[index] = 420;
                    index++; 
                    times[index] = 840;
                    index++;
                sc++;
                sr++;
                if(sc > 9) {
                    sc = 0;
                    bruteCounter = sr;
                }
                Serial.print(" \n");
                for (int i = 1; i < index - 1; i += 1)
                {
                    Serial.print(times[i]);
                    Serial.print(", ");

                }
                Serial.print(" \n");

                for (int i = 1; i < index - 1; i += 2)
                {
                    digitalWrite(CC1101_CCGDO0A, HIGH);
                    delayMicroseconds(times[i]);
                    digitalWrite(CC1101_CCGDO0A, LOW);
                    delayMicroseconds(times[i+1]);
                }
                delayMicroseconds(15000);  
                for (int i = 1; i < index - 1; i += 2)
                {
                    digitalWrite(CC1101_CCGDO0A, HIGH);
                    delayMicroseconds(times[i]);
                    digitalWrite(CC1101_CCGDO0A, LOW);
                    delayMicroseconds(times[i+1]);
                }
                delayMicroseconds(15000);                 
                i++;
                 }
            break;
        case 2:
            while (i < count12)  
            {
               // ELECHOUSE_cc1101.SendData((char*)binary_combinations_came_12bit[i], 12);
                i++;  
            }
            break;
        default:
            break;
    }

    delay(20);
    digitalWrite(CC1101_CCGDO0A, LOW);
    bruteIsRunning = false;
}

void CC1101_CLASS::enableRCSwitch()
{
    if (!CC1101_is_initialized) {
        CC1101_CLASS::init();
    }
    CC1101_CLASS::loadPreset();

   // ELECHOUSE_cc1101.Init();

    if (CC1101_MODULATION == 2)
    {
        ELECHOUSE_cc1101.setDcFilterOff(0);
    }

    if (CC1101_MODULATION == 0)
    {
        ELECHOUSE_cc1101.setDcFilterOff(1);
    }

     ELECHOUSE_cc1101.setDcFilterOff(1);
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
    
    pinMode(CC1101_CCGDO0A, INPUT);
//    receiverGPIO = digitalPinToInterrupt(CC1101_CCGDO0A);    

    mySwitch.enableReceive(CC1101_CCGDO0A); // Receiver on

}