#ifndef C1101_H
#define C1101_H

#include "../../globals.h"
#include "RCSwitch.h"
#include "ESPiLight.h"
#include "SPI.h"
#include <driver/timer.h>

#define SAMPLE_SIZE 4092
#define MAX_SIGNAL_LENGTH 10000000  

//---------------------------------------------------------------------------//
//-----------------------------Presets-Variables-----------------------------//
//---------------------------------------------------------------------------//
//-----------------------------Flags-----------------------------------------//
extern bool CC1101_is_initialized;
extern bool CC1101_recieve_is_running;
extern bool CC1101_transmit_is_running;
extern bool CC1101_is_idle;
extern bool CC1101_interupt_attached;
extern uint16_t sample[];
extern uint8_t samplecount;
extern bool startLow;
extern uint32_t actualFreq;
extern float strongestASKFreqs[4];
extern int strongestASKRSSI[4];
extern float strongestFSKFreqs[2];
extern int strongestFSKRSSI[2];

#include <vector>

struct Signal {
    std::vector<int64_t> samples;

    void addSample(int64_t sample) {
        samples.push_back(sample);
    }

    int64_t getSample(std::size_t index) const {
        return samples[index];
    }

    bool empty() const {
        return samples.empty();
    }

    void clear() {
        samples.clear();
    }

    std::size_t size() const {
        return samples.size();
    }
};

struct SignalCollection {
    std::vector<Signal> signals;
    std::vector<Signal> ABSsignals;

    Signal& getSignal(size_t i) {
        return signals.at(i);
    }

     Signal& getABSSignal(size_t i) {
        return ABSsignals.at(i);
    }

    void addSignal(const Signal& signal) {
        signals.push_back(signal);
    }

     void addABSSignal(const Signal& signal) {
        ABSsignals.push_back(signal);
    }


    std::size_t size() const {
        return signals.size();
    }

    bool empty() const {        
        return signals.empty();
    }

    void clear() {
        signals.clear();
        ABSsignals.clear();
    }
};



class CC1101_CLASS {
public:
    static SignalCollection allData;
    float CC1101_DRATE = 3.79372;
    float CC1101_RX_BW = 650.00;
    float CC1101_DEVIATION = 47.60;
    int CC1101_PKT_FORMAT = 0;
    int CC1101_SYNC = 2;
    float CC1101_FREQ = 433.92;
    int CC1101_MODULATION;
    static std::vector<int64_t> samplesToSend;

    struct ReceivedData {
        std::vector<int64_t> samples;
        std::vector<Signal> signals;
        volatile unsigned long lastReceiveTime = 0;
        volatile unsigned long sampleCount = 0;
        volatile unsigned long normalizedCount = 0;
        std::vector<uint16_t> pulseTrainVec;
        bool startstate;

        size_t size(){
          return  samples.size();
        }
    };

    static ReceivedData receivedData;

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
    void signalAnalyse();
    bool CheckReceived(void);
    void initRaw();
    void sendRaw();
    void sendSamples(int samples[], int samplesLength);
    static void signalAnalyseTask(void* pvParameters);
    void startSignalAnalyseTask();
    void fskAnalyze();
    void sendByteSequence(const uint8_t sequence[], const uint16_t pulseWidth, const uint8_t messageLength);
    void enableScanner(float start, float stop);

private:
    uint16_t spaceAvg = 0;
    size_t smoothcount;
    uint16_t sampleSmooth[SAMPLE_SIZE];
    String generateFilename(float frequency, int modulation, float bandwidth);
    String generateRandomString(int length);
    void decode(uint16_t* pulseTrain, size_t length);
    bool levelFlag;                         // Current GPIO level
    timer_idx_t timerIndex = TIMER_0;               // Timer index
};



#endif
