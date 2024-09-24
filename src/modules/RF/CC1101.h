#ifndef C1101_H
#define C1101_H

#include "globals.h"

   //---------------------------------------------------------------------------//
  //-----------------------------Presets-Variables-----------------------------//
 //---------------------------------------------------------------------------//


class CC1101_CLASS {
public:
    int CC1101_MODULATION;
    float CC1101_DRATE;
    float CC1101_RX_BW;
    float CC1101_DEVIATION;
    int   CC1101_SYNC_MODE;
    int CC1101_PKT_FORMAT;
    void setCC1101Preset(CC1101_PRESET preset);
    void loadPreset();
    bool init();
    void showResultRecPlay();
    void disableReceiver();
    bool captureLoop();
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

private:

};

#endif 
