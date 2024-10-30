#ifndef FLIPPER_CONFIGS_H
#define FLIPPER_CONFIGS_H

#include <stdint.h>
#define CC1101_IOCFG2       0x00        // GDO2 output pin configuration
#define CC1101_IOCFG1       0x01        // GDO1 output pin configuration
#define CC1101_IOCFG0       0x02        // GDO0 output pin configuration
#define CC1101_FIFOTHR      0x03        // RX FIFO and TX FIFO thresholds
#define CC1101_SYNC1        0x04        // Sync word, high INT8U
#define CC1101_SYNC0        0x05        // Sync word, low INT8U
#define CC1101_PKTLEN       0x06        // Packet length
#define CC1101_PKTCTRL1     0x07        // Packet automation control
#define CC1101_PKTCTRL0     0x08        // Packet automation control
#define CC1101_ADDR         0x09        // Device address
#define CC1101_CHANNR       0x0A        // Channel number
#define CC1101_FSCTRL1      0x0B        // Frequency synthesizer control
#define CC1101_FSCTRL0      0x0C        // Frequency synthesizer control
#define CC1101_FREQ2        0x0D        // Frequency control word, high INT8U
#define CC1101_FREQ1        0x0E        // Frequency control word, middle INT8U
#define CC1101_FREQ0        0x0F        // Frequency control word, low INT8U
#define CC1101_MDMCFG4      0x10        // Modem configuration
#define CC1101_MDMCFG3      0x11        // Modem configuration
#define CC1101_MDMCFG2      0x12        // Modem configuration
#define CC1101_MDMCFG1      0x13        // Modem configuration
#define CC1101_MDMCFG0      0x14        // Modem configuration
#define CC1101_DEVIATN      0x15        // Modem deviation setting
#define CC1101_MCSM2        0x16        // Main Radio Control State Machine configuration
#define CC1101_MCSM1        0x17        // Main Radio Control State Machine configuration
#define CC1101_MCSM0        0x18        // Main Radio Control State Machine configuration
#define CC1101_FOCCFG       0x19        // Frequency Offset Compensation configuration
#define CC1101_BSCFG        0x1A        // Bit Synchronization configuration
#define CC1101_AGCCTRL2     0x1B        // AGC control
#define CC1101_AGCCTRL1     0x1C        // AGC control
#define CC1101_AGCCTRL0     0x1D        // AGC control
#define CC1101_WOREVT1      0x1E        // High INT8U Event 0 timeout
#define CC1101_WOREVT0      0x1F        // Low INT8U Event 0 timeout
#define CC1101_WORCTRL      0x20        // Wake On Radio control
#define CC1101_FREND1       0x21        // Front end RX configuration
#define CC1101_FREND0       0x22        // Front end TX configuration
#define CC1101_FSCAL3       0x23        // Frequency synthesizer calibration
#define CC1101_FSCAL2       0x24        // Frequency synthesizer calibration
#define CC1101_FSCAL1       0x25        // Frequency synthesizer calibration
#define CC1101_FSCAL0       0x26        // Frequency synthesizer calibration
#define CC1101_RCCTRL1      0x27        // RC oscillator configuration
#define CC1101_RCCTRL0      0x28        // RC oscillator configuration
#define CC1101_FSTEST       0x29        // Frequency synthesizer calibration control
#define CC1101_PTEST        0x2A        // Production test
#define CC1101_AGCTEST      0x2B        // AGC test
#define CC1101_TEST2        0x2C        // Various test settings
#define CC1101_TEST1        0x2D        // Various test settings
#define CC1101_TEST0        0x2E        // Various test settings

//CC1101 Strobe commands
#define CC1101_SRES         0x30        // Reset chip.
#define CC1101_SFSTXON      0x31        // Enable and calibrate frequency synthesizer (if MCSM0.FS_AUTOCAL=1).
                                        // If in RX/TX: Go to a wait state where only the synthesizer is
                                        // running (for quick RX / TX turnaround).
#define CC1101_SXOFF        0x32        // Turn off crystal oscillator.
#define CC1101_SCAL         0x33        // Calibrate frequency synthesizer and turn it off
                                        // (enables quick start).
#define CC1101_SRX          0x34        // Enable RX. Perform calibration first if coming from IDLE and
                                        // MCSM0.FS_AUTOCAL=1.
#define CC1101_STX          0x35        // In IDLE state: Enable TX. Perform calibration first if
                                        // MCSM0.FS_AUTOCAL=1. If in RX state and CCA is enabled:
                                        // Only go to TX if channel is clear.
#define CC1101_SIDLE        0x36        // Exit RX / TX, turn off frequency synthesizer and exit
                                        // Wake-On-Radio mode if applicable.
#define CC1101_SAFC         0x37        // Perform AFC adjustment of the frequency synthesizer
#define CC1101_SWOR         0x38        // Start automatic RX polling sequence (Wake-on-Radio)
#define CC1101_SPWD         0x39        // Enter power down mode when CSn goes high.
#define CC1101_SFRX         0x3A        // Flush the RX FIFO buffer.
#define CC1101_SFTX         0x3B        // Flush the TX FIFO buffer.
#define CC1101_SWORRST      0x3C        // Reset real time clock.
#define CC1101_SNOP         0x3D        // No operation. May be used to pad strobe commands to two
                                        // INT8Us for simpler software.
//CC1101 STATUS REGSITER
#define CC1101_PARTNUM      0x30
#define CC1101_VERSION      0x31
#define CC1101_FREQEST      0x32
#define CC1101_LQI          0x33
#define CC1101_RSSI         0x34
#define CC1101_MARCSTATE    0x35
#define CC1101_WORTIME1     0x36
#define CC1101_WORTIME0     0x37
#define CC1101_PKTSTATUS    0x38
#define CC1101_VCO_VC_DAC   0x39
#define CC1101_TXBYTES      0x3A
#define CC1101_RXBYTES      0x3B

//CC1101 PATABLE,TXFIFO,RXFIFO
#define CC1101_PATABLE      0x3E
#define CC1101_TXFIFO       0x3F
#define CC1101_RXFIFO       0x3F

// from 
// https://github.com/flipperdevices/flipperzero-firmware/blob/a47a55bfee0c0bbd5724953d89144f55544f6a52/lib/subghz/devices/cc1101_configs.c

const uint8_t subghz_device_cc1101_preset_ook_270khz_async_regs[] = {
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

const uint8_t subghz_device_cc1101_preset_ook_650khz_async_regs[] = {
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
    //  CC1101_AGCTRL0,0x40, // 01 - Low hysteresis, small asymmetric dead zone, medium gain; 00 - 8 samples agc; 00 - Normal AGC, 00 - 4dB boundary
    //  CC1101_AGCTRL1,0x00, // 0; 0 - LNA 2 gain is decreased to minimum before decreasing LNA gain; 00 - Relative carrier sense threshold disabled; 0000 - RSSI to MAIN_TARGET
    //  CC1101_AGCCTRL2, 0x03, // 00 - DVGA all; 000 - MAX LNA+LNA2; 011 - MAIN_TARGET 24 dB
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

const uint8_t subghz_device_cc1101_preset_2fsk_dev2_38khz_async_regs[] = {

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

const uint8_t subghz_device_cc1101_preset_2fsk_dev47_6khz_async_regs[] = {

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

const uint8_t subghz_device_cc1101_preset_msk_99_97kb_async_regs[] = {
    /* GPIO GD0 */
    CC1101_IOCFG0,
    0x06,

    CC1101_FIFOTHR,
    0x07, // The only important bit is ADC_RETENTION
    CC1101_SYNC1,
    0x46,
    CC1101_SYNC0,
    0x4C,
    CC1101_ADDR,
    0x00,
    CC1101_PKTLEN,
    0x00,
    CC1101_CHANNR,
    0x00,

    CC1101_PKTCTRL0,
    0x05,

    CC1101_FSCTRL0,
    0x23,
    CC1101_FSCTRL1,
    0x06,

    CC1101_MDMCFG0,
    0xF8,
    CC1101_MDMCFG1,
    0x22,
    CC1101_MDMCFG2,
    0x72,
    CC1101_MDMCFG3,
    0xF8,
    CC1101_MDMCFG4,
    0x5B,
    CC1101_DEVIATN,
    0x47,

    CC1101_MCSM0,
    0x18,
    CC1101_FOCCFG,
    0x16,

    CC1101_AGCCTRL0,
    0xB2,
    CC1101_AGCCTRL1,
    0x00,
    CC1101_AGCCTRL2,
    0xC7,

    CC1101_FREND0,
    0x10,
    CC1101_FREND1,
    0x56,

    CC1101_BSCFG,
    0x1C,
    CC1101_FSTEST,
    0x59,

    /* End load reg */
    0,
    0,

    // msk_async_patable[8]
    0xC0, // 10dBm 0xC0, 7dBm 0xC8, 5dBm 0x84, 0dBm 0x60, -10dBm 0x34, -15dBm 0x1D, -20dBm 0x0E, -30dBm 0x12
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
};


#endif // !FLIPPER_CONFIGS_H