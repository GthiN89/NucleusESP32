#ifndef RADIO_RECEIVER_H
#define RADIO_RECEIVER_H

#include <RadioLib.h>
#include <Module.h>
#include "globals.h"
#include "SPI.h"
#include <rtl_433_ESP.h>

class RadioReceiver {
public:
    void setup();
    void loop();

private:
    SPISettings SetSPI;

    rtl_433_ESP rf;
};

#endif
