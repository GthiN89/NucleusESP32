#ifndef ISO14443ASTARGET_H
#define ISO14443ASTARGET_H

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include "globals.h"

class ISO14443ASTarget {
public:
    ISO14443ASTarget();
    void begin();
    void process();

private:
    // Adjust pins as needed


    
};

#endif
