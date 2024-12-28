#ifndef JAMMER_H
#define JAMMER_H

#include "EEPROM.h"
#include <RF24.h>
#include "globals.h"

// Pin definitions
#define CYD_MOSI 21   // Master Out Slave In
#define CYD_MISO 35   // Master In Slave Out
#define CYD_SCLK 22   // Serial Clock
#define RF24_CS 16
#define RF24_CE 17   // NRF24 CE

class Jammer {
private:
    RF24 radio;
    int bluetooth_jam_method;
    int drone_jam_method;
    int ble_jam_method;
    int wifi_jam_method;
    int zigbee_jam_method;
    int Separate_or_together;
    int misc_jam_method;

    // Radio configuration
    rf24_pa_dbm_e paLevel = RF24_PA_MAX;
    rf24_datarate_e dataRate = RF24_2MBPS;
    rf24_crclength_e crcLength = RF24_CRC_DISABLED;



   

public:


        Jammer();
    void setup();
    void loop();

     void initializeEEPROM();
    void loadSettings();
    void bluetooth_jam();
    void drone_jam();
    void ble_jam();
    void wifi_jam();
    void wifi_channel(int channel);
    void zigbee_jam();
    void misc_jam(int channel1, int channel2);
};

#endif