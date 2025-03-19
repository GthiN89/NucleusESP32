#include "CC1101.h"
#include "Arduino.h"
#include "ELECHOUSE_CC1101_SRC_DRV.h"
#include "GUI/ScreenManager.h"
#include <sstream>
#include <ctime>
#include <vector>
#include <string> 
#include <iostream>
#include <numeric>
#include <algorithm>
#include "GUI/events.h"
#include "SPI.h"
#include "modules/ETC/SDcard.h"
#include <esp_timer.h>
#include <esp_attr.h>
#include <driver/gpio.h>
#include <esp_intr_alloc.h>
#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>
#include "soc/gpio_struct.h"
#include <driver/timer.h>
#include <typeinfo>
#include <soc/rtc.h>
#include <cstdint>
#include <cmath>
#include <cstdlib>





float start_freq = 433;
float stop_freq = 434;
float freq = start_freq;
long compare_freq;
float mark_freq;
int rssi;
int mark_rssi = -100;
uint32_t localSampleCount = 0;
uint16_t shortPulseAvg = 0;
uint16_t longPulseAvg = 0;
uint32_t pauseAVG = 0;
SDcard& SD_RF = SDcard::getInstance();
ScreenManager& screenMgr1 = ScreenManager::getInstance();

using namespace std;
int receiverGPIO;
String rawString;
uint16_t  sample[SAMPLE_SIZE];
int error_toleranz = 200;
uint8_t samplecount = 0;

bool CC1101_is_initialized = false;
bool receiverEnabled = false;
bool CC1101_recieve_is_running = false;
bool CC1101_transmit_is_running = false;
bool CC1101_isiddle = true;
bool CC1101_interup_attached = false;
uint32_t actualFreq;

int CC1101_MODULATION = 2;
String fullPath;   

RCSwitch mySwitch;

bool reversed;

// float strongestASKFreqs[4] = {0};  // Store the four strongest ASK/OOK frequencies
// int strongestASKRSSI[4] = {-200}; // Initialize with very low RSSI values
// float strongestFSKFreqs[2] = {0}; // Store the two strongest FSK frequencies (F0 and F1)
// int strongestFSKRSSI[2] = {-200}; // Initialize FSK RSSI values

SignalCollection CC1101_CLASS::allData;
volatile  bool recordingStarted = false;
volatile  int64_t startRec = false;

RCSwitch CC1101_CLASS::getRCSwitch() {
 return mySwitch;
}




CC1101_CLASS::ReceivedData CC1101_CLASS::receivedData;

void IRAM_ATTR InterruptHandler(void *arg) {
    if (!gpio_get_level(CC1101_CCGDO0A)) {
        reversed = false;
    } else {
        reversed = true;
    }

    if (recordingStarted) {
        recordingStarted = false;
        startRec = esp_timer_get_time();
    }
    static volatile uint64_t DRAM_ATTR lastTime = 0;
    volatile const uint64_t time = esp_timer_get_time();
    int64_t  duration = time - lastTime;
    lastTime = time;


    // Simple noise filtering
    if (((duration > 100) and (duration > 0)) or ((-duration > 100) and (-duration > 0))) { 
        noInterrupts();
        if (CC1101_CLASS::receivedData.samples.size() < SAMPLE_SIZE) {
            if(reversed){
                CC1101_CLASS::receivedData.samples.push_back(-duration);
            } else {
                CC1101_CLASS::receivedData.samples.push_back(duration);
            }
            CC1101_CLASS::receivedData.lastReceiveTime = esp_timer_get_time();
            CC1101_CLASS::receivedData.sampleCount++;
        }
        if (duration > 50000 or duration < -50000) {
                CC1101_CLASS::receivedData.samples.clear();
        }
        interrupts();
    }
}


//encoders


bool CC1101_CLASS::init() {
    digitalWrite(PN532_SS, HIGH);
    SPI.end();
    delay(10);
    digitalWrite(CC1101_CS, LOW);
    ELECHOUSE_cc1101.setSpiPin(CC1101_SCLK, CC1101_MISO, CC1101_MOSI, CC1101_CS);
    ELECHOUSE_cc1101.Init();

    if (ELECHOUSE_cc1101.getCC1101()) {
        ELECHOUSE_cc1101.SpiWriteReg(CC1101_AGCCTRL2, 0x07);  // Maximum LNA gain
        ELECHOUSE_cc1101.SpiWriteReg(CC1101_AGCCTRL1, 0x00);  // Fastest AGC
        ELECHOUSE_cc1101.SpiWriteReg(CC1101_AGCCTRL0, 0x91);  // Normal AGC, 16 samples
        ELECHOUSE_cc1101.SpiWriteReg(CC1101_FREND1, 0xB6);    // RX frontend configuration
        ELECHOUSE_cc1101.SpiWriteReg(CC1101_FREND0, 0x10);    // TX frontend configuration
        
        ELECHOUSE_cc1101.setGDO(CCGDO0A, CCGDO0A);
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
/*
+------------------+----------------+
| Threshold (dBm)  | AGCCTRL1 Value |
+------------------+----------------+
| -80              | 0x80           |
| -70              | 0x88           |
| -60              | 0x90           |
| -50              | 0x98           |
| -40              | 0xA0           |
| -30              | 0xA8           |
| -20              | 0xB0           |
| -10              | 0xB8           |
|   0              | 0xC0           |
| +10              | 0xC8           |
| +20              | 0xD0           |
| +30              | 0xD8           |
| +40              | 0xE0           |
| +50              | 0xE8           |
| +60              | 0xF0           |
| +70              | 0xF8           |
| +80              | 0xFF           |
+------------------+----------------+
+------------+--------+-------------------------+
| GDO Pin    | Reg    | Inversion Bit (INV)    |
+------------+--------+-------------------------+
| GDO2       | 0x00   | Bit 6 (1 = Inverted)   |
| GDO1       | 0x01   | Bit 6 (1 = Inverted)   |
| GDO0       | 0x02   | Bit 6 (1 = Inverted)   |
+------------+--------+-------------------------+
| Note: Inversion is controlled by setting Bit 6 
|       of the respective IOCFGx register.
+------------------------------------------------+
*/

void CC1101_CLASS::enableReceiver() {
        //Serial.println("CC1101: enableReceiver");
        CC1101_CLASS::allData.empty();
        samplecount = 0;

     ELECHOUSE_cc1101.SpiStrobe(0x30); // Reset CC1101
     localSampleCount = 0;
     delay(50);


        CC1101_CLASS::init();

    CC1101_CLASS::loadPreset();



    ELECHOUSE_cc1101.SpiWriteReg(CC1101_IOCFG1, 0x0D); // Set GP2
    ELECHOUSE_cc1101.SpiWriteReg(CC1101_PKTCTRL0, 0x32); // Async mode
    ELECHOUSE_cc1101.setDRate(CC1101_DRATE);
    delay(10);

    ELECHOUSE_cc1101.setRxBW(CC1101_RX_BW);
    ELECHOUSE_cc1101.setDcFilterOff(1);
    ELECHOUSE_cc1101.setPktFormat(3);
    ELECHOUSE_cc1101.setModulation(CC1101_MODULATION);
    ELECHOUSE_cc1101.setMHZ(CC1101_MHZ);
    ELECHOUSE_cc1101.setDeviation(CC1101_DEVIATION);
    ELECHOUSE_cc1101.setDRate(CC1101_DRATE);

    delay(10);

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << CC1101_CCGDO0A),
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_ANYEDGE
    };
    gpio_config(&io_conf);
   // gpio_pulldown_en(GPIO_NUM_17);
        ELECHOUSE_cc1101.SetRx();
        delay(20);
    if(!gpio_get_level(CC1101_CCGDO0A)) {
        delay(10);
        uint8_t iocfg0 = ELECHOUSE_cc1101.SpiReadReg(CC1101_IOCFG1);
        iocfg0 |= (1 << 6); 
        ELECHOUSE_cc1101.SpiWriteReg(CC1101_IOCFG1, iocfg0);
        delay(20);
    }
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    gpio_isr_handler_add(CC1101_CCGDO0A, InterruptHandler, NULL);

  //  ELECHOUSE_cc1101.SetRx();
    receiverEnabled = true;

        CC1101_CLASS::receivedData.samples.clear();
        CC1101_CLASS::receivedData.lastReceiveTime = 0;
        CC1101_CLASS::receivedData.sampleCount = 0;
        CC1101_CLASS::receivedData.signals.clear();

        delay(500);
    recordingStarted = true;

    interrupts();
}


void CC1101_CLASS::emptyReceive() {
       ELECHOUSE_cc1101.SpiStrobe(0x30); // Reset CC1101
     localSampleCount = 0;
     delay(50);


        CC1101_CLASS::init();

    CC1101_CLASS::loadPreset();



    ELECHOUSE_cc1101.SpiWriteReg(CC1101_IOCFG1, 0x0D); // Set GP2
        delay(10);
    ELECHOUSE_cc1101.SpiWriteReg(CC1101_PKTCTRL0, 0x32); // Async mode
    delay(10);

    ELECHOUSE_cc1101.setRxBW(CC1101_RX_BW);
        delay(10);
    ELECHOUSE_cc1101.setDcFilterOff(1);
    ELECHOUSE_cc1101.setPktFormat(3);
    ELECHOUSE_cc1101.setModulation(CC1101_MODULATION);
    ELECHOUSE_cc1101.setMHZ(CC1101_MHZ);
    ELECHOUSE_cc1101.setDeviation(CC1101_DEVIATION);
    ELECHOUSE_cc1101.setDRate(CC1101_DRATE);
    ELECHOUSE_cc1101.setPA(12);
    delay(10);

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_NUM_17),
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_ANYEDGE
    };
    gpio_config(&io_conf);
    gpio_pulldown_en(GPIO_NUM_17);
        ELECHOUSE_cc1101.SetRx();
        delay(20);
    if(!gpio_get_level(CC1101_CCGDO0A)) {
        delay(10);
        uint8_t iocfg0 = ELECHOUSE_cc1101.SpiReadReg(CC1101_IOCFG1);
        iocfg0 |= (1 << 6); 
        ELECHOUSE_cc1101.SpiWriteReg(CC1101_IOCFG1, iocfg0);
        delay(20);
    }
}


void CC1101_CLASS::enableReceiverCustom() {
    CC1101TH TH;
    //Serial.println("Custom Receiver enabled");
     int BW =  lv_spinbox_get_value(screenMgr1.SubGHzCustomScreen_->spinbox);
     int TRS = lv_spinbox_get_value(screenMgr1.SubGHzCustomScreen_->spinbox1);
     int SYNC = lv_spinbox_get_value(screenMgr1.SubGHzCustomScreen_->spinbox2);
     int PTK = lv_spinbox_get_value(screenMgr1.SubGHzCustomScreen_->spinbox3);
     int MOD = lv_spinbox_get_value(screenMgr1.SubGHzCustomScreen_->spinbox4);       
     int DEV = lv_spinbox_get_value(screenMgr1.SubGHzCustomScreen_->spinbox5);
     int DRATE = lv_spinbox_get_value(screenMgr1.SubGHzCustomScreen_->spinbox6);
      //Serial.println(BW);
      //Serial.println(TRS);
      //Serial.println(SYNC);
      //Serial.println(PTK);
      //Serial.println(MOD);
      //Serial.println(DEV);
      //Serial.println(DRATE);

       ELECHOUSE_cc1101.SpiStrobe(0x30); // Reset CC1101
     localSampleCount = 0;
     delay(50);


        CC1101_CLASS::init();

    CC1101_CLASS::loadPreset();

 ELECHOUSE_cc1101.SpiWriteReg(CC1101_IOCFG1, 0x0D); // Set GP2
        delay(10);
    ELECHOUSE_cc1101.SpiWriteReg(CC1101_PKTCTRL0, 0x32); // Async mode
    delay(10);
    ELECHOUSE_cc1101.SpiWriteReg(CC1101_AGCCTRL1 ,TH.getRegValue(TRS));
   delay(10);

    delay(10);
    ELECHOUSE_cc1101.setRxBW(BW);
        delay(10);
    ELECHOUSE_cc1101.setDcFilterOff(1);
    ELECHOUSE_cc1101.setPktFormat(3);
    ELECHOUSE_cc1101.setModulation(MOD);
    ELECHOUSE_cc1101.setMHZ(CC1101_MHZ);
    ELECHOUSE_cc1101.setDeviation(DEV);
    ELECHOUSE_cc1101.setDRate(DRATE);

 delay(10);

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_NUM_17),
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_ANYEDGE
    };
    gpio_config(&io_conf);
   // gpio_pulldown_en(GPIO_NUM_17);
        ELECHOUSE_cc1101.SetRx();
        delay(20);
    if(!gpio_get_level(CC1101_CCGDO0A)) {
        delay(10);
        uint8_t iocfg0 = ELECHOUSE_cc1101.SpiReadReg(CC1101_IOCFG1);
        iocfg0 |= (1 << 6); 
        ELECHOUSE_cc1101.SpiWriteReg(CC1101_IOCFG1, iocfg0);
        delay(20);
    }
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    gpio_isr_handler_add(GPIO_NUM_17, InterruptHandler, NULL);

  //  ELECHOUSE_cc1101.SetRx();
    receiverEnabled = true;

        CC1101_CLASS::receivedData.samples.clear();
        CC1101_CLASS::receivedData.lastReceiveTime = 0;
        CC1101_CLASS::receivedData.sampleCount = 0;
        CC1101_CLASS::receivedData.signals.clear();

        delay(500);

    interrupts();

}

void CC1101_CLASS::setCC1101Preset(CC1101_PRESET preset) {
    C1101preset = preset;
}



void CC1101_CLASS::disableReceiver()
{
    gpio_isr_handler_remove(GPIO_NUM_17);
    gpio_uninstall_isr_service();
    ELECHOUSE_cc1101.setSidle();
    CC1101.emptyReceive();
 
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
        // CC1101_SYNC = 0x47;          // Sync word from `Custom_preset_data` (uint8_t 3)
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
        CC1101_SYNC = 0x47;          // Sync word from `Custom_preset_data` (uint8_t 3)
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
         CC1101_SYNC = 0x47;          // Sync word from `Custom_preset_data` (uint8_t 3)
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
        //Serial.println(CC1101_MODULATION);
        break;
    }
    //Serial.print("preset loaded");
}

bool CC1101_CLASS::CheckReceived() {
    if(CC1101_CLASS::receivedData.sampleCount  > 512) {
        CC1101_CLASS::receivedData.sampleCount = 0;
        CC1101_CLASS::receivedData.lastReceiveTime = 0;
        return true;
    }
    else if (CC1101_CLASS::receivedData.sampleCount  < 24 or
            (esp_timer_get_time() - CC1101_CLASS::receivedData.lastReceiveTime) > 3000000) {
            return false;
    }
     else if (CC1101_CLASS::receivedData.sampleCount  > 24 and
            (esp_timer_get_time() - startRec) > 500000) {
        CC1101_CLASS::receivedData.sampleCount = 0;
        CC1101_CLASS::receivedData.lastReceiveTime = 0;
        return true;
    
    }
    return false;
}

void CC1101_CLASS::fskAnalyze() {
  //
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
    pinMode(CCGDO0A, INPUT);

    // Start scanning on second core
    CC1101_CLASS::startSignalAnalyseTask();
}
void CC1101_CLASS::sendByteSequence(const uint8_t sequence[], const uint16_t pulseWidth, const uint8_t messageLength) {
    CC1101_CLASS::init();
    delay(5);
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
 void CC1101_CLASS::signalAnalyseTask(void* pvParameters) {
    //CC1101_CLASS *cc1101 = static_cast<CC1101_CLASS *>(pvParameters);

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
 //   long compare_freq = 0;

    //Serial.println(F("\r\nScanning predefined frequency list, press any key to stop..."));


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
        //Serial.print(F("Scanning at "));
        //Serial.print(freq, 6);
        //Serial.print(F(" MHz, RSSI: "));
        //Serial.println(rssi);

        if (Serial.available()) { 
            break;
        }
    }

    if (mark_rssi > -75) {  
        //Serial.print(F("\r\nSignal found at "));
        //Serial.print(F("Freq: "));
        //Serial.print(mark_freq);
  
  
      
  //Serial.print(F(" MHz, RSSI: "));
  //Serial.println(mark_rssi);
} else {
  //Serial.println(F("\r\nNo strong signal found."));
}

//Serial.println(F("\r\nScanning stopped."));

ELECHOUSE_cc1101.SetRx();
}


void CC1101_CLASS::startSignalAnalyseTask() {
    xTaskCreatePinnedToCore(
        CC1101_CLASS::signalAnalyseTask,  // Function to run
        "SignalAnalyseTask",              // Task name
        8192,                             // Stack size
        this,                             // Task parameter (pass instance)
        1,                                // Priority
        NULL,                             // Task handle
        1                                 // Core 1
    );
}


void CC1101_CLASS::handleSignal(){

////////////////////////////////////////////////////
        Signal data;

     //   //Serial.println(F("Raw samples: "));
        CC1101_CLASS::receivedData.sampleCount = 0;
        
        for (const auto &sample : CC1101_CLASS::receivedData.samples) {
            //Serial.print(sample);
            data.addSample(sample);
            //Serial.print(" ");
            CC1101_CLASS::receivedData.sampleCount++;
        }

        CC1101_CLASS::allData.addSignal(data);

        

     lv_obj_t * textareaRC;
    if(C1101preset == CUSTOM){
        textareaRC = screenMgr1.text_area_SubGHzCustom;        
    } else {
        textareaRC = screenMgr1.getTextArea();
    }

    lv_textarea_set_text(textareaRC, "\nRAW signal");//, \nCount: ");

    std::ostringstream  rawString;

    for (const auto& sample : CC1101_CLASS::receivedData.samples) {
        rawString << sample << " ";
    }

    void filterSignal();



CC1101_CLASS::disableReceiver();

SD_RF.restartSD();

if (!SD_RF.directoryExists("/recordedRFRawAll/")) {
    SD_RF.createDirectory("/recordedRFRawAll/");
}

String filename = CC1101_CLASS::generateFilename(CC1101_MHZ, CC1101_MODULATION, CC1101_RX_BW);
String fullPath = "/recordedRFRawAll/" + filename;
FlipperSubFile subFile;
File32* outputFilePtr = SD_RF.createOrOpenFile(fullPath.c_str(), O_WRITE | O_CREAT);
if (outputFilePtr) {
    File32& outputFile = *outputFilePtr; 
    std::vector<uint8_t> customPresetData;
if (C1101preset == CUSTOM) {
    customPresetData.insert(customPresetData.end(), {
        CC1101_MDMCFG4, ELECHOUSE_cc1101.SpiReadReg(CC1101_MDMCFG4),
        CC1101_MDMCFG3, ELECHOUSE_cc1101.SpiReadReg(CC1101_MDMCFG3),
        CC1101_MDMCFG2, ELECHOUSE_cc1101.SpiReadReg(CC1101_MDMCFG2),
        CC1101_DEVIATN, ELECHOUSE_cc1101.SpiReadReg(CC1101_DEVIATN),
        CC1101_FREND0,  ELECHOUSE_cc1101.SpiReadReg(CC1101_FREND0),
        0x00, 0x00
    });

    std::array<uint8_t, 8> paTable;
    ELECHOUSE_cc1101.SpiReadBurstReg(0x3E, paTable.data(), paTable.size());
    customPresetData.insert(customPresetData.end(), paTable.begin(), paTable.end());
}
subFile.generateRaw(outputFile, C1101preset, customPresetData, rawString, CC1101_MHZ);
SD_RF.closeFile(outputFilePtr);
}
}

bool CC1101_CLASS::decode() {

    // for (int i=0; i >CC1101_CLASS::receivedData.samples.size(); i++) {
    //     //Serial.print(CC1101_CLASS::receivedData.samples[i]);
    //     //Serial.print(", ");
    // }
    if (!CC1101_CLASS::receivedData.signals.empty()) {
        const auto& lastSignal = CC1101_CLASS::receivedData.signals.back();
        // for (int i = 0; i < lastSignal.samples.size(); i++) {
        //     //Serial.print(lastSignal.samples[i]);
        //     //Serial.print(", ");
        // }
    }
    

    if (CC1101_CLASS::receivedData.samples.empty()) {
  //      //Serial.println("No pulses to decode.");
        return false;
    }
            //Serial.println("decode.");


    filterSignal();
//    //Serial.println("count:");
//    //Serial.println(CC1101_CLASS::receivedData.samples.size());
//    //Serial.println("Pulses:");
   //Serial.println(pulses[0]);
   //Serial.println(pulses[1]);
  //  delay(5);
    // //Serial.println("filtered values\n");
    // for(int i = 0; i < CC1101.receivedData.filtered.size(); i++) {
    //     //Serial.print(CC1101.receivedData.filtered[i]);
    //     //Serial.print(", ");
    // }
    if ((DURATION_DIFF(pulses[0], 500) < 40) &&
        (DURATION_DIFF(pulses[1], 1000) < 90)) {
            //Serial.println("is Hormann");
        if (hormannProtocol.decode(CC1101.receivedData.filtered.data(), CC1101_CLASS::receivedData.samples.size())) {
            hormannProtocol.getCodeString(pulses[0], pulses[1]);
            return true;
        }
    }

    if ((DURATION_DIFF(pulses[0], 320) < 50) &&
        (DURATION_DIFF(pulses[1], 640) < 90)) {
            //Serial.println("is Came");
        if (cameProtocol.decode(CC1101.receivedData.filtered.data(), CC1101_CLASS::receivedData.samples.size())) {
            cameProtocol.getCodeString(pulses[0], pulses[1]);
            return true;
        }
    }

    if ((DURATION_DIFF(pulses[0], 555) < 40) &&
        (DURATION_DIFF(pulses[1], 1111) < 90)) {
            //Serial.println("is Ansonic");
        if (ansonicProtocol.decode(CC1101.receivedData.filtered.data(), CC1101_CLASS::receivedData.samples.size())) {
            ansonicProtocol.getCodeString(pulses[0], pulses[1]);
            return true;
        }
    }

    if ((DURATION_DIFF(pulses[0], 700) < 50) &&
        (DURATION_DIFF(pulses[1], 1400) < 90)) {
           //Serial.println("is NiceFlow");
        if (niceFloProtocol.decode(CC1101.receivedData.filtered.data(), CC1101_CLASS::receivedData.samples.size())) {
            niceFloProtocol.getCodeString(pulses[0], pulses[1]);
            return true;
        }
    }

    if ((DURATION_DIFF(pulses[0], 300) < 50) &&
        (DURATION_DIFF(pulses[1], 900) < 90)) {
            //Serial.println("is SMC5326");
        if (smc5326Protocol.decode(CC1101.receivedData.filtered.data(), CC1101_CLASS::receivedData.samples.size())) {
            smc5326Protocol.getCodeString(pulses[0], pulses[1]);
            return true;
        }
    }

    std::ostringstream samples;
    for (size_t i = 0; i < CC1101.receivedData.filtered.size(); ++i) {
        samples << CC1101.receivedData.filtered.data()[i];
        if(i != CC1101.receivedData.filtered.size() - 1)
            samples << " ";
    }


   

    CC1101_CLASS::receivedData.samples.clear();
    CC1101_CLASS::receivedData.sampleCount = 0;

    return false;
}




void CC1101_CLASS::sendRaw() {
    CC1101_CLASS::init();
    delay(5);
            Signal samplesData;

            if(CC1101_CLASS::allData.signals.empty()) return;

            if(C1101CurrentState != STATE_BRUTE) {
            detachInterrupt(CC1101_CCGDO0A);
            detachInterrupt(CC1101_CCGDO0A);


             if(!CC1101_CLASS::receivedData.filtered.empty()) {
               Signal filtered;
               for (int i = 0; CC1101_CLASS::receivedData.filtered.size() < i; i++){
                        filtered.addSample(CC1101_CLASS::receivedData.filtered[i]);
               }
            }
            samplesData = CC1101_CLASS::allData.getSignal(CC1101_CLASS::allData.signals.size() - 1);
            CC1101_CLASS::initRaw();}
            else  {
                CC1101_CLASS::allData.clear();
                for (size_t i = 0; i < 26; i++)
                {
                    samplesData.addSample(samplesToSend[i]);

                }
            }



           
            CC1101_CLASS::levelFlag = samplesData.samples[0] > 0; 

            samplesToSend.clear();
            for (size_t j = 0; j < samplesData.samples.size(); ++j) {
                if (samplesData.samples[j] > 0){
                samplesToSend.push_back(samplesData.samples[j]);
                } else {
                    samplesToSend.push_back(-samplesData.samples[j]);
                }

            }

     

            for (size_t i = 0; i < samplesToSend.size(); i++) {
                
                gpio_set_level(CC1101_CCGDO0A, CC1101_CLASS::levelFlag);
                CC1101_CLASS::levelFlag = !CC1101_CLASS::levelFlag;
                delayMicroseconds(samplesToSend[i]); 
                
            }
            CC1101_CLASS::disableTransmit();

}

void CC1101_CLASS::enableRCSwitch(){
     if (ELECHOUSE_cc1101.getCC1101()){       // Check the CC1101 Spi connection.
  //Serial.println("Connection OK");
  }else{
  //Serial.println("Connection Error");
  }

  ELECHOUSE_cc1101.Init();            // must be set to initialize the cc1101!
  ELECHOUSE_cc1101.setMHZ(CC1101_FREQ); // Here you can set your basic frequency. The lib calculates the frequency automatically (default = 433.92).The cc1101 can: 300-348 MHZ, 387-464MHZ and 779-928MHZ. Read More info from datasheet.
    mySwitch.setReceiveTolerance(20);
    mySwitch.enableReceive(CC1101_CCGDO0A);  // Receiver on interrupt 0 => that is pin #2

  ELECHOUSE_cc1101.SetRx();  // set Receive on
}



void CC1101_CLASS::initRaw() {
  //Serial.print("Init CC1101 raw");


        CC1101_CLASS::init();



    ELECHOUSE_cc1101.Init();                // must be set to initialize the cc1101!
    ELECHOUSE_cc1101.setGDO0(CC1101_CCGDO0A);         // set lib internal gdo pin (gdo0). Gdo2 not use for this example.
    ELECHOUSE_cc1101.setCCMode(0);          // set config for internal transmission mode. value 0 is for RAW recording/replaying
    ELECHOUSE_cc1101.setModulation(CC1101_MODULATION);      // set modulation mode. 0 = 2-FSK, 1 = GFSK, 2 = ASK/OOK, 3 = 4-FSK, 4 = MSK.
    ELECHOUSE_cc1101.setDeviation(CC1101_DEVIATION);   // Set the Frequency deviation in kHz. Value from 1.58 to 380.85. Default is 47.60 kHz.
    ELECHOUSE_cc1101.setDRate(CC1101_DRATE);
    ELECHOUSE_cc1101.setPktFormat(3);
    ELECHOUSE_cc1101.SetTx();
    
    gpio_set_direction(CC1101_CCGDO0A, GPIO_MODE_OUTPUT); 
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

void CC1101_CLASS::sendSamples(int timings[], int timingsLength, bool levelFlag)
{
    
    
    //Serial.print(F("\r\nReplaying RAW data from the buffer...1\r\n"));
    //Serial.print("Transmitting\n");
    //Serial.print(timingsLength);
    //Serial.print("\n----------------\n");

    for (size_t i = 0; i < timingsLength; i++) {
        gpio_set_level(CC1101_CCGDO0A, levelFlag);
        levelFlag = !levelFlag;
        delayMicroseconds(timings[i]);                 
    }

    //Serial.print("Transmitted\n");
    digitalWrite(CC1101_CCGDO0A, LOW); 
    //Serial.print(F("\r\nReplaying RAW data complete.\r\n\r\n"));

}

void CC1101_CLASS::enableTransmit()
{
    digitalWrite(CC1101_CS, LOW);
    pinMode(CC1101_CCGDO0A, OUTPUT); 
    CC1101_CLASS::init();
    delay(5);
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
    gpio_set_pull_mode(CC1101_CCGDO0A, GPIO_FLOATING); 
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

void CC1101_CLASS::sendEncoded(RFProtocol protocol, float frequency, int16_t bitLenght, int8_t repeats, int64_t code){
    CC1101_CLASS::init();
    delay(5);
    pinMode(CC1101_CCGDO0A, OUTPUT);
    digitalWrite(CC1101_CCGDO0A, LOW);
    //Serial.println(frequency);
    setFrequency(frequency);
    setCC1101Preset(AM650);
    loadPreset();
    ELECHOUSE_cc1101.setPA(12);
    initRaw();

    switch (protocol) {
        case CAME:
                cameProtocol.yield(code);
                delay(5);


                for(int k = 0; k < repeats; k++) {
                    bool levelFlag = false;
                    for (size_t j = 0; j < samplesToSend.size(); j++) {
                        gpio_set_level(CC1101_CCGDO0A, levelFlag);
                        levelFlag = !levelFlag; 
                        delayMicroseconds(samplesToSend[j]);                
                }
                gpio_set_level(CC1101_CCGDO0A, LOW);
                delayMicroseconds(11520);
                }
            break;
    
        case NICE:
                niceFloProtocol.yield(code);
                delay(5);


                for(int k = 0; k < repeats; k++) {
                    bool levelFlag = false;
                    for (size_t j = 0; j < samplesToSend.size(); j++) {
                        gpio_set_level(CC1101_CCGDO0A, levelFlag);
                        levelFlag = !levelFlag; 
                        delayMicroseconds(samplesToSend[j]);                
                }
                gpio_set_level(CC1101_CCGDO0A, LOW);
                delayMicroseconds(25200);
                }
            break; 
    
        case ANSONIC:
                ansonicProtocol.yield(code);
                delay(5);


                for(int k = 0; k < repeats; k++) {
                    bool levelFlag = false;
                    for (size_t j = 0; j < samplesToSend.size(); j++) {
                        gpio_set_level(CC1101_CCGDO0A, levelFlag);
                        levelFlag = !levelFlag; 
                        delayMicroseconds(samplesToSend[j]);                
                }
                gpio_set_level(CC1101_CCGDO0A, LOW);
                delayMicroseconds(19425);
                }
            break;
    
        case HOLTEK:
                holtekProtocol.yield(code);
                delay(5);


                for(int k = 0; k < repeats; k++) {
                    bool levelFlag = false;
                    for (size_t j = 0; j < samplesToSend.size(); j++) {
                        gpio_set_level(CC1101_CCGDO0A, levelFlag);
                        levelFlag = !levelFlag; 
                        delayMicroseconds(samplesToSend[j]);                
                }
                gpio_set_level(CC1101_CCGDO0A, LOW);
                delayMicroseconds(14400);
                }
            break;
    
        case LINEAR:
            while(encoderState != EncoderStepReady) {
                // ...
            }
            break;
    
        case SMC5326:
            while(encoderState != EncoderStepReady) {
                smc5326Protocol.yield(code);
            }
            break;
    
        default:
            break;
    }
    

    

}






bool CC1101_CLASS::checkReversed(int64_t big) {
    for(int i = 0; i < CC1101.receivedData.samples.size(); i++) {
        if(CC1101.receivedData.samples[i] > (big *13)) return true;
    }
    return false;
}

void CC1101_CLASS::reverseLogicState() {
    for(int i = 0; i < CC1101.receivedData.samples.size(); i++) {
        CC1101.receivedData.samples[i] = -CC1101.receivedData.samples[i];
    }
}

void CC1101_CLASS::filterAll() {
    CC1101.receivedData.filtered.clear();
    int64_t shortMin = pulses[0] * 0.7;
    int64_t shortMax = pulses[0] * 1.3;
    int64_t longMin  = pulses[1] * 0.7;
    int64_t longMax  = pulses[1] * 1.3;
    int64_t spaceMin = pulses[1] * 13;
    int64_t space    = pulses[1] * 18;
    //Serial.println(spaceMin);

    for (int i = 0; i < CC1101.receivedData.samples.size(); i++) {
        int64_t sample = CC1101.receivedData.samples[i];
        if (sample > 0) {
            if (sample > spaceMin) {
                CC1101.receivedData.filtered.push_back(space);
            } else if (sample > shortMin && sample < shortMax) {
                CC1101.receivedData.filtered.push_back(pulses[0]);
            } else if (sample > longMin && sample < longMax) {
                CC1101.receivedData.filtered.push_back(pulses[1]);
            }
        } else {
            sample = -sample;
            if (sample > spaceMin) {
                CC1101.receivedData.filtered.push_back(-space);
            } else if (sample > shortMin && sample < shortMax) {
                CC1101.receivedData.filtered.push_back(-pulses[0]);
            } else if (sample > longMin && sample < longMax) {
                CC1101.receivedData.filtered.push_back(-pulses[1]);
            }
        }
    }


    if (!SD_RF.directoryExists("/recordedFilteredAll/")) {
        SD_RF.createDirectory("/recordedFilteredAll/");
    }

    String filename = CC1101_CLASS::generateFilename(CC1101_MHZ, CC1101_MODULATION, CC1101_RX_BW);
    String fullPath = "/recordedFilteredAll/" + filename;
    FlipperSubFile subFile;
    File32* outputFilePtr = SD_RF.createOrOpenFile(fullPath.c_str(), O_WRITE | O_CREAT);
    if (outputFilePtr) {
        File32& outputFile = *outputFilePtr; 
        std::vector<uint8_t> customPresetData;
    if (C1101preset == CUSTOM) {
        customPresetData.insert(customPresetData.end(), {
            CC1101_MDMCFG4, ELECHOUSE_cc1101.SpiReadReg(CC1101_MDMCFG4),
            CC1101_MDMCFG3, ELECHOUSE_cc1101.SpiReadReg(CC1101_MDMCFG3),
            CC1101_MDMCFG2, ELECHOUSE_cc1101.SpiReadReg(CC1101_MDMCFG2),
            CC1101_DEVIATN, ELECHOUSE_cc1101.SpiReadReg(CC1101_DEVIATN),
            CC1101_FREND0,  ELECHOUSE_cc1101.SpiReadReg(CC1101_FREND0),
            0x00, 0x00
        });
    
        std::array<uint8_t, 8> paTable;
        ELECHOUSE_cc1101.SpiReadBurstReg(0x3E, paTable.data(), paTable.size());
        customPresetData.insert(customPresetData.end(), paTable.begin(), paTable.end());
    }
    std::ostringstream filteredString;
    for (const auto& sample : CC1101_CLASS::receivedData.filtered) {
        filteredString << sample << " ";
    }
    subFile.generateRaw(outputFile, C1101preset, customPresetData, filteredString, CC1101_MHZ);
    SD_RF.closeFile(outputFilePtr);
}
    
}


void CC1101_CLASS::filterSignal() {
    const auto& samples = CC1101.receivedData.samples;
    if (samples.empty()) return;

    std::vector<int64_t> absArr;
    absArr.reserve(samples.size());
    for (auto x : samples)
        absArr.push_back(std::abs(x));
    if (absArr.empty()) return;
    std::sort(absArr.begin(), absArr.end());

    std::vector<std::vector<int64_t>> groups;
    std::vector<int64_t> currGroup;
    currGroup.push_back(absArr[0]);
    int64_t groupMin = absArr[0];

    for (size_t i = 1; i < absArr.size(); i++) {
        int64_t x = absArr[i];
        if (x <= static_cast<int64_t>(1.3 * groupMin))
            currGroup.push_back(x);
        else {
            groups.push_back(std::move(currGroup));
            currGroup = { x };
            groupMin = x;
        }
    }
    if (!currGroup.empty())
        groups.push_back(std::move(currGroup));
    if (groups.empty()) return;

    if (groups.size() == 1) {
        const auto& grp = groups[0];
        size_t mid = grp.size() / 2;
        int64_t median = grp[mid];
        pulses = { median };
        return;
    }

    std::vector<std::pair<int64_t, int64_t>> groupStats;
    groupStats.reserve(groups.size());
    for (auto &g : groups) {
        size_t mid = g.size() / 2;
        groupStats.push_back({ static_cast<int64_t>(g.size()), g[mid] });
    }

    std::sort(groupStats.begin(), groupStats.end(),
    [](const std::pair<int64_t, int64_t>& a, const std::pair<int64_t, int64_t>& b) {
         return (a.first == b.first) ? (a.second < b.second) : (a.first > b.first);
    });
    if (groupStats.size() < 2) return;

    int64_t rep1 = groupStats[0].second;
    int64_t rep2 = groupStats[1].second;
    if (rep1 > rep2) std::swap(rep1, rep2);

    int64_t sum = rep1 + rep2;
    constexpr int numCandidates = 8;
    std::array<uint32_t, numCandidates> diffs;
    std::array<int64_t, numCandidates> smallCandidates, bigCandidates;
    for (int i = 0; i < numCandidates; i++) {
        int denom = i + 3;       // 3..10
        int multiplier = i + 2;    // 2..9
        smallCandidates[i] = sum / denom;
        bigCandidates[i] = smallCandidates[i] * multiplier;
        diffs[i] = DURATION_DIFF(rep1, smallCandidates[i]);
    }
    auto minIt = std::min_element(diffs.begin(), diffs.end());
    size_t index = std::distance(diffs.begin(), minIt);

    pulses.clear();
    pulses.push_back(smallCandidates[index]);
    pulses.push_back(bigCandidates[index]);

    if (checkReversed(rep2))
        reverseLogicState();

    if (DURATION_DIFF(pulses[0], rep1) < 40 && DURATION_DIFF(pulses[1], rep2) < 80)
        filterAll();
    else
        pulses = { rep1, rep2 };

    //Serial.print("Pulses Med: ");
    //Serial.print(rep1);
    //Serial.print(" ");
    //Serial.println(rep2);
}


