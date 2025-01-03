#include "CC1101.h"
#include "Arduino.h"
#include "ELECHOUSE_CC1101_SRC_DRV.h"
#include "GUI/ScreenManager.h"
#include <sstream>
#include <ctime>
#include <vector>
#include <string> 
#include "GUI/events.h"
#include "SPI.h"
#include "modules/ETC/SDcard.h"
#include "ESPiLight.h"
#include <esp_timer.h>
#include <esp_attr.h>
#include <driver/gpio.h>
#include <esp_intr_alloc.h>
#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>



float start_freq = 433;
float stop_freq = 434;
float freq = start_freq;
long compare_freq;
float mark_freq;
int rssi;
int mark_rssi = -100;

     String protDecode[]={
"Unknown",
"01 Princeton, PT-2240",
"02 AT-Motor?",
"03",
"04",
"05",
"06 HT6P20B",
"07 HS2303-PT, i. e. used in AUKEY Remote",
"08 Came 12bit, HT12E",
"09 Nice_Flo 12bit",
"10 V2 phoenix",
"11 Nice_FloR-S 52bit",
"12 Keeloq 64/66 falseok",
"13 test CFM",
"14 test StarLine",
"15",
"16 Einhell",
"17 InterTechno PAR-1000",
"18 Intertechno ITT-1500",
"19 Murcury",
"20 AC114",
"21 DC250",
"22 Mandolyn/Lidl TR-502MSV/RC-402/RC-402DX",
"23 Lidl TR-502MSV/RC-402 - Flavien",
"24 Lidl TR-502MSV/RC701",
"25 NEC",
"26 Arlec RC210",
"27 Zap, FHT-7901",
"28", // github.com/sui77/rc-switch/pull/115
"29 NEXA",
"30 Anima",
"31 Mertik Maxitrol G6R-H4T1",
"32", //github.com/sui77/rc-switch/pull/277
"33 Dooya Control DC2708L",
"34 DIGOO SD10 ", //so as to use this protocol RCSWITCH_SEPARATION_LIMIT must be set to 2600
"35 Dooya 5-Channel blinds remote DC1603",
"36 DC2700AC", //Dooya remote DC2700AC for Dooya DT82TV curtains motor
"37 DEWENWILS Power Strip",
"38 Nexus weather, 36 bit",
"39 Louvolite with premable"
};

SDcard& SD_RF = SDcard::getInstance();

ESPiLight espilight(-1); 


ScreenManager& screenMgr1 = ScreenManager::getInstance();

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;


using namespace std;
int receiverGPIO;

String rawString;

static unsigned long lastTime = 0;

uint16_t  sample[SAMPLE_SIZE];

int error_toleranz = 200;

uint8_t samplecount = 0;

bool CC1101_is_initialized = false;
bool CC1101_recieve_is_running = false;
bool CC1101_transmit_is_running = false;
bool CC1101_isiddle = true;
bool CC1101_interup_attached = false;
uint32_t actualFreq;

int CC1101_MODULATION = 2;

int smoothcount;
unsigned long samplesmooth[SAMPLE_SIZE];

String fullPath;   

RCSwitch mySwitch;

 float strongestASKFreqs[4] = {0};  // Store the four strongest ASK/OOK frequencies
    int strongestASKRSSI[4] = {-200}; // Initialize with very low RSSI values
    float strongestFSKFreqs[2] = {0}; // Store the two strongest FSK frequencies (F0 and F1)
    int strongestFSKRSSI[2] = {-200}; // Initialize FSK RSSI values

RCSwitch CC1101_CLASS::getRCSwitch() {
 return mySwitch;
}


#define CONFIG_CPU_FREQ_MHZ 240
#define CYCLES_PER_MICROSECOND CONFIG_CPU_FREQ_MHZ


static inline uint32_t IRAM_ATTR getCycleCount() {
    uint32_t ccount;
    __asm__ __volatile__("rsr %0,ccount":"=a" (ccount));
    return ccount;
}

// microseconds to CPU cycles
static inline uint32_t IRAM_ATTR usToTicks(uint32_t us) {
    return us * CYCLES_PER_MICROSECOND;
}

//CPU cycles to microseconds
static inline uint32_t IRAM_ATTR ticksToUs(uint32_t ticks) {
    return ticks / CYCLES_PER_MICROSECOND;
}

void IRAM_ATTR InterruptHandler() {    
    if (!receiverEnabled) return;

    static volatile uint64_t DRAM_ATTR lastTime = 0;
    static volatile bool DRAM_ATTR signalStarted = false;
    
    const uint64_t time = xthal_get_ccount();
    const uint64_t duration = time - lastTime;
    lastTime = time;


    constexpr uint64_t TIMEOUT_TICKS = 1000000 * 240; // 1000ms timeout
    constexpr uint64_t MIN_DURATION_TICKS = 50 * 240; // 50µs minimum duration


    if (duration > TIMEOUT_TICKS) {
        if (!signalStarted) {
            samplecount = 0;
            signalStarted = false;
        }
        return;
    }


    if (duration < MIN_DURATION_TICKS) return;


    if (!signalStarted && duration > MIN_DURATION_TICKS) {
        signalStarted = true;
    }


    if (samplecount >= SAMPLE_SIZE) {
        signalStarted = false;
        return;
    }


    sample[samplecount++] = duration / 240;


    if (signalStarted && duration > TIMEOUT_TICKS/2) {
        signalStarted = false;
    }
}



bool CC1101_CLASS::init() {
    ELECHOUSE_cc1101.setSpiPin(CC1101_SCLK, CC1101_MISO, CC1101_MOSI, CC1101_CS);
    ELECHOUSE_cc1101.Init();

    if (ELECHOUSE_cc1101.getCC1101()) {
        ELECHOUSE_cc1101.SpiWriteReg(CC1101_AGCCTRL2, 0x07);  // Maximum LNA gain
        ELECHOUSE_cc1101.SpiWriteReg(CC1101_AGCCTRL1, 0x00);  // Fastest AGC
        ELECHOUSE_cc1101.SpiWriteReg(CC1101_AGCCTRL0, 0x91);  // Normal AGC, 16 samples
        ELECHOUSE_cc1101.SpiWriteReg(CC1101_FREND1, 0xB6);    // RX frontend configuration
        ELECHOUSE_cc1101.SpiWriteReg(CC1101_FREND0, 0x10);    // TX frontend configuration
        
        ELECHOUSE_cc1101.setGDO(CCGDO0A, CCGDO2A);
        ELECHOUSE_cc1101.setSidle();
        CC1101_is_initialized = true;
        return true;
    }
    return false;
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


void CC1101_CLASS::enableReceiver() {
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    portENTER_CRITICAL(&mux);

    if (!CC1101_is_initialized) {
        CC1101_CLASS::init();
    }
    CC1101_CLASS::loadPreset();

    // Clear sample buffer with memset
    memset(sample, 0, sizeof(uint16_t) * SAMPLE_SIZE);
    samplecount = 0;

    // Optimize CC1101 settings for longer signals
    ELECHOUSE_cc1101.setSidle();
    
    // Set wider bandwidth and disable filters
    ELECHOUSE_cc1101.setRxBW(CC1101_RX_BW);  // Maximum bandwidth
    ELECHOUSE_cc1101.setDcFilterOff(1); // Disable DC blocking
    
    // Disable packet handling
    ELECHOUSE_cc1101.setSyncMode(0);    // No sync word
    ELECHOUSE_cc1101.setPktFormat(3);   // Async serial mode
    
    // Configure for maximum sensitivity
    ELECHOUSE_cc1101.SpiWriteReg(0x18, 0x50); // AGC settings
    ELECHOUSE_cc1101.SpiWriteReg(0x1B, 0x07); // High sensitivity
    ELECHOUSE_cc1101.SpiWriteReg(0x1D, 0x91); // Frequency offset compensation

    // Standard configuration
    ELECHOUSE_cc1101.setModulation(CC1101_MODULATION);
    ELECHOUSE_cc1101.setMHZ(CC1101_MHZ);
    ELECHOUSE_cc1101.setDeviation(CC1101_DEVIATION);
    ELECHOUSE_cc1101.setDRate(CC1101_DRATE);

    // Configure GPIO for interrupt
    pinMode(CC1101_CCGDO2A, INPUT);
    receiverGPIO = digitalPinToInterrupt(CC1101_CCGDO2A);    

    // Flush FIFOs and calibrate
    ELECHOUSE_cc1101.SpiStrobe(0x3A);    // Flush RX
    ELECHOUSE_cc1101.SpiStrobe(0x3B);    // Flush TX
    ELECHOUSE_cc1101.SpiStrobe(0x33);    // Calibrate

    // Enter RX mode
    ELECHOUSE_cc1101.SetRx();
    receiverEnabled = true;

    // Attach interrupt with high priority
    esp_intr_alloc(ETS_GPIO_INTR_SOURCE,
                   ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_LEVEL1,
                   (intr_handler_t)InterruptHandler,
                   NULL,
                   NULL);
    attachInterrupt(CC1101_CCGDO2A, InterruptHandler, CHANGE);

    portEXIT_CRITICAL(&mux);
}


void CC1101_CLASS::setCC1101Preset(CC1101_PRESET preset) {
    C1101preset = preset;
}



void CC1101_CLASS::disableReceiver()
{
    detachInterrupt((uint8_t)receiverGPIO);
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
        CC1101_SYNC = 6;
        break;
    case FM15k:
        CC1101_MODULATION = 0;
        CC1101_DRATE = 3.794;
        CC1101_RX_BW = 135;
        CC1101_DEVIATION = 15.869;
        CC1101_SYNC = 7;
        break;
        case FSK12k:
        CC1101_MODULATION = 0;       // Modulation derived from preset (2-FSK)
        CC1101_DRATE = 12.69;        // Derived from preset name (12.69kHz deviation)
        CC1101_RX_BW = 200;          // Receiver bandwidth (200kHz as per preset name)
        CC1101_DEVIATION = 12.69;    // Derived from deviation value in preset name
        // CC1101_SYNC = 0x47;          // Sync word from `Custom_preset_data` (byte 3)
        // CC1101_CONFIG = {            // Full configuration extracted from `Custom_preset_data`
        //     0x02, 0x0D, 0x03, 0x47, 0x08, 0x32, 0x0B, 0x06, 0x15, 0x30,
        //     0x14, 0x00, 0x13, 0x00, 0x12, 0x00, 0x11, 0x32, 0x10, 0xA7,
        //     0x18, 0x18, 0x19, 0x1D, 0x1D, 0x92, 0x1C, 0x00, 0x1B, 0x04,
        //     0x20, 0xFB, 0x22, 0x17, 0x21, 0xB6, 0x00, 0x00, 0x00, 0x12,
        //     0x0E, 0x34, 0x60, 0xC5, 0xC1, 0xC0
        // };
        break;
    
    case FSK25k:
        CC1101_MODULATION = 0;       // Modulation derived from preset (2-FSK)
        CC1101_DRATE = 25.39;        // Derived from preset name (25.39kHz deviation)
        CC1101_RX_BW = 200;          // Receiver bandwidth (200kHz as per preset name)
        CC1101_DEVIATION = 25.39;    // Derived from deviation value in preset name
        CC1101_SYNC = 0x47;          // Sync word from `Custom_preset_data` (byte 3)
        // CC1101_CONFIG = {            // Full configuration extracted from `Custom_preset_data`
        //     0x02, 0x0D, 0x03, 0x47, 0x08, 0x32, 0x0B, 0x06, 0x15, 0x40,
        //     0x14, 0x00, 0x13, 0x00, 0x12, 0x00, 0x11, 0x32, 0x10, 0xA7,
        //     0x18, 0x18, 0x19, 0x1D, 0x1D, 0x92, 0x1C, 0x00, 0x1B, 0x04,
        //     0x20, 0xFB, 0x22, 0x17, 0x21, 0xB6, 0x00, 0x00, 0x00, 0x12,
        //     0x0E, 0x34, 0x60, 0xC5, 0xC1, 0xC0
        // };
        break;
    
    case FSK31k:
        CC1101_MODULATION = 0;       // Modulation derived from preset (2-FSK)
        CC1101_DRATE = 31.73;        // Derived from preset name (31.73kHz deviation)
        CC1101_RX_BW = 200;          // Receiver bandwidth (200kHz as per preset name)
        CC1101_DEVIATION = 31.73;    // Derived from deviation value in preset name
        // CC1101_SYNC = 0x47;          // Sync word from `Custom_preset_data` (byte 3)
        // CC1101_CONFIG = {            // Full configuration extracted from `Custom_preset_data`
        //     0x02, 0x0D, 0x03, 0x47, 0x08, 0x32, 0x0B, 0x06, 0x15, 0x42,
        //     0x14, 0x00, 0x13, 0x00, 0x12, 0x00, 0x11, 0x32, 0x10, 0xA7,
        //     0x18, 0x18, 0x19, 0x1D, 0x1D, 0x92, 0x1C, 0x00, 0x1B, 0x04,
        //     0x20, 0xFB, 0x22, 0x17, 0x21, 0xB6, 0x00, 0x00, 0x00, 0x12,
        //     0x0E, 0x34, 0x60, 0xC5, 0xC1, 0xC0
        // };
        break;
    case PAGER:
        CC1101_MODULATION = 0;
        CC1101_DRATE = 0.625;
        CC1101_RX_BW = 270;
        CC1101_DEVIATION = 5.157;
        CC1101_SYNC = 6;
        break;
    case HND1:
        CC1101_MODULATION = 0;
        CC1101_DRATE = 37.04;
        CC1101_RX_BW = 250;
        CC1101_DEVIATION = 30;
        CC1101_SYNC = 6;
        break;
    case HND2:
        CC1101_MODULATION = 0;
        CC1101_DRATE = 15.357;
        CC1101_RX_BW = 67;
        CC1101_DEVIATION = 15.869;
        CC1101_SYNC = 7;
        break;
    default:
        Serial.println(CC1101_MODULATION);
        break;
    }
    Serial.print("preset loaded");
}

bool CC1101_CLASS::CheckReceived() {
    static unsigned long signalStartTime = 0;
    static const unsigned long MIN_SIGNAL_LENGTH = 1000; // 1ms minimum
    static const unsigned long SIGNAL_TIMEOUT = 250000;  // 250ms timeout

    if (samplecount == 0) {
        signalStartTime = micros();
        return false;
    }

    unsigned long currentTime = micros();
    unsigned long signalDuration = currentTime - signalStartTime;

    // Signal is complete if:
    // 1. We have minimum number of samples
    // 2. AND either timeout occurred or buffer is full
    if (samplecount >= 24 && 
        (signalDuration > MIN_SIGNAL_LENGTH) &&
        (signalDuration > SIGNAL_TIMEOUT || samplecount >= SAMPLE_SIZE - 1)) {
        
        receiverEnabled = false;
        return true;
    }

    return false;
}

void CC1101_CLASS::fskAnalyze() {
    Serial.println("ana run");         

    while (true) { 

        if (CC1101_MODULATION == 2) {
            freq = start_freq;

            while (freq <= stop_freq) {
                ELECHOUSE_cc1101.setMHZ(freq);
                int rssi = ELECHOUSE_cc1101.getRssi();

                if (rssi > -80) { 
 
                    for (int i = 0; i < 4; i++) {
                        if (rssi > strongestASKRSSI[i]) {
      
                            for (int j = 3; j > i; j--) {
                                strongestASKRSSI[j] = strongestASKRSSI[j - 1];
                                strongestASKFreqs[j] = strongestASKFreqs[j - 1];
                            }

                            strongestASKRSSI[i] = rssi;
                            strongestASKFreqs[i] = freq;


                            Serial.println(String("New ASK Frequency: ") + strongestASKFreqs[i] +
                                           " MHz | RSSI: " + strongestASKRSSI[i]);
         
                            break;
                        }
                    }
                }

                freq += 0.10; 
            }
        } else if (CC1101_MODULATION == 0) {
            freq = start_freq;

            while (freq <= stop_freq) {
                ELECHOUSE_cc1101.setMHZ(freq);
                int rssi = ELECHOUSE_cc1101.getRssi();

                if (rssi > -80) { 
                    if (rssi > strongestFSKRSSI[0]) {

                        strongestFSKRSSI[1] = strongestFSKRSSI[0];
                        strongestFSKFreqs[1] = strongestFSKFreqs[0];

                        strongestFSKRSSI[0] = rssi;
                        strongestFSKFreqs[0] = freq;


                        Serial.println(String("New FSK Frequencies: F0 = ") + strongestFSKFreqs[0] +
                                       " MHz | RSSI: " + strongestFSKRSSI[0] +
                                       ", F1 = " + strongestFSKFreqs[1] +
                                       " MHz | RSSI: " + strongestFSKRSSI[1]);
                    } else if (rssi > strongestFSKRSSI[1]) {

                        strongestFSKRSSI[1] = rssi;
                        strongestFSKFreqs[1] = freq;


                        Serial.println(String("Updated FSK F1: ") + strongestFSKFreqs[1] +
                                       " MHz | RSSI: " + strongestFSKRSSI[1]);
                    }
                }

                freq += 0.10; 
            }
        } else {
            Serial.println("Unsupported modulation type");
        }
    }
}

void CC1101_CLASS::enableScanner(float start, float stop) {
    start_freq = start;
    stop_freq = stop;

    if (!CC1101_is_initialized) {
        CC1101_CLASS::init();
    }
    CC1101_CLASS::loadPreset();


        ELECHOUSE_cc1101.setDcFilterOff(1);


    ELECHOUSE_cc1101.setSyncMode(CC1101_SYNC);
    ELECHOUSE_cc1101.setPktFormat(CC1101_PKT_FORMAT);
    ELECHOUSE_cc1101.setModulation(CC1101_MODULATION);
    ELECHOUSE_cc1101.setDeviation(CC1101_DEVIATION);
    ELECHOUSE_cc1101.setDRate(CC1101_DRATE);
    ELECHOUSE_cc1101.setRxBW(CC1101_RX_BW);
    pinMode(CCGDO2A, INPUT);

    // Start scanning on second core
    startSignalanalyseTask();
}
void CC1101_CLASS::sendByteSequence(const uint8_t sequence[], const uint16_t pulseWidth, const uint8_t messageLength) {
    uint8_t dataByte;
    uint8_t i; 
    for (i = 0; i <= messageLength; i++) 
    {
        dataByte = sequence[i];
        for (int8_t bit = 7; bit >= 0; bit--)
        { 
            digitalWrite(CC1101_CCGDO0A, (dataByte & (1 << bit)) != 0 ? HIGH : LOW);
            delayMicroseconds(pulseWidth);
        }
    }    
}
void CC1101_CLASS::signalanalyseTask(void *pvParameters) {
    CC1101_CLASS *cc1101 = static_cast<CC1101_CLASS *>(pvParameters);

    // Initialize scanning parameters
    const uint32_t subghz_frequency_list[] = {
        300000000, 303875000, 304250000, 310000000, 315000000, 318000000,  //  300-348 MHz
        390000000, 418000000, 433075000, 433420000, 433920000, 434420000, 434775000, 438900000,  //  387-464 MHz
        868350000, 868000000, 915000000, 925000000  //  779-928 MHz
    };
    int num_frequencies = sizeof(subghz_frequency_list) / sizeof(subghz_frequency_list[0]);
    int rssi = 0;
    int mark_rssi = -100;
    float mark_freq = 0;
    long compare_freq = 0;

    Serial.println(F("\r\nScanning predefined frequency list, press any key to stop..."));


    ELECHOUSE_cc1101.Init();
    ELECHOUSE_cc1101.setRxBW(58);
    ELECHOUSE_cc1101.SetRx();


    for (int i = 0; i < num_frequencies; ++i) {
        float freq = subghz_frequency_list[i] / 1000000.0;  // Convert to MHz
        ELECHOUSE_cc1101.setMHZ(freq);
        rssi = ELECHOUSE_cc1101.getRssi();

        if (rssi > -75) {  
            if (rssi > mark_rssi) {
                mark_rssi = rssi;
                mark_freq = freq;
            }
        }

        // Print progress
        Serial.print(F("Scanning at "));
        Serial.print(freq, 6);
        Serial.print(F(" MHz, RSSI: "));
        Serial.println(rssi);

        if (Serial.available()) { 
            break;
        }
    }

    if (mark_rssi > -75) {  
        Serial.print(F("\r\nSignal found at "));
        Serial.print(F("Freq: "));
        Serial.print(mark_freq);
        Serial.print(F(" MHz, RSSI: "));
        Serial.println(mark_rssi);
    } else {
        Serial.println(F("\r\nNo strong signal found."));
    }

    Serial.println(F("\r\nScanning stopped."));
    
    ELECHOUSE_cc1101.SetRx();
}

void CC1101_CLASS::startSignalanalyseTask() {
    xTaskCreatePinnedToCore(
        CC1101_CLASS::signalanalyseTask,  // Function to run
        "SignalAnalyseTask",              // Task name
        8192,                             // Stack size
        this,                             // Task parameter (pass instance)
        1,                                // Priority
        NULL,                             // Task handle
        1                                 // Core 1
    );
}

void CC1101_CLASS::signalanalyse(){
 #define signalstorage 10

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

lv_obj_t * textareaRC = screenMgr1.getTextArea();
  bool lastbin=0;
  lv_textarea_set_text(textareaRC, "\nDemodulating\n");


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

uint16_t pulseTrain[SAMPLE_SIZE]; 
size_t length = smoothcount;

for (size_t i = 0; i < smoothcount; i++) {
    pulseTrain[i] = static_cast<uint16_t>(samplesmooth[i]);
}

    
    lv_obj_t * container = screenMgr1.getSquareLineContainer();

    lv_textarea_add_text(textareaRC, "\nNew RAW signal, \nCount: ");
    lv_textarea_add_text(textareaRC, String(smoothcount).c_str());
    String rawString = "";

Serial.println("");
    for (int i = 0; i < smoothcount; i++) {
            rawString += (i > 0 ? (i % 2 == 1 ? " -" : " ") : "");
            rawString += samplesmooth[i];
            Serial.print(samplesmooth[i]);
            Serial.print(", ");
        }
Serial.println("");
lv_textarea_add_text(textareaRC, "\nCapture Complete.");

decodeWithESPiLight(pulseTrain, length);


lv_obj_set_scroll_dir(container, LV_DIR_HOR);
lv_obj_add_flag(container, LV_OBJ_FLAG_SCROLLABLE);

lv_obj_t *chart = lv_chart_create(container);
lv_obj_set_size(chart, 1000, 100);  
lv_chart_set_type(chart, LV_CHART_TYPE_SCATTER);  


lv_chart_series_t *ser1 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);

lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, -10, 40);  

size_t num_elements = sizeof(samplesmooth) / sizeof(samplesmooth[0]);
std::vector<unsigned long> filtered_values;

bool found_first = false;
size_t i_sec = 0;
size_t num_elements_f = 0;

for (size_t i = 0; i < num_elements; ++i) {
    if (samplesmooth[i] > 1000) {
        if (found_first) break;         
        found_first = true;
        i_sec = i;                       
        continue;                      
    }
    
    if (found_first) {    
        if (samplesmooth[i] > 1000) break;          
        if (i - i_sec > 20) break;                 

        filtered_values.push_back(samplesmooth[i] / 100);
        Serial.println(samplesmooth[i] / 100);
        num_elements_f++;
        continue;
    }
    
    
}


int x_pos = 5;           
int high_value = 25;     
int low_value = 5;       
int pulse_width;    

lv_chart_set_point_count(chart, 100);  
Serial.println("yes");

for (int i = 0; i < num_elements_f; i++) {  
pulse_width = filtered_values[i];



    lv_chart_set_next_value2(chart, ser1, x_pos, high_value);
    lv_chart_set_next_value2(chart, ser1, x_pos + pulse_width, high_value);
    

    x_pos += pulse_width;


    lv_chart_set_next_value2(chart, ser1, x_pos, low_value);
    lv_chart_set_next_value2(chart, ser1, x_pos + pulse_width, low_value);


    x_pos += pulse_width;

}

lv_obj_set_style_line_width(chart, 2, LV_PART_ITEMS); 


lv_obj_scroll_to_x(container, 0, LV_ANIM_OFF);

lv_chart_refresh(chart);

     FlipperSubFile subFile;
     CC1101_CLASS::disableReceiver();
    SD_RF.restartSD();

if (!SD_RF.directoryExists("/recordedRF/")) {
    SD_RF.createDirectory("/recordedRF/");
}

String filename = CC1101_CLASS::generateFilename(CC1101_MHZ, CC1101_MODULATION, CC1101_RX_BW);
String fullPath = "/recordedRF/" + filename;

File32* outputFilePtr = SD_RF.createOrOpenFile(fullPath.c_str(), O_WRITE | O_CREAT);
if (outputFilePtr) {
    File32& outputFile = *outputFilePtr; 
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

        std::array<byte, 8> paTable;
        ELECHOUSE_cc1101.SpiReadBurstReg(0x3E, paTable.data(), paTable.size());
        customPresetData.insert(customPresetData.end(), paTable.begin(), paTable.end());
    }
    subFile.generateRaw(outputFile, C1101preset, customPresetData, rawString, CC1101_MHZ);
    SD_RF.closeFile(outputFilePtr);

CC1101_CLASS::enableReceiver();
}
}
void CC1101_CLASS::decodeWithESPiLight(uint16_t *pulseTrain, size_t length) {
    if (length == 0) {
        Serial.println("No pulses to decode.");
        return;
    }

    lv_obj_t *ta = screenMgr1.getTextArea();

    size_t result = espilight.parsePulseTrain(pulseTrain, length);
    if (result > 0) {
        Serial.println("Signal successfully decoded by ESPiLight:");

        String protocolName = ""; 
        String decodedData = "";  

        lv_textarea_set_text(ta, "\nProtocol: ");
        lv_textarea_add_text(ta, protocolName.c_str());
        lv_textarea_add_text(ta, "\nDecoded Data: ");
        lv_textarea_add_text(ta, decodedData.c_str());

        Serial.print("\nProtocol: ");
        Serial.println(protocolName);
        Serial.print("\nDecoded Data: ");
        Serial.println(decodedData);
        return;
    }

    Serial.println("Failed to decode signal with ESPiLight. Trying RC Switch...");

    size_t currentStartIndex = 0;

    while (currentStartIndex < length) {
        size_t startIndex = 0;
        size_t endIndex = 0;
        bool foundSeparator = false;

        // Find the next sequence
        for (size_t i = currentStartIndex; i < length; i++) {
            if (pulseTrain[i] > 1000) {
                if (!foundSeparator) {
                    foundSeparator = true;
                    startIndex = i + 1;  
                } else {
                    endIndex = i;       
                    break;
                }
            }
        }


        if (!foundSeparator || endIndex <= startIndex) {
            Serial.println("No more valid sequences found.");
            break;
        }


        size_t sequenceLength = endIndex - startIndex;
        std::vector<uint16_t> currentSequence(sequenceLength);
        for (size_t i = 0; i < sequenceLength; i++) {
            currentSequence[i] = pulseTrain[startIndex + i];
        }

        if (sequenceLength > RCSWITCH_MAX_CHANGES) {
            Serial.println("Error: Pulse train too long for RC Switch.");
            currentStartIndex = endIndex + 1;  
            continue;
        }


        mySwitch.enableReceive();


        for (size_t i = 0; i < sequenceLength; i++) {
            mySwitch.timings[i] = currentSequence[i];
        }


        for (int protocol = 1; protocol <= mySwitch.getNumProtos(); protocol++) {
            if (mySwitch.receiveProtocol(protocol, sequenceLength)) {
                unsigned long long receivedValue = mySwitch.getReceivedValue();

                Serial.println("Decoded Signal using RC Switch:");
                Serial.print("Protocol: ");
                Serial.println(protDecode[protocol]);
                Serial.print("Value: ");
                Serial.println(receivedValue);
                Serial.print("Bit Length: ");
                Serial.println(mySwitch.getReceivedBitlength());
                Serial.print("Pulse Length: ");
                Serial.println(mySwitch.getReceivedDelay());

                lv_textarea_add_text(ta, "\nDecoded Signal:");
                lv_textarea_add_text(ta, "\nProtocol: ");
                lv_textarea_add_text(ta, protDecode[protocol].c_str());
                lv_textarea_add_text(ta, "\nValue: ");
                lv_textarea_add_text(ta, std::to_string(receivedValue).c_str());
                lv_textarea_add_text(ta, "\nBit Length: ");
                lv_textarea_add_text(ta, std::to_string(mySwitch.getReceivedBitlength()).c_str());
                lv_textarea_add_text(ta, "\nPulse Length: ");
                lv_textarea_add_text(ta, std::to_string(mySwitch.getReceivedDelay()).c_str());

                if (receivedValue == 0) {
                    Serial.println("Unknown encoding.");
                } else {
                    Serial.print("Binary: ");
                    lv_textarea_add_text(ta, "\nBinary: ");
                    for (int i = mySwitch.getReceivedBitlength() - 1; i >= 0; i--) {
                        Serial.print((receivedValue >> i) & 1);
                        lv_textarea_add_text(ta, std::to_string((receivedValue >> i) & 1).c_str());
                    }
                    Serial.println();

                    char hexBuffer[20];
                    snprintf(hexBuffer, sizeof(hexBuffer), "0x%lX", receivedValue);
                    lv_textarea_add_text(ta, "\nHex: ");
                    lv_textarea_add_text(ta, hexBuffer);

                    if (receivedValue <= 0x7F) {
                        char asciiBuffer[2] = {0};
                        asciiBuffer[0] = static_cast<char>(receivedValue);
                        Serial.print("ASCII: '");
                        Serial.write(asciiBuffer[0]);
                        Serial.println("'");

                        lv_textarea_add_text(ta, "\nASCII: '");
                        lv_textarea_add_text(ta, asciiBuffer);
                        lv_textarea_add_text(ta, "'");
                    } else {
                        lv_textarea_add_text(ta, "\nASCII: (non-printable)");
                    }
                }

                mySwitch.resetAvailable();
                return;  
            }
        }


        currentStartIndex = endIndex + 1;
    }

    Serial.println("Failed to decode signal with RC Switch.");
}



void CC1101_CLASS::sendRaw() {
    detachInterrupt(CC1101_CCGDO0A);
    CC1101_CLASS::initrRaw();
    Serial.print(F("\r\nReplaying RAW data from the buffer...\r\n"));

    Serial.print("Transmitting\n");
    Serial.print(smoothcount);
    Serial.print("\n----------------\n");
    for (int i = 1; i < smoothcount - 1; i += 2)
    {
        unsigned long highTime = max((unsigned long)(samplesmooth[i]), 0UL);
        unsigned long lowTime = max((unsigned long)(samplesmooth[i + 1]), 0UL);

        GPIO.out_w1ts = (1 << CC1101_CCGDO0A);
        ets_delay_us(highTime);
        GPIO.out_w1tc = (1 << CC1101_CCGDO0A);
        ets_delay_us(lowTime);

    }
    Serial.print("Transmitted\n");

    ELECHOUSE_cc1101.setSidle();

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

void CC1101_CLASS::sendSamples(int timings[], int timingsLength)
{
    CC1101_CLASS::initrRaw();
    

    pinMode(CC1101_CCGDO0A, OUTPUT);
    Serial.print(F("\r\nReplaying RAW data from the buffer...1\r\n"));

    Serial.print("Transmitting\n");
    Serial.print(timingsLength);

    Serial.print("\n----------------\n");
    for (int i = 1; i < timingsLength - 1; i += 2)
    {
        unsigned long highTime = max((unsigned long)(timings[i]), 0UL);
        unsigned long lowTime = max((unsigned long)(timings[i + 1]), 0UL);

        GPIO.out_w1ts = (1 << CC1101_CCGDO0A);
        ets_delay_us(highTime);
        GPIO.out_w1tc = (1 << CC1101_CCGDO0A);
        ets_delay_us(lowTime);

    }
    Serial.print("Transmitted\n");

    digitalWrite(CC1101_CCGDO0A, LOW); 

    Serial.print(F("\r\nReplaying RAW data complete.\r\n\r\n"));
    ELECHOUSE_cc1101.setSidle(); 
    ELECHOUSE_cc1101.goSleep();  
    disableTransmit();
  //  SDInit();
}

void CC1101_CLASS::enableTransmit()
{
    digitalWrite(CC1101_CS, LOW);
    pinMode(CC1101_CCGDO0A, OUTPUT); 
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
    ELECHOUSE_cc1101.SetTx();
     

 //   mySwitch.enableTransmit(CC1101_CCGDO0A);
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

