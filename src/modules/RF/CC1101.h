#ifndef C1101_H
#define C1101_H

#include "../../globals.h"
#include "RCSwitch.h"
#include "ESPiLight.h"
#include "SPI.h"
#include <driver/timer.h>
//decoders/encoders
#include "protocols/HormannProtocol.h" 
#include "protocols/CameProtocol.h" 
#include "protocols/NiceFloProtocol.h"
#include "protocols/AnsonicProtocol.h"
#include "protocols/Smc5326Protocol.h"
#include "protocols/math.h"


#define SAMPLE_SIZE 768

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

struct Histogram {
    std::vector<int> bins;  // Frequency count for each bin.
    int binSize;            // The resolution of each bin.
    int minVal;             // Minimum pulse value in the window.
};




struct CC1101TH {
    std::map<int, uint8_t> valueMap = {
        {-70, 0x88},
        {-60, 0x90},
        {-50, 0x98},
        {-40, 0xA0},
        {-30, 0xA8},
        {-20, 0xB0},
        {-10, 0xB8},
        {  0, 0xC0},
        { 10, 0xC8},
        { 20, 0xD0},
        { 30, 0xD8},
        { 40, 0xE0},
        { 50, 0xE8},
        { 60, 0xF0},
        { 70, 0xF8},
        { 80, 0xFF}
    };

    uint8_t getRegValue(int input) const {
        auto it = valueMap.find(input);
        if (it != valueMap.end()) {
            return it->second;
        }
        return 0; // Default or error value
    }
};





class CC1101_CLASS {
public:
    static SignalCollection allData;
    float CC1101_DRATE = 115.051;
    float CC1101_RX_BW = 650.00;
    float CC1101_DEVIATION = 47.60;
    int CC1101_PKT_FORMAT = 0;
    int CC1101_SYNC = 2;
    float CC1101_FREQ = 433.92;
    int CC1101_MODULATION;


    struct ReceivedData {
        std::vector<int64_t> samples;
        std::vector<int64_t> filtered;
        std::vector<Signal> signals;
        volatile unsigned long lastReceiveTime = 0;
        volatile unsigned long sampleCount = 0;
        volatile unsigned long normalizedCount = 0;
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
    void enableRCSwitch();
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
    void sendSamples(int timings[], int timingsLength, bool levelFlag);
    void fskAnalyze();
    void sendByteSequence(const uint8_t sequence[], const uint16_t pulseWidth, const uint8_t messageLength);
    void enableScanner(float start, float stop);
    void emptyReceive();
    void filterSignal();
    bool decode();
    bool checkReversed(int64_t big);
    void reverseLogicState();
    void filterAll(); 

private:
    //decoder instances
    HormannProtocol hormannProtocol;
    CameProtocol cameProtocol;
    AnsonicProtocol ansonicProtocol;
    NiceFloProtocol niceFloProtocol;
    Smc5326Protocol  smc5326Protocol;



    uint16_t spaceAvg = 0;
    size_t smoothcount;
    uint16_t sampleSmooth[SAMPLE_SIZE];
    String generateFilename(float frequency, int modulation, float bandwidth);
    String generateRandomString(int length);
    Histogram buildHistogram(const std::vector<int>& window, int binSize, int& maxVal);
    std::vector<int> detectPeaks(const Histogram& hist);
    bool levelFlag;                         // Current GPIO level
    timer_idx_t timerIndex = TIMER_0;               // Timer index
    std::vector<uint16_t> pulses;
    int DURATION_DIFF1(int x, int y); 

};



#endif
