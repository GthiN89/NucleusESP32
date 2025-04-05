#ifndef C1101_H
#define C1101_H

#include "../../globals.h"
#include "RCSwitch.h"

#include "SPI.h"
#include <driver/timer.h>
//decoders/encoders
#include "protocols/HormannProtocol.h" 
#include "protocols/CameProtocol.h" 
#include "protocols/NiceFloProtocol.h"
#include "protocols/AnsonicProtocol.h"
#include "protocols/Smc5326Protocol.h"
#include "protocols/Holtek_HT12xProtocol.h"
#include "protocols/kia.hpp"
#include "protocols/KeeLoqProtocol.hpp"
//#include "protocols/TPMSGenericData.h"

#define SAMPLE_SIZE 2048
#define MAX_SIGNAL_LENGTH 10000000  
#define TE_MIN_COUNT   5        // Minimum number of high pulses required to calculate TE
#define GAP_MULTIPLIER 10       // A low pulse longer than GAP_MULTIPLIER * TE is considered a gap
const float BIN_RAW_GAP_MULTIPLIER = 10.0;  // A low pulse longer than (TE * GAP_MULTIPLIER) is considered a gap
const uint16_t BIN_RAW_TE_MIN_COUNT = 5;  // Minimum number of high pulses to compute TE

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

enum RFProtocol {
    CAME,
    NICE,
    ANSONIC,
    HOLTEK,
    LINEAR,
    SMC5326
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
        std::vector<Signal> signals;
        std::vector<int64_t> filtered;
        std::vector<int64_t> filtered4;
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
    void handleSignal();
    bool CheckReceived(void);
    void initRaw();
    void sendRaw();
    void sendSamples(int timings[], int timingsLength, bool levelFlag);
    static void signalAnalyseTask(void* pvParameters);
    void startSignalAnalyseTask();
    void fskAnalyze();
    void sendByteSequence(const uint8_t sequence[], const uint16_t pulseWidth, const uint8_t messageLength);
    void enableScanner(float start, float stop);
    void emptyReceive();
    void filterSignal();
    bool decode();
    bool checkReversed(int64_t big);
    void reverseLogicState();
    void filterAll(); 
    void sendEncoded(RFProtocol protocol, float frequency, int16_t bitLenght, int8_t repeats, int64_t code);

    void SaveToSD();
private:
    //decoder instances
    HormannProtocol hormannProtocol;
    CameProtocol cameProtocol;
    AnsonicProtocol ansonicProtocol;
    HoltekProtocol holtekProtocol;
    NiceFloProtocol niceFloProtocol;
    SMC5326Protocol  smc5326Protocol;
    KiaProtocol kiaProtocol;
    KeeLoqProtocolDecoder keeloqDecoder;
    //TPMSProtocolDecoder tpmsDecoder;

    



    String generateFilename(float frequency, int modulation, float bandwidth);
    String generateRandomString(int length);
   

    bool levelFlag;                         // Current GPIO level
    timer_idx_t timerIndex = TIMER_0;               // Timer index
    std::vector<uint64_t> pulses;
   
};



#endif
