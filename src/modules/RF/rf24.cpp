#include "rf24.h"

Jammer::Jammer() : radio(RF24(RF24_CS, RF24_CE)) {}

void Jammer::initializeEEPROM() {
    EEPROM.begin(512);
    for (int i = 0; i < 7; ++i) {
        if (EEPROM.read(i) == 255) {
            EEPROM.write(i, 0);
            EEPROM.commit();
        }
    }
}

void Jammer::loadSettings() {
    bluetooth_jam_method = EEPROM.read(0);
    drone_jam_method = EEPROM.read(1);
    ble_jam_method = EEPROM.read(2);
    wifi_jam_method = EEPROM.read(3);
    Separate_or_together = EEPROM.read(4);
    zigbee_jam_method = EEPROM.read(5);
    misc_jam_method = EEPROM.read(6);
}

void Jammer::setup() {
    initializeEEPROM();
    loadSettings();
    radio.begin();
    radio.setAutoAck(false);
    radio.stopListening();
    radio.setPALevel(paLevel);
    radio.setDataRate(dataRate);
    radio.setCRCLength(crcLength);
}



void Jammer::loop() {
    switch (currentState) {
        case IDLE:
            // Do nothing
            break;
        case BLUETOOTH_JAM:
            bluetooth_jam();
            break;
        case DRONE_JAM:
            drone_jam();
            break;
        case BLE_JAM:
            ble_jam();
            break;
        case WIFI_JAM:
            wifi_jam();
            break;
        case ZIGBEE_JAM:
            zigbee_jam();
            break;
        case MISC_JAM:
            misc_jam(10, 20); // Example channels
            break;
    }
}

void Jammer::bluetooth_jam() {
    while (currentState == BLUETOOTH_JAM) {
        if (Separate_or_together == 0) {
            if (bluetooth_jam_method == 0) {
                for (int i = 0; i < 22; i++) {
                    radio.setChannel(i);
                }
            } else if (bluetooth_jam_method == 1) {
                radio.setChannel(random(40));
            } else if (bluetooth_jam_method == 2) {
                for (int i = 0; i < 80; i++) {
                    radio.setChannel(i);
                }
            }
        } else if (Separate_or_together == 1) {
            int random_channel = random(80);
            radio.setChannel(random_channel);
        }
    }
}

void Jammer::drone_jam() {
    while (currentState == DRONE_JAM) {
        if (Separate_or_together == 0) {
            if (drone_jam_method == 0) {
                radio.setChannel(random(64));
            } else if (drone_jam_method == 1) {
                for (int i = 0; i < 125; i++) {
                    radio.setChannel(i);
                }
            }
        } else if (Separate_or_together == 1) {
            int random_channel = random(125);
            radio.setChannel(random_channel);
        }
    }
}

void Jammer::ble_jam() {
    const byte ble_channels[] = {2, 26, 80};
    while (currentState == BLE_JAM) {
        for (int i = 0; i < 3; i++) {
            radio.setChannel(ble_channels[i]);
        }
    }
}

void Jammer::wifi_jam() {
    const char jam_text[] = "xxxxxxxxxxxxxxxx";
    while (currentState == WIFI_JAM) {
        for (int channel = 0; channel < 13; channel++) {
            for (int i = (channel * 5) + 1; i < (channel * 5) + 23; i++) {
                radio.setChannel(i);
                radio.writeFast(&jam_text, sizeof(jam_text));
            }
        }
    }
}

void Jammer::wifi_channel(int channel) {
    const char jam_text[] = "xxxxxxxxxxxxxxxx";
    while (currentState == WIFI_JAM) {
        for (int i = (channel * 5) + 1; i < (channel * 5) + 23; i++) {
            radio.setChannel(i);
            radio.writeFast(&jam_text, sizeof(jam_text));
        }
    }
}

void Jammer::zigbee_jam() {
    const char jam_text[] = "xxxxxxxxxxxxxxxx";
    while (currentState == ZIGBEE_JAM) {
        for (int channel = 11; channel < 27; channel++) {
            for (int i = 5 + 5 * (channel - 11); i < (5 + 5 * (channel - 11)) + 6; i++) {
                radio.setChannel(i);
                radio.writeFast(&jam_text, sizeof(jam_text));
            }
        }
    }
}

void Jammer::misc_jam(int channel1, int channel2) {
    const char jam_text[] = "xxxxxxxxxxxxxxxx";
    while (currentState == MISC_JAM) {
        for (int i = 0; i <= channel2 - channel1; i++) {
            radio.setChannel(channel1 + i);
            radio.writeFast(&jam_text, sizeof(jam_text));
        }
    }
}
