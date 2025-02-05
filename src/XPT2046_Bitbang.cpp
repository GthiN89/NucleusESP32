#include "XPT2046_Bitbang.h"

//#define _mosiPin 32
//#define _misoPin 39
//#define _clkPin  25
//#define _csPin   33
#define RERUN_CALIBRATE false
#define CMD_READ_Y  0x90 // Command for XPT2046 to read Y position
#define CMD_READ_X  0xD0 // Command for XPT2046 to read X position

XPT2046_Bitbang::XPT2046_Bitbang(uint8_t mosiPin, uint8_t misoPin, uint8_t clkPin, uint8_t csPin) : 
    _mosiPin(mosiPin), _misoPin(misoPin), _clkPin(clkPin), _csPin(csPin) {
    // other initializations, if required
}

void XPT2046_Bitbang::begin() {
    pinMode(_mosiPin, OUTPUT);
    pinMode(_misoPin, INPUT);
    pinMode(_clkPin, OUTPUT);
    pinMode(_csPin, OUTPUT);
    digitalWrite(_csPin, HIGH);
    digitalWrite(_clkPin, LOW);


}

int XPT2046_Bitbang::readSPI(byte command) {
    int result = 0;

    for (int i = 7; i >= 0; i--) {
        digitalWrite(_mosiPin, command & (1 << i));
        digitalWrite(_clkPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(_clkPin, LOW);
        delayMicroseconds(10);
    }

    for (int i = 11; i >= 0; i--) {
        digitalWrite(_clkPin, HIGH);
        delayMicroseconds(10);
        result |= (digitalRead(_misoPin) << i);
        digitalWrite(_clkPin, LOW);
        delayMicroseconds(10);
    }

    return result;
}

void XPT2046_Bitbang::calibrate() {
    Serial.println("Calibration starting...");
    Serial.println("Touch the top-left corner, hold it down until the next message...");
    delay(3000);
    digitalWrite(_csPin, LOW);
    cal.xMin = readSPI(CMD_READ_X);
    cal.yMin = readSPI(CMD_READ_Y);
    digitalWrite(_csPin, HIGH);

    Serial.println("Touch the bottom-right corner, hold it down until the next message...");
    delay(3000);
    digitalWrite(_csPin, LOW);
    cal.xMax = readSPI(CMD_READ_X);
    cal.yMax = readSPI(CMD_READ_Y);
    digitalWrite(_csPin, HIGH);

    Serial.println("Calibration done!");
}




void XPT2046_Bitbang::setCalibration(int xMin, int yMin, int xMax, int yMax) {
    cal.xMin = xMin;
    cal.yMin = yMin;
    cal.xMax = xMax;
    cal.yMax = yMax;
}

Point XPT2046_Bitbang::getTouch() {
    digitalWrite(_csPin, LOW);
    int x = readSPI(CMD_READ_X);
    int y = readSPI(CMD_READ_Y);
    digitalWrite(_csPin, HIGH);
    x = map(x, cal.xMin, cal.xMax, 0, TFT_WIDTH);
    y = map(y, cal.yMin, cal.yMax, 0, TFT_HEIGHT);
    if (x > TFT_WIDTH){x = TFT_WIDTH;}
    if (x < 0){x = 0;}
    if (y > TFT_HEIGHT){y = TFT_HEIGHT;}
    if (y < 0){y = 0;}
    return Point{x, y};
}
