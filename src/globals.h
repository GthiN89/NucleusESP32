#ifndef GLOBALS_H
#define GLOBALS_H

#include "SPI.h"
#include "Arduino.h"
#include "ELECHOUSE_CC1101_SRC_DRV.h"

#include <map>
#include <string>
#include <vector>

    enum State {
        IDLE,
        BLUETOOTH_JAM,
        DRONE_JAM,
        BLE_JAM,
        WIFI_JAM,
        ZIGBEE_JAM,
        MISC_JAM
    };

   void setState(State newState);

extern State currentState;

extern std::vector<int64_t> samplesToSend;

 enum CC1101_PRESET {
     AM650,
     AM270,
     FM238,
     FM476,
     FM95,
     FSK12k,
     FM15k,
     FSK25k,
     FSK31k,
     PAGER,
     HND1,
     HND2,
     CUSTOM
 };

extern CC1101_PRESET  C1101preset;
 enum EncoderStep {
        EncoderStepIddle,
        EncoderStepStart,
        EncodeStepStartBit,
        EncoderStepDurations,
        EncoderStepReady //to relase
    };
extern EncoderStep encoderState;


const SPISettings spiSettings = SPISettings(SPI_CLOCK_DIV4, MSBFIRST, SPI_MODE0); // May have to be set if hardware is not fully compatible to Arduino specifications.


#include <lvgl.h>
#include <vector>
#include <stdio.h>
#include <string.h>
#include "SPI.h"
#include "Arduino.h"
   //---------------------------------------------------------------------------//
  //-----------------------------SETINGS---------------------------------------//
 //---------------------------------------------------------------------------//
#define CYD_MOSI 21   // Master Out Slave In
#define CYD_MISO 35  // Master In Slave Out
#define CYD_SCLK 22  // Serial Clock
#define RFID_CS -1
#define RF24_CS 16

// gd0 17
// sck 22
// miso 35
// CKN 27
// mosi 21
// GD2 - 4

#define IRQ_PIN CCGDO2A
#define RFID_RST -1
#define RF24_CE  CCGDO0A



// Pin configuration for CC1101 nfc CC-27 NRF24 CC-16
#define CC1101_CS 27    // Chip Select
#define CC1101_MOSI CYD_MOSI   // Master Out Slave In
#define CC1101_MISO CYD_MISO  // Master In Slave Out
#define CC1101_SCLK CYD_SCLK  // Serial Clock

#define CCGDO0A GPIO_NUM_17      // GDO0-NFC IRQ, NF IRQ - input only
#define CCGDO2A GPIO_NUM_4      // GDO2 -rst - input output

#define CC1101_CCGDO0A CCGDO0A      // GDO0
#define CC1101_CCGDO2A CCGDO2A      // GDO2 -rst NF CE

#define CMD_READ_Y  0x90 // Command for XPT2046 to read Y position
#define CMD_READ_X  0xD0 // Command for XPT2046 to read X position

// Pin definitions for touchscreen and SD card
#define MOSI_PIN 32
#define MISO_PIN 39
#define CLK_PIN  25
#define CS_PIN   33

#define SD_CS 5 // SD card CS pin


//ir tx = pin 26 rx 34


class Jammer; // Forward declaration if needed

extern bool teslaSucessFlag;

extern Jammer jammer;



   //---------------------------------------------------------------------------//
  //-----------------------------ENUMBS----------------------------------------//
 //---------------------------------------------------------------------------//
// 






enum C1101State
{
  STATE_IDLE,
  STATE_GENERATOR,
  STATE_ANALYZER,
  STATE_RCSWITCH,
  STATE_SCANNER,
  STATE_CAPTURE,
  STATE_PLAYBACK,
  STATE_TESLA_US,
  STATE_TESLA_EU,
  STATE_SEND_FLIPPER,
  STATE_BRUTE,
  STATE_DETECT,
};
extern uint8_t C1101CurrentState;

enum BruteProtocol {
  CAME_12bit,
  NICE_12bit,
  BRUTE_IDLE,
};

extern uint8_t BruteCurrentState;



enum RunnigModule
{
  MODULE_NONE,
  MODULE_CC1101,
  MODULE_IR
};

// Current State
 extern uint8_t runningModule;



extern bool C1101LoadPreset;
//extern bool receiverEnabled;
extern float CC1101_MHZ;


enum BTState
{
  STATE_SOUR_APPLE_IDLE,
  STATE_SOUR_APPLE,
  STATE_BTSPAM,
  STATE_BT_IDDLE,
};




//////////////////////////////////////////////////////////////////////////
//FLAGS
///////////////////
extern bool CC1101_init; 

// Function declarations
typedef void (*TouchCallback)();

CC1101_PRESET convert_str_to_enum(const char * selected_str);
const char* presetToString(CC1101_PRESET preset);
const uint8_t* presetTobyteArray(CC1101_PRESET preset);

extern char* current_dir;
//extern char* selected_file;
//extern lv_obj_t* selected_label;
extern lv_obj_t* selected_btn;

extern uint16_t bruteCounter;


//file browser globals

extern char* current_dir;
extern char* selected_file;

const uint8_t AM270_regs[] = {
    // https://e2e.ti.com/support/wireless-connectivity/sub-1-ghz-group/sub-1-ghz/f/sub-1-ghz-forum/382066/cc1101---don-t-know-the-correct-registers-configuration

    /* GPIO GD0 */
    CC1101_IOCFG0,
    0x0D, // GD0 as async serial data output/input

    /* FIFO and internals */
    CC1101_FIFOTHR,
    0x47, // The only important bit is ADC_RETENTION, FIFO Tx=33 Rx=32

    /* Packet engine */
    CC1101_PKTCTRL0,
    0x32, // Async, continious, no whitening

    /* Frequency Synthesizer Control */
    CC1101_FSCTRL1,
    0x06, // IF = (26*10^6) / (2^10) * 0x06 = 152343.75Hz

    // Modem Configuration
    CC1101_MDMCFG0,
    0x00, // Channel spacing is 25kHz
    CC1101_MDMCFG1,
    0x00, // Channel spacing is 25kHz
    CC1101_MDMCFG2,
    0x30, // Format ASK/OOK, No preamble/sync
    CC1101_MDMCFG3,
    0x32, // Data rate is 3.79372 kBaud
    CC1101_MDMCFG4,
    0x67, // Rx BW filter is 270.833333kHz

    /* Main Radio Control State Machine */
    CC1101_MCSM0,
    0x18, // Autocalibrate on idle-to-rx/tx, PO_TIMEOUT is 64 cycles(149-155us)

    /* Frequency Offset Compensation Configuration */
    CC1101_FOCCFG,
    0x18, // no frequency offset compensation, POST_K same as PRE_K, PRE_K is 4K, GATE is off

    /* Automatic Gain Control */
    CC1101_AGCCTRL0,
    0x40, // 01 - Low hysteresis, small asymmetric dead zone, medium gain; 00 - 8 samples agc; 00 - Normal AGC, 00 - 4dB boundary
    CC1101_AGCCTRL1,
    0x00, // 0; 0 - LNA 2 gain is decreased to minimum before decreasing LNA gain; 00 - Relative carrier sense threshold disabled; 0000 - RSSI to MAIN_TARGET
    CC1101_AGCCTRL2,
    0x03, // 00 - DVGA all; 000 - MAX LNA+LNA2; 011 - MAIN_TARGET 24 dB

    /* Wake on radio and timeouts control */
    CC1101_WORCTRL,
    0xFB, // WOR_RES is 2^15 periods (0.91 - 0.94 s) 16.5 - 17.2 hours

    /* Frontend configuration */
    CC1101_FREND0,
    0x11, // Adjusts current TX LO buffer + high is PATABLE[1]
    CC1101_FREND1,
    0xB6, //

    /* End load reg */
    0,
    0,

    //ook_async_patable[8]
    0x00,
    0xC0, // 12dBm 0xC0, 10dBm 0xC5, 7dBm 0xCD, 5dBm 0x86, 0dBm 0x50, -6dBm 0x37, -10dBm 0x26, -15dBm 0x1D, -20dBm 0x17, -30dBm 0x03
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
};

const uint8_t AM650_regs[] = {
    // https://e2e.ti.com/support/wireless-connectivity/sub-1-ghz-group/sub-1-ghz/f/sub-1-ghz-forum/382066/cc1101---don-t-know-the-correct-registers-configuration

    /* GPIO GD0 */
    CC1101_IOCFG0,
    0x0D, // GD0 as async serial data output/input

    /* FIFO and internals */
    CC1101_FIFOTHR,
    0x07, // The only important bit is ADC_RETENTION

    /* Packet engine */
    CC1101_PKTCTRL0,
    0x32, // Async, continious, no whitening

    /* Frequency Synthesizer Control */
    CC1101_FSCTRL1,
    0x06, // IF = (26*10^6) / (2^10) * 0x06 = 152343.75Hz

    // Modem Configuration
    CC1101_MDMCFG0,
    0x00, // Channel spacing is 25kHz
    CC1101_MDMCFG1,
    0x00, // Channel spacing is 25kHz
    CC1101_MDMCFG2,
    0x30, // Format ASK/OOK, No preamble/sync
    CC1101_MDMCFG3,
    0x32, // Data rate is 3.79372 kBaud
    CC1101_MDMCFG4,
    0x17, // Rx BW filter is 650.000kHz

    /* Main Radio Control State Machine */
    CC1101_MCSM0,
    0x18, // Autocalibrate on idle-to-rx/tx, PO_TIMEOUT is 64 cycles(149-155us)

    /* Frequency Offset Compensation Configuration */
    CC1101_FOCCFG,
    0x18, // no frequency offset compensation, POST_K same as PRE_K, PRE_K is 4K, GATE is off

    /* Automatic Gain Control */
    // CC1101_AGCTRL0,0x40, // 01 - Low hysteresis, small asymmetric dead zone, medium gain; 00 - 8 samples agc; 00 - Normal AGC, 00 - 4dB boundary
    // CC1101_AGCTRL1,0x00, // 0; 0 - LNA 2 gain is decreased to minimum before decreasing LNA gain; 00 - Relative carrier sense threshold disabled; 0000 - RSSI to MAIN_TARGET
    // CC1101_AGCCTRL2, 0x03, // 00 - DVGA all; 000 - MAX LNA+LNA2; 011 - MAIN_TARGET 24 dB
    //MAGN_TARGET for RX filter BW =< 100 kHz is 0x3. For higher RX filter BW's MAGN_TARGET is 0x7.
    CC1101_AGCCTRL0,
    0x91, // 10 - Medium hysteresis, medium asymmetric dead zone, medium gain ; 01 - 16 samples agc; 00 - Normal AGC, 01 - 8dB boundary
    CC1101_AGCCTRL1,
    0x0, // 0; 0 - LNA 2 gain is decreased to minimum before decreasing LNA gain; 00 - Relative carrier sense threshold disabled; 0000 - RSSI to MAIN_TARGET
    CC1101_AGCCTRL2,
    0x07, // 00 - DVGA all; 000 - MAX LNA+LNA2; 111 - MAIN_TARGET 42 dB

    /* Wake on radio and timeouts control */
    CC1101_WORCTRL,
    0xFB, // WOR_RES is 2^15 periods (0.91 - 0.94 s) 16.5 - 17.2 hours

    /* Frontend configuration */
    CC1101_FREND0,
    0x11, // Adjusts current TX LO buffer + high is PATABLE[1]
    CC1101_FREND1,
    0xB6, //

    /* End load reg */
    0,
    0,

    //ook_async_patable[8]
    0x00,
    0xC0, // 12dBm 0xC0, 10dBm 0xC5, 7dBm 0xCD, 5dBm 0x86, 0dBm 0x50, -6dBm 0x37, -10dBm 0x26, -15dBm 0x1D, -20dBm 0x17, -30dBm 0x03
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
};

const uint8_t FM238_regs[] ={

    /* GPIO GD0 */
    CC1101_IOCFG0,
    0x0D, // GD0 as async serial data output/input

    /* Frequency Synthesizer Control */
    CC1101_FSCTRL1,
    0x06, // IF = (26*10^6) / (2^10) * 0x06 = 152343.75Hz

    /* Packet engine */
    CC1101_PKTCTRL0,
    0x32, // Async, continious, no whitening
    CC1101_PKTCTRL1,
    0x04,

    // // Modem Configuration
    CC1101_MDMCFG0,
    0x00,
    CC1101_MDMCFG1,
    0x02,
    CC1101_MDMCFG2,
    0x04, // Format 2-FSK/FM, No preamble/sync, Disable (current optimized)
    CC1101_MDMCFG3,
    0x83, // Data rate is 4.79794 kBaud
    CC1101_MDMCFG4,
    0x67, //Rx BW filter is 270.833333 kHz
    CC1101_DEVIATN,
    0x04, //Deviation 2.380371 kHz

    /* Main Radio Control State Machine */
    CC1101_MCSM0,
    0x18, // Autocalibrate on idle-to-rx/tx, PO_TIMEOUT is 64 cycles(149-155us)

    /* Frequency Offset Compensation Configuration */
    CC1101_FOCCFG,
    0x16, // no frequency offset compensation, POST_K same as PRE_K, PRE_K is 4K, GATE is off

    /* Automatic Gain Control */
    CC1101_AGCCTRL0,
    0x91, //10 - Medium hysteresis, medium asymmetric dead zone, medium gain ; 01 - 16 samples agc; 00 - Normal AGC, 01 - 8dB boundary
    CC1101_AGCCTRL1,
    0x00, // 0; 0 - LNA 2 gain is decreased to minimum before decreasing LNA gain; 00 - Relative carrier sense threshold disabled; 0000 - RSSI to MAIN_TARGET
    CC1101_AGCCTRL2,
    0x07, // 00 - DVGA all; 000 - MAX LNA+LNA2; 111 - MAIN_TARGET 42 dB

    /* Wake on radio and timeouts control */
    CC1101_WORCTRL,
    0xFB, // WOR_RES is 2^15 periods (0.91 - 0.94 s) 16.5 - 17.2 hours

    /* Frontend configuration */
    CC1101_FREND0,
    0x10, // Adjusts current TX LO buffer
    CC1101_FREND1,
    0x56,

    /* End load reg */
    0,
    0,

    // 2fsk_async_patable[8]
    0xC0, // 10dBm 0xC0, 7dBm 0xC8, 5dBm 0x84, 0dBm 0x60, -10dBm 0x34, -15dBm 0x1D, -20dBm 0x0E, -30dBm 0x12
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
};


const uint8_t FM476_regs[] = {

    /* GPIO GD0 */
    CC1101_IOCFG0,
    0x0D, // GD0 as async serial data output/input

    /* Frequency Synthesizer Control */
    CC1101_FSCTRL1,
    0x06, // IF = (26*10^6) / (2^10) * 0x06 = 152343.75Hz

    /* Packet engine */
    CC1101_PKTCTRL0,
    0x32, // Async, continious, no whitening
    CC1101_PKTCTRL1,
    0x04,

    // // Modem Configuration
    CC1101_MDMCFG0,
    0x00,
    CC1101_MDMCFG1,
    0x02,
    CC1101_MDMCFG2,
    0x04, // Format 2-FSK/FM, No preamble/sync, Disable (current optimized)
    CC1101_MDMCFG3,
    0x83, // Data rate is 4.79794 kBaud
    CC1101_MDMCFG4,
    0x67, //Rx BW filter is 270.833333 kHz
    CC1101_DEVIATN,
    0x47, //Deviation 47.60742 kHz

    /* Main Radio Control State Machine */
    CC1101_MCSM0,
    0x18, // Autocalibrate on idle-to-rx/tx, PO_TIMEOUT is 64 cycles(149-155us)

    /* Frequency Offset Compensation Configuration */
    CC1101_FOCCFG,
    0x16, // no frequency offset compensation, POST_K same as PRE_K, PRE_K is 4K, GATE is off

    /* Automatic Gain Control */
    CC1101_AGCCTRL0,
    0x91, //10 - Medium hysteresis, medium asymmetric dead zone, medium gain ; 01 - 16 samples agc; 00 - Normal AGC, 01 - 8dB boundary
    CC1101_AGCCTRL1,
    0x00, // 0; 0 - LNA 2 gain is decreased to minimum before decreasing LNA gain; 00 - Relative carrier sense threshold disabled; 0000 - RSSI to MAIN_TARGET
    CC1101_AGCCTRL2,
    0x07, // 00 - DVGA all; 000 - MAX LNA+LNA2; 111 - MAIN_TARGET 42 dB

    /* Wake on radio and timeouts control */
    CC1101_WORCTRL,
    0xFB, // WOR_RES is 2^15 periods (0.91 - 0.94 s) 16.5 - 17.2 hours

    /* Frontend configuration */
    CC1101_FREND0,
    0x10, // Adjusts current TX LO buffer
    CC1101_FREND1,
    0x56,

    /* End load reg */
    0,
    0,

    // 2fsk_async_patable[8]
    0xC0, // 10dBm 0xC0, 7dBm 0xC8, 5dBm 0x84, 0dBm 0x60, -10dBm 0x34, -15dBm 0x1D, -20dBm 0x0E, -30dBm 0x12
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
};


const uint8_t FM95_regs[] = {0x02, 0x0D, 0x03, 0x07, 0x08, 0x32, 0x0B, 0x06, 0x14, 0x00, 0x13, 0x00, 0x12, 0x30, 0x11,
                                                                      0x32, 0x10, 0x17, 0x18, 0x18, 0x19, 0x18, 0x1D, 0x91, 0x1C, 0x00, 0x1B, 0x07, 0x20, 0xFB,
                                                                      0x22, 0x11, 0x21, 0xB6, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const uint8_t FM15k_regs[] = {0x02, 0x0D, 0x03, 0x07, 0x08, 0x32, 0x0B, 0x06, 0x14, 0x00, 0x13, 0x00, 0x12, 0x30, 0x11,
                                                                      0x32, 0x10, 0x17, 0x18, 0x18, 0x19, 0x18, 0x1D, 0x91, 0x1C, 0x00, 0x1B, 0x07, 0x20, 0xFB,
                                                                      0x22, 0x11, 0x21, 0xB6, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const uint8_t HONDA1[] =    {0x02, 0x0D, 0x0B, 0x06, 0x08, 0x32, 0x07, 0x04, 0x14, 0x00, 0x13, 0x02, 0x12, 0x04, 0x11, 0x36,
                                                                       0x10, 0x69, 0x15, 0x32, 0x18, 0x18, 0x19, 0x16, 0x1D, 0x91, 0x1C, 0x00, 0x1B, 0x07, 0x20,
                                                                       0xFB, 0x22, 0x10, 0x21, 0x56, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,};

const uint8_t HONDA2[] =  {0x02, 0x0D, 0x0B, 0x06, 0x08, 0x32, 0x07, 0x04, 0x14, 0x00, 0x13, 0x02, 0x12, 0x07, 0x11, 0x36, 0x10, 0xE9, 0x15, 0x32, 0x18, 0x18, 0x19, 0x16, 0x1D, 0x92, 0x1C, 0x40, 0x1B, 0x03, 0x20, 0xFB, 0x22, 0x10, 0x21, 0x56, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};


#endif // GLOBALS_H
