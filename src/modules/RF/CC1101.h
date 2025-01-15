#ifndef C1101_H
#define C1101_H

#include "../../globals.h"
#include "RCSwitch.h"
#include "ESPiLight.h"

#include "SPI.h"
#define SAMPLE_SIZE 2048
#define MAX_SIGNAL_LENGTH 10000000  


   //---------------------------------------------------------------------------//
  //-----------------------------Presets-Variables-----------------------------//
 //---------------------------------------------------------------------------//
   //---------------------------------------------------------------------------//
  //-----------------------------Flags-----------------------------------------//
 //---------------------------------------------------------------------------//
 extern bool CC1101_is_initialized;
 extern bool CC1101_recieve_is_running;
 extern bool CC1101_transmit_is_running;
 extern bool CC1101_isiddle;
 extern bool CC1101_interup_attached;
 extern uint16_t  sample[];
 extern uint8_t samplecount;
 extern bool startLow;
 extern uint32_t actualFreq;
extern float strongestASKFreqs[4];  // Store the four strongest ASK/OOK frequencies
extern int strongestASKRSSI[4]; // Initialize with very low RSSI values
extern float strongestFSKFreqs[2]; // Store the two strongest FSK frequencies (F0 and F1)
extern int strongestFSKRSSI[2]; // Initialize FSK RSSI values




class CC1101_CLASS {
public:
    float CC1101_DRATE = 3.79372;;
    float CC1101_RX_BW = 650.00;;
    float CC1101_DEVIATION = 47.60;
    int CC1101_PKT_FORMAT = 0;
    int CC1101_SYNC = 2;
    float CC1101_FREQ = 433.92;
    int CC1101_MODULATION;
    typedef struct
{
  std::vector<unsigned long> samples;
  volatile unsigned long lastReceiveTime = 0;
  volatile unsigned long sampleCount = 0;
  volatile unsigned long normalizedCount = 0;
  std::vector<uint16_t> pulseTrainVec;
} recievedData;

    
    bool init();
    RCSwitch getRCSwitch();
    void setCC1101Preset(CC1101_PRESET preset);
    void loadPreset();
    void disableReceiver();
    void enableReceiverCustom();
    void setFrequency(float freq);
    void enableReceiver();
    void setSync(int sync);
    void setPTK(int ptk);
    void enableTransmit();
    void disableTransmit();
    void saveSignal();
    void signalanalyse();
    bool CheckReceived(void);
    void initrRaw();
    void sendRaw();
    void sendSamples(int samples[], int samplesLength);
    static void signalanalyseTask(void* pvParameters);
    void startSignalanalyseTask();
    void fskAnalyze();
    void sendByteSequence(const uint8_t sequence[], const uint16_t pulseWidth, const uint8_t messageLength);
   
    void enableScanner(float start, float stop);
 static recievedData receivedData;
private:
    size_t smoothcount;
    uint16_t samplesmooth[SAMPLE_SIZE];
    String rawString = "";
    int minsample = 15;

    String generateFilename(float frequency, int modulation, float bandwidth);
    String generateRandomString(int length);
   void decode(uint16_t *pulseTrain, size_t length);
   
};

#endif
