#ifndef SOUR_APPLE_H
#define SOUR_APPLE_H

#include <NimBLEDevice.h>

class sourApple {
public:
    void setup();
    void loop();

private:
    NimBLEAdvertisementData getOAdvertisementData();
};

#endif // SOUR_APPLE_H