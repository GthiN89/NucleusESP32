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


#define SAMPLE_SIZE 1024

#define BufferSize 2048

using namespace std;

byte bigrecordingbuffer[2048] = {0};
long data_to_send[2000];

int receiverGPIO;

int pulseLenght = 0;
bool recievedSubGhz;
 char frequency_buffer[10];
 char selected_str[32]; 
 bool ProtAnaRxEn;
 char filename_buffer[32];
 bool CC1101_TX = false;
 volatile unsigned long ReceivedValue = 0;
volatile unsigned int ReceivedBitlength = 0;
volatile unsigned int ReceivedDelay = 0;
volatile unsigned int ReceivedProtocol = 0;
long transmit_push[2000];
int transmissions = 1;
String transmit = "";


CC1101_PRESET C1101preset = AM650;
int CC1101_PKT_FORMAT = 0;

int CC1101_MODULATION = 2;

int CC1101_SYNC = 1;

int samplecount;
const int minsample = 30;
static unsigned long lastTime = 0;

int sample[SAMPLE_SIZE];

int error_toleranz = 200;

bool CC1101_is_initialized = false;
bool CC1101_recieve_is_running = false;
bool CC1101_transmit_is_running = false;
bool CC1101_isiddle = true;
bool CC1101_interup_attached = false;


String fullPath;   

RCSwitch mySwitch;

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
        if (samplecount == 1 && digitalRead(CC1101_CCGDO0A) != HIGH)
        {
            samplecount = 0;
        }
    }

    lastTime = time;

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

bool CC1101_CLASS::init()
{
    ELECHOUSE_cc1101.setSpiPin(CC1101_SCLK, CC1101_MISO, CC1101_MOSI, CC1101_CS);
    ELECHOUSE_cc1101.Init();

    if (ELECHOUSE_cc1101.getCC1101())
    {
        ELECHOUSE_cc1101.setSidle();
        CC1101_isiddle = true;
        return true;
    }
    else
    {
        ELECHOUSE_cc1101.setSidle();
        CC1101_isiddle = true;
        return false;
    }
    ELECHOUSE_cc1101.setGDO(CCGDO0A, CCGDO2A);
}

void CC1101_CLASS::showResultRecPlay()
{
    ScreenManager& screenMgr = ScreenManager::getInstance();

    lv_obj_t * text_area = screenMgr.getTextArea();
    

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
    detachInterrupt((uint8_t)receiverGPIO);
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
            CC1101_MODULATION = 2;
            CC1101_DRATE = 3.79372;
            CC1101_RX_BW = 270.833333;
            CC1101_DEVIATION = 1.58;
            break;
    }
}

bool CC1101_CLASS::CheckReceived()
{
   delay(1);
  if (samplecount >= minsample && micros()-lastTime >100000){
    return 1;
  }else{
    return 0;
  }
}

void CC1101_CLASS::signalanalyse(){
      ScreenManager& screenMgr = ScreenManager::getInstance();
    lv_obj_t * textareaRC = screenMgr.getTextArea();

    lv_textarea_set_text(textareaRC, "New RAW signal, Count: ");
    lv_textarea_add_text(textareaRC, String(samplecount).c_str());
    lv_textarea_add_text(textareaRC,"\n");
    String rawString = "";

    for (int i = 1; i < samplecount; i++)
    {
        rawString += sample[i];
        rawString += ",";
    }

    lv_textarea_add_text(textareaRC, "Capture Complete | Sample: ");
    lv_textarea_add_text(textareaRC, rawString.c_str());
  return;
}

void CC1101_CLASS::sendRaw() {
    detachInterrupt(CC1101_CCGDO0A);
    ScreenManager& screenMgr = ScreenManager::getInstance();
    lv_obj_t * textarea = screenMgr.getTextArea();
    CC1101_CLASS::initrRaw();
    ELECHOUSE_cc1101.setCCMode(0); 
    ELECHOUSE_cc1101.setPktFormat(3);
    ELECHOUSE_cc1101.SetTx();

    pinMode(CC1101_CCGDO0A, OUTPUT);
    lv_textarea_set_text(textarea, "Replaying RAW data from the buffer...\n");
    Serial.print(F("\r\nReplaying RAW data from the buffer...\r\n"));

    Serial.print("Transmitting\n");
    for (int i = 1; i < samplecount - 1; i += 2)
    {
        // Casting to unsigned long to resolve type mismatch with 0UL
        unsigned long highTime = max((unsigned long)(sample[i] - 100), 0UL);
        unsigned long lowTime = max((unsigned long)(sample[i + 1] - 100), 0UL);

        digitalWrite(CC1101_CCGDO0A, HIGH);
        delayMicroseconds(highTime);
        digitalWrite(CC1101_CCGDO0A, LOW);
        delayMicroseconds(lowTime);
    }
    Serial.print("Transmitted\n");
    delay(20);
    digitalWrite(CC1101_CCGDO0A, LOW); 

    Serial.print(F("\r\nReplaying RAW data complete.\r\n\r\n"));
    lv_textarea_set_text(textarea, "Replaying RAW data complete.\n");
    ELECHOUSE_cc1101.setSidle();  // Set to idle state
    ELECHOUSE_cc1101.goSleep();   // Put CC1101 into sleep mode
    disableTransmit();
}


bool CC1101_CLASS::getPulseLenghtLoop() {
    CC1101_CLASS::signalanalyse();
    ScreenManager& screenMgr = ScreenManager::getInstance();
    return true;
}


void CC1101_CLASS::initrRaw() {
  Serial.print("Init CC1101 raw");
   // initializing library with custom pins selected
     ELECHOUSE_cc1101.setSpiPin(CC1101_SCLK, CC1101_MISO, CC1101_MOSI, CC1101_CS);
     ELECHOUSE_cc1101.setGDO(CC1101_CCGDO0A, CC1101_CCGDO2A);

    // Main part to tune CC1101 with proper frequency, modulation and encoding    
    ELECHOUSE_cc1101.Init();                // must be set to initialize the cc1101!
    ELECHOUSE_cc1101.setGDO0(CC1101_CCGDO0A);         // set lib internal gdo pin (gdo0). Gdo2 not use for this example.
    ELECHOUSE_cc1101.setCCMode(1);          // set config for internal transmission mode. value 0 is for RAW recording/replaying
    ELECHOUSE_cc1101.setModulation(2);      // set modulation mode. 0 = 2-FSK, 1 = GFSK, 2 = ASK/OOK, 3 = 4-FSK, 4 = MSK.
    ELECHOUSE_cc1101.setMHZ(433.92);        // Here you can set your basic frequency. The lib calculates the frequency automatically (default = 433.92).The cc1101 can: 300-348 MHZ, 387-464MHZ and 779-928MHZ. Read More info from datasheet.
    ELECHOUSE_cc1101.setDeviation(47.60);   // Set the Frequency deviation in kHz. Value from 1.58 to 380.85. Default is 47.60 kHz.
    ELECHOUSE_cc1101.setChannel(0);         // Set the Channelnumber from 0 to 255. Default is cahnnel 0.
    ELECHOUSE_cc1101.setChsp(199.95);       // The channel spacing is multiplied by the channel number CHAN and added to the base frequency in kHz. Value from 25.39 to 405.45. Default is 199.95 kHz.
    ELECHOUSE_cc1101.setRxBW(812.50);       // Set the Receive Bandwidth in kHz. Value from 58.03 to 812.50. Default is 812.50 kHz.
    ELECHOUSE_cc1101.setDRate(9.6);         // Set the Data Rate in kBaud. Value from 0.02 to 1621.83. Default is 99.97 kBaud!
    ELECHOUSE_cc1101.setPA(10);             // Set TxPower. The following settings are possible depending on the frequency band.  (-30  -20  -15  -10  -6    0    5    7    10   11   12) Default is max!
    ELECHOUSE_cc1101.setSyncMode(2);        // Combined sync-word qualifier mode. 0 = No preamble/sync. 1 = 16 sync word bits detected. 2 = 16/16 sync word bits detected. 3 = 30/32 sync word bits detected. 4 = No preamble/sync, carrier-sense above threshold. 5 = 15/16 + carrier-sense above threshold. 6 = 16/16 + carrier-sense above threshold. 7 = 30/32 + carrier-sense above threshold.
    ELECHOUSE_cc1101.setSyncWord(211, 145); // Set sync word. Must be the same for the transmitter and receiver. Default is 211,145 (Syncword high, Syncword low)
    ELECHOUSE_cc1101.setAdrChk(0);          // Controls address check configuration of received packages. 0 = No address check. 1 = Address check, no broadcast. 2 = Address check and 0 (0x00) broadcast. 3 = Address check and 0 (0x00) and 255 (0xFF) broadcast.
    ELECHOUSE_cc1101.setAddr(0);            // Address used for packet filtration. Optional broadcast addresses are 0 (0x00) and 255 (0xFF).
    ELECHOUSE_cc1101.setWhiteData(0);       // Turn data whitening on / off. 0 = Whitening off. 1 = Whitening on.
    ELECHOUSE_cc1101.setPktFormat(0);       // Format of RX and TX data. 0 = Normal mode, use FIFOs for RX and TX. 1 = Synchronous serial mode, Data in on GDO0 and data out on either of the GDOx pins. 2 = Random TX mode; sends random data using PN9 generator. Used for test. Works as normal mode, setting 0 (00), in RX. 3 = Asynchronous serial mode, Data in on GDO0 and data out on either of the GDOx pins.
    ELECHOUSE_cc1101.setLengthConfig(1);    // 0 = Fixed packet length mode. 1 = Variable packet length mode. 2 = Infinite packet length mode. 3 = Reserved
    ELECHOUSE_cc1101.setPacketLength(0);    // Indicates the packet length when fixed packet length mode is enabled. If variable packet length mode is used, this value indicates the maximum packet length allowed.
    ELECHOUSE_cc1101.setCrc(0);             // 1 = CRC calculation in TX and CRC check in RX enabled. 0 = CRC disabled for TX and RX.
    ELECHOUSE_cc1101.setCRC_AF(0);          // Enable automatic flush of RX FIFO when CRC is not OK. This requires that only one packet is in the RXIFIFO and that packet length is limited to the RX FIFO size.
    ELECHOUSE_cc1101.setDcFilterOff(0);     // Disable digital DC blocking filter before demodulator. Only for data rates ≤ 250 kBaud The recommended IF frequency changes when the DC blocking is disabled. 1 = Disable (current optimized). 0 = Enable (better sensitivity).
    ELECHOUSE_cc1101.setManchester(0);      // Enables Manchester encoding/decoding. 0 = Disable. 1 = Enable.
    ELECHOUSE_cc1101.setFEC(0);             // Enable Forward Error Correction (FEC) with interleaving for packet payload (Only supported for fixed packet length mode. 0 = Disable. 1 = Enable.
    ELECHOUSE_cc1101.setPRE(0);             // Sets the minimum number of preamble bytes to be transmitted. Values: 0 : 2, 1 : 3, 2 : 4, 3 : 6, 4 : 8, 5 : 12, 6 : 16, 7 : 24
    ELECHOUSE_cc1101.setPQT(0);             // Preamble quality estimator threshold. The preamble quality estimator increases an internal counter by one each time a bit is received that is different from the previous bit, and decreases the counter by 8 each time a bit is received that is the same as the last bit. A threshold of 4∙PQT for this counter is used to gate sync word detection. When PQT=0 a sync word is always accepted.
}

bool CC1101_CLASS::captureLoop()
{
        if (pulseLenght <= 0) {
    Serial.println("Invalid pulse length. Cannot proceed with recording.");
    return false; // or handle the error appropriately
}
Serial.print(to_string(pulseLenght).c_str());
        // detachInterrupt(CC1101_CCGDO0A);
        // detachInterrupt(CC1101_CCGDO2A);
      //  byte textbuffer[128];
        ScreenManager& screenMgr = ScreenManager::getInstance();
        lv_obj_t * text_area = screenMgr.getTextAreaRCSwitchMethod();
        // take interval period for samplink
        // setting = atoi(cmdline);
        // if (setting>0)
        // {
        // setup async mode on CC1101 with GDO0 pin processing

       // int setting2;
        ELECHOUSE_cc1101.setCCMode(0); 
        ELECHOUSE_cc1101.setPktFormat(3);
        ELECHOUSE_cc1101.SetRx();


        //start recording to the buffer with bitbanging of GDO0 pin state
        Serial.print("Waiting for radio signal to start RAW recording...\n");
        lv_textarea_set_text(text_area, "Waiting for radio signal...\n");

        pinMode(CC1101_CCGDO0A, INPUT);

        // this is only for ESP32 boards because they are getting some noise on the beginning
        //setting2 = digitalRead(CC1101_CCGDO0A);
        delayMicroseconds(1000);  

        int setting2;
        // waiting for some data first or serial port signal
        //while (!Serial.available() ||  (digitalRead(gdo0) == LOW) ); 
        while ( digitalRead(CC1101_CCGDO0A) == LOW );

        
        //start recording to the buffer with bitbanging of GDO0 pin state
        Serial.print("Starting RAW recording to the buffer...\n");
        lv_textarea_set_text(text_area, "Starting RAW recording...\n");
        for (int i=0; i<BufferSize ; i++)  
           { 
             byte receivedbyte = 0;
             for(int j=7; j > -1; j--)  // 8 bits in a byte
               {
                 bitWrite(receivedbyte, j, digitalRead(CC1101_CCGDO0A)); // Capture GDO0 state into the byte
                 delayMicroseconds(pulseLenght);                   // delay for selected sampling interval
               }; 
                 // store the output into recording buffer
             bigrecordingbuffer[i] = receivedbyte;
           }
        Serial.print("Recording RAW data complete.\n");
        lv_textarea_set_text(text_area, "Recording complete.\n");
     
        C1101CurrentState = STATE_IDLE;
        ELECHOUSE_cc1101.setSidle();  // Set to idle state
        ELECHOUSE_cc1101.goSleep();   // Put CC1101 into sleep mode
    
    // Optionally disable chip select (CS) to fully power down the CC1101
        digitalWrite(CC1101_CS, HIGH);
        Serial.print(F("\r\nRecorded RAW data as bit stream:\r\n"));


    //     ELECHOUSE_cc1101.setSidle();  // Set to idle state
    // ELECHOUSE_cc1101.goSleep();   // Put CC1101 into sleep mode
    
    // // Optionally disable chip select (CS) to fully power down the CC1101
    // digitalWrite(CC1101_CS, HIGH); 
            ELECHOUSE_cc1101.setCCMode(1); 
        ELECHOUSE_cc1101.setPktFormat(0);
        ELECHOUSE_cc1101.SetRx();
        
        return true;
}
  
   //     else { Serial.print(F("Wrong parameters.\r\n")); };

   // 
void CC1101_CLASS::asciitohex(byte *ascii_ptr, byte *hex_ptr,int len)
{
    byte i,j,k;
    for(i = 0; i < len; i++)
    {
      // high byte first
      j = ascii_ptr[i] / 16;
      if (j>9) 
         { k = j - 10 + 65; }
      else 
         { k = j + 48; }
      hex_ptr[2*i] = k ;
      // low byte second
      j = ascii_ptr[i] % 16;
      if (j>9) 
         { k = j - 10 + 65; }
      else
         { k = j + 48; }
      hex_ptr[(2*i)+1] = k ; 
    };
    hex_ptr[(2*i)+2] = '\0' ; 
}
   
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
  Serial.print("\n----------------");
  Serial.print(to_string(pulseLenght).c_str());
  Serial.print("----------------\n");

        ScreenManager& screenMgr = ScreenManager::getInstance();
        lv_obj_t * text_area = screenMgr.getTextAreaRCSwitchMethod();
        // take interval period for sampling
        // setup async mode on CC1101 and go into TX mode
        // with GDO0 pin
        if (pulseLenght <= 0) {
    Serial.println("Invalid pulse length. Cannot proceed with transmission.");
    return false;
}
        CC1101_CLASS::initrRaw();
        ELECHOUSE_cc1101.setCCMode(0); 
        ELECHOUSE_cc1101.setPktFormat(3);
        ELECHOUSE_cc1101.SetTx();
        ELECHOUSE_cc1101.setPA(12);
        pinMode(CC1101_CCGDO0A, OUTPUT);
        //start replaying GDO0 bit state from data in the buffer with bitbanging 
        Serial.print(F("\r\nReplaying RAW data from the buffer...\r\n"));
        lv_textarea_set_text(text_area, "Replaying RAW data from the buffer...\n");
       
        

        
        for (int i=1; i<BufferSize ; i++)  
           { 
             byte receivedbyte = bigrecordingbuffer[i];
             for(int j=7; j > -1; j--)  // 8 bits in a byte
               {
                 digitalWrite(CC1101_CCGDO0A, bitRead(receivedbyte, j)); // Set GDO0 according to recorded byte
                 delayMicroseconds(pulseLenght);                      // delay for selected sampling interval
               }; 
           }

        
        Serial.print(F("\r\nReplaying RAW data complete.\r\n\r\n"));
        lv_textarea_set_text(text_area, "Replaying RAW data complete.\n");
        ELECHOUSE_cc1101.setSidle(); 
            ELECHOUSE_cc1101.setSidle();  // Set to idle state
    ELECHOUSE_cc1101.goSleep();   // Put CC1101 into sleep mode
    
    // Optionally disable chip select (CS) to fully power down the CC1101
    digitalWrite(CC1101_CS, HIGH); 
        // setting normal pkt format again
    
        // pinMode(gdo0pin, INPUT);   
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
    ELECHOUSE_cc1101.SetRx();
    pinMode(CCGDO2A, OUTPUT);  

    mySwitch.enableTransmit(CC1101_CCGDO2A);
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

bool  CC1101_CLASS::initCC1101() {
    digitalWrite(5, HIGH);
     SPI.begin();
    Serial.println("Initializing CC1101");
    ELECHOUSE_cc1101.setSpiPin(CC1101_SCLK, CC1101_MISO, CC1101_MOSI, CC1101_CS);
    ELECHOUSE_cc1101.Init();
    ELECHOUSE_cc1101.setGDO(CCGDO0A, CCGDO2A);
    ELECHOUSE_cc1101.setMHZ(CC1101_MHZ);
    ELECHOUSE_cc1101.setPA(12);
    if (CC1101_TX) {
        ELECHOUSE_cc1101.SetTx();
        pinMode(CCGDO0A, OUTPUT);  
    } else {
        ELECHOUSE_cc1101.SetRx();
        pinMode(CCGDO2A, INPUT);  
    }
    ELECHOUSE_cc1101.setModulation(CC1101_MODULATION);
    ELECHOUSE_cc1101.setDRate(CC1101_DRATE);
    ELECHOUSE_cc1101.setRxBW(CC1101_RX_BW);
    ELECHOUSE_cc1101.setPktFormat(CC1101_PKT_FORMAT);

    if (!ELECHOUSE_cc1101.getCC1101()) {
        Serial.println("CC1101 Connection Error");
        return false;
    } else {
        Serial.println("CC1101 Initialized successfully");
        return true;
    }
}



void CC1101_CLASS::initRCSwitch()
{
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
    
    pinMode(CC1101_CCGDO0A, INPUT);
    receiverGPIO = digitalPinToInterrupt(CC1101_CCGDO0A);    
    ELECHOUSE_cc1101.SetRx();
/////////////////////////////
    receiverEnabled = true;
//////////////////////////////
       mySwitch.enableReceive(CC1101_CCGDO0A); // Receiver on
}



void CC1101_CLASS::disableRCSwitch() 
{
    mySwitch.disableReceive();
    ELECHOUSE_cc1101.setSidle();
    ELECHOUSE_cc1101.goSleep();
}




bool CC1101_CLASS::RCmethodResult()
{

    ScreenManager& screenMgr = ScreenManager::getInstance();
    lv_obj_t * text_are_ = screenMgr.getTextArea();
 if (mySwitch.available() && !recievedSubGhz) {
        unsigned long receivedValue = mySwitch.getReceivedValue();
        unsigned long receivedBitLength = mySwitch.getReceivedBitlength();
        unsigned long receivedProtocol = mySwitch.getReceivedProtocol();
        recievedSubGhz = true;

        if (text_are_) {
            char buffer[32];
            Serial.println("Received Value:");
            Serial.println(receivedValue);
            lv_textarea_add_text(text_are_, "Value: ");
            sprintf(buffer, "%lu", receivedValue);
            lv_textarea_add_text(text_are_, buffer);
            lv_textarea_add_text(text_are_, "\n");

            int receivedBitLength = mySwitch.getReceivedBitlength();
            Serial.println("Received Bit Length:");
            Serial.println(receivedBitLength);
            lv_textarea_add_text(text_are_, "Length: ");
            sprintf(buffer, "%d", receivedBitLength);
            lv_textarea_add_text(text_are_, buffer);
            lv_textarea_add_text(text_are_, "\n");

            int receivedProtocol = mySwitch.getReceivedProtocol();
            Serial.println("Received Protocol:");
            Serial.println(receivedProtocol);
            lv_textarea_add_text(text_are_, "Protocol: ");
            sprintf(buffer, "%d", receivedProtocol);
            lv_textarea_add_text(text_are_, buffer);
            lv_textarea_add_text(text_are_, "\n");
            return true;
        }
        
    }return false;
}

void CC1101_CLASS::ResetRCSwitch()
{
    mySwitch.resetAvailable();
}

