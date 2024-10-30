#ifndef C1101_H
#define C1101_H

#include "../../globals.h"
#define SAMPLE_SIZE 1024

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
 extern bool CC1101_TX;
 extern int sample[];
 extern int samplecount;
 extern float CC1101_FREQ;

class CC1101_CLASS {
public:



unsigned long samplesmooth[SAMPLE_SIZE];
  int smoothcount=0;
    String rawString = "";
    int BufferSize = 32;
    int pulseLenght;
    int CC1101_MODULATION = 2;
    float CC1101_DRATE = 3.79372;;
    float CC1101_RX_BW = 650.00;;
    float CC1101_DEVIATION = 47.60;
    int   CC1101_SYNC_MODE;
    int CC1101_PKT_FORMAT;
    
    long tempValue = 0;
    int tempBitLength = 0;
    int tempDelay = 0;
    int tempProtocol = 0;
    String rawdataString = "";


    void setCC1101Preset(CC1101_PRESET preset);
    void loadPreset();
    bool init();
    void showResultRecPlay();
    void disableReceiver();
    void setFrequency(float freq);
    void enableReceiver();
    void setSync(int sync);
    void setPTK(int ptk);
    String generateFilename(float frequency, int modulation, float bandwidth);
    String generateRandomString(int length);
    bool sendCapture();
    void enableTransmit();
    void disableTransmit();
    void saveSignal();
    void signalanalyse();
    static bool CheckReceived(void);
    void asciitohex(byte *ascii_ptr, byte *hex_ptr,int len);
    void initrRaw();
    void initRCSwitch();
    void disableRCSwitch();
    bool RCmethodResult();
    void ResetRCSwitch();
    bool initCC1101();
    void sendRaw();
   // bool saveToSD(); 
    void sendSamples(int samples[], int samplesLength);
    void sendBrute(int type);


private:

};

#endif 
