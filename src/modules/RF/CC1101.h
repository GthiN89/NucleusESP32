#ifndef C1101_H
#define C1101_H

#include "../../globals.h"
#define SAMPLE_SIZE 512



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
 extern int sample[];
 extern int samplecount;
 extern bool startLow;


class CC1101_CLASS {
public:
    float CC1101_DRATE = 3.79372;;
    float CC1101_RX_BW = 650.00;;
    float CC1101_DEVIATION = 47.60;
    int CC1101_PKT_FORMAT = 0;
    int CC1101_SYNC = 2;
    float CC1101_FREQ = 433.92;
    
    bool init();
    RCSwitch getRCSwitch();
    void setCC1101Preset(CC1101_PRESET preset);
    void loadPreset();
    void enableRCSwitch();
    void disableReceiver();
    void setFrequency(float freq);
    void enableReceiver();
    void setSync(int sync);
    void setPTK(int ptk);
    String generateFilename(float frequency, int modulation, float bandwidth);
    String generateRandomString(int length);
    void enableTransmit();
    void disableTransmit();
    void saveSignal();
    void signalanalyse();
    bool CheckReceived(void);
    void initrRaw();
    void sendRaw();
    void sendSamples(int samples[], int samplesLength);
    void sendFromFile();
    void sendBrute(int type);
private:
    SPIClass  CC1101SPI;;
    int smoothcount;
    unsigned long samplesmooth[SAMPLE_SIZE];
    String rawString = "";
    int minsample = 15;
    
};

#endif 
