#include "XPT2046_Bitbang.h"
#include "main.h"

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

    if((!SPIFFS.begin(true)) || (!loadCalibration() || (RERUN_CALIBRATE))) {
//        calibrateGraphic();
 //       saveCalibration();
    }
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
    Serial.println(F("Calibration starting..."));
    Serial.println(F("Touch the top-left corner, hold it down until the next message..."));
    delay(3000);
    digitalWrite(_csPin, LOW);
    cal.xMin = readSPI(CMD_READ_X);
    cal.yMin = readSPI(CMD_READ_Y);
    digitalWrite(_csPin, HIGH);

    Serial.println(F("Touch the bottom-right corner, hold it down until the next message..."));
    delay(3000);
    digitalWrite(_csPin, LOW);
    cal.xMax = readSPI(CMD_READ_X);
    cal.yMax = readSPI(CMD_READ_Y);
    digitalWrite(_csPin, HIGH);

    Serial.println(F("Calibration done!"));
     Serial.println(F("xMin"));
    Serial.println(cal.xMin);
    Serial.println(F("xMax"));
    Serial.println(cal.xMax);
    Serial.println(F("yMin"));
    Serial.println(cal.yMin);
    Serial.println(F("yMax"));
    Serial.println(cal.yMax);
}

bool XPT2046_Bitbang::loadCalibration() {
    preferences.begin("calib", true);  // Open NVS with read access
    bool isCalibrated = preferences.getBool("calibrated", false);  // Default to false if not found

    if (isCalibrated && preferences.isKey("xMin") && preferences.isKey("yMin") && preferences.isKey("xMax") && preferences.isKey("yMax")) {
        cal.xMin = preferences.getInt("xMin");
        cal.yMin = preferences.getInt("yMin");
        cal.xMax = preferences.getInt("xMax");
        cal.yMax = preferences.getInt("yMax");
        preferences.end();  // Close NVS access
        Serial.println(F("Calibration data loaded from NVS."));
        return true;
    } else {
        preferences.end();  // Close NVS access
        Serial.println(F("No calibration data found in NVS."));
        return false;
    }
}
void XPT2046_Bitbang::saveCalibration() {
    preferences.begin("calib", false);  // Open the NVS namespace "calib"
    preferences.putInt("xMin", cal.xMin);
    preferences.putInt("yMin", cal.yMin);
    preferences.putInt("xMax", cal.xMax);
    preferences.putInt("yMax", cal.yMax);
    preferences.putBool("calibrated", true);  // Set calibration flag
    preferences.end();  // Close the preferences
    Serial.println(F("Calibration data saved to NVS."));
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

Calibration* XPT2046_Bitbang::getCalPointer() {
     return &cal;
}
