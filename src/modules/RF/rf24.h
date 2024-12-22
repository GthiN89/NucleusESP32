#ifndef RF24CLASS_H
#define RF24CLASS_H

#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
// nRF24 physical layer properties
#define RADIOLIB_NRF24_FREQUENCY_STEP_SIZE                      1000000.0
#define RADIOLIB_NRF24_MAX_PACKET_LENGTH                        32

// nRF24 SPI commands
#define RADIOLIB_NRF24_CMD_READ                                 0b00000000
#define RADIOLIB_NRF24_CMD_WRITE                                0b00100000
#define RADIOLIB_NRF24_CMD_READ_RX_PAYLOAD                      0b01100001
#define RADIOLIB_NRF24_CMD_WRITE_TX_PAYLOAD                     0b10100000
#define RADIOLIB_NRF24_CMD_FLUSH_TX                             0b11100001
#define RADIOLIB_NRF24_CMD_FLUSH_RX                             0b11100010
#define RADIOLIB_NRF24_CMD_REUSE_TX_PAXLOAD                     0b11100011
#define RADIOLIB_NRF24_CMD_READ_RX_PAYLOAD_WIDTH                0b01100000
#define RADIOLIB_NRF24_CMD_WRITE_ACK_PAYLOAD                    0b10101000
#define RADIOLIB_NRF24_CMD_WRITE_TX_PAYLOAD_NOACK               0b10110000
#define RADIOLIB_NRF24_CMD_NOP                                  0b11111111

// nRF24 register map
#define RADIOLIB_NRF24_REG_CONFIG                               0x00
#define RADIOLIB_NRF24_REG_EN_AA                                0x01
#define RADIOLIB_NRF24_REG_EN_RXADDR                            0x02
#define RADIOLIB_NRF24_REG_SETUP_AW                             0x03
#define RADIOLIB_NRF24_REG_SETUP_RETR                           0x04
#define RADIOLIB_NRF24_REG_RF_CH                                0x05
#define RADIOLIB_NRF24_REG_RF_SETUP                             0x06
#define RADIOLIB_NRF24_REG_STATUS                               0x07
#define RADIOLIB_NRF24_REG_OBSERVE_TX                           0x08
#define RADIOLIB_NRF24_REG_RPD                                  0x09
#define RADIOLIB_NRF24_REG_RX_ADDR_P0                           0x0A
#define RADIOLIB_NRF24_REG_RX_ADDR_P1                           0x0B
#define RADIOLIB_NRF24_REG_RX_ADDR_P2                           0x0C
#define RADIOLIB_NRF24_REG_RX_ADDR_P3                           0x0D
#define RADIOLIB_NRF24_REG_RX_ADDR_P4                           0x0E
#define RADIOLIB_NRF24_REG_RX_ADDR_P5                           0x0F
#define RADIOLIB_NRF24_REG_TX_ADDR                              0x10
#define RADIOLIB_NRF24_REG_RX_PW_P0                             0x11
#define RADIOLIB_NRF24_REG_RX_PW_P1                             0x12
#define RADIOLIB_NRF24_REG_RX_PW_P2                             0x13
#define RADIOLIB_NRF24_REG_RX_PW_P3                             0x14
#define RADIOLIB_NRF24_REG_RX_PW_P4                             0x15
#define RADIOLIB_NRF24_REG_RX_PW_P5                             0x16
#define RADIOLIB_NRF24_REG_FIFO_STATUS                          0x17
#define RADIOLIB_NRF24_REG_DYNPD                                0x1C
#define RADIOLIB_NRF24_REG_FEATURE                              0x1D

// RADIOLIB_NRF24_REG_CONFIG                                                  MSB   LSB   DESCRIPTION
#define RADIOLIB_NRF24_MASK_RX_DR_IRQ_OFF                       0b01000000  //  6     6   RX_DR will not be reflected on IRQ pin
#define RADIOLIB_NRF24_MASK_RX_DR_IRQ_ON                        0b00000000  //  6     6   RX_DR will be reflected on IRQ pin as active low (default)
#define RADIOLIB_NRF24_MASK_TX_DS_IRQ_OFF                       0b00100000  //  5     5   TX_DS will not be reflected on IRQ pin
#define RADIOLIB_NRF24_MASK_TX_DS_IRQ_ON                        0b00000000  //  5     5   TX_DS will be reflected on IRQ pin as active low (default)
#define RADIOLIB_NRF24_MASK_MAX_RT_IRQ_OFF                      0b00010000  //  4     4   MAX_RT will not be reflected on IRQ pin
#define RADIOLIB_NRF24_MASK_MAX_RT_IRQ_ON                       0b00000000  //  4     4   MAX_RT will be reflected on IRQ pin as active low (default)
#define RADIOLIB_NRF24_CRC_OFF                                  0b00000000  //  3     3   CRC calculation: disabled
#define RADIOLIB_NRF24_CRC_ON                                   0b00001000  //  3     3                    enabled (default)
#define RADIOLIB_NRF24_CRC_8                                    0b00000000  //  2     2   CRC scheme: CRC8 (default)
#define RADIOLIB_NRF24_CRC_16                                   0b00000100  //  2     2               CRC16
#define RADIOLIB_NRF24_POWER_UP                                 0b00000010  //  1     1   power up
#define RADIOLIB_NRF24_POWER_DOWN                               0b00000000  //  1     1   power down
#define RADIOLIB_NRF24_PTX                                      0b00000000  //  0     0   enable primary Tx
#define RADIOLIB_NRF24_PRX                                      0b00000001  //  0     0   enable primary Rx

// RADIOLIB_NRF24_REG_EN_AA
#define RADIOLIB_NRF24_AA_ALL_OFF                               0b00000000  //  5     0   auto-ACK on all pipes: disabled
#define RADIOLIB_NRF24_AA_ALL_ON                                0b00111111  //  5     0                          enabled (default)
#define RADIOLIB_NRF24_AA_P5_OFF                                0b00000000  //  5     5   auto-ACK on pipe 5: disabled
#define RADIOLIB_NRF24_AA_P5_ON                                 0b00100000  //  5     5                       enabled (default)
#define RADIOLIB_NRF24_AA_P4_OFF                                0b00000000  //  4     4   auto-ACK on pipe 4: disabled
#define RADIOLIB_NRF24_AA_P4_ON                                 0b00010000  //  4     4                       enabled (default)
#define RADIOLIB_NRF24_AA_P3_OFF                                0b00000000  //  3     3   auto-ACK on pipe 3: disabled
#define RADIOLIB_NRF24_AA_P3_ON                                 0b00001000  //  3     3                       enabled (default)
#define RADIOLIB_NRF24_AA_P2_OFF                                0b00000000  //  2     2   auto-ACK on pipe 2: disabled
#define RADIOLIB_NRF24_AA_P2_ON                                 0b00000100  //  2     2                       enabled (default)
#define RADIOLIB_NRF24_AA_P1_OFF                                0b00000000  //  1     1   auto-ACK on pipe 1: disabled
#define RADIOLIB_NRF24_AA_P1_ON                                 0b00000010  //  1     1                       enabled (default)
#define RADIOLIB_NRF24_AA_P0_OFF                                0b00000000  //  0     0   auto-ACK on pipe 0: disabled
#define RADIOLIB_NRF24_AA_P0_ON                                 0b00000001  //  0     0                       enabled (default)

// RADIOLIB_NRF24_REG_EN_RXADDR
#define RADIOLIB_NRF24_P5_OFF                                   0b00000000  //  5     5   receive pipe 5: disabled (default)
#define RADIOLIB_NRF24_P5_ON                                    0b00100000  //  5     5                   enabled
#define RADIOLIB_NRF24_P4_OFF                                   0b00000000  //  4     4   receive pipe 4: disabled (default)
#define RADIOLIB_NRF24_P4_ON                                    0b00010000  //  4     4                   enabled
#define RADIOLIB_NRF24_P3_OFF                                   0b00000000  //  3     3   receive pipe 3: disabled (default)
#define RADIOLIB_NRF24_P3_ON                                    0b00001000  //  3     3                   enabled
#define RADIOLIB_NRF24_P2_OFF                                   0b00000000  //  2     2   receive pipe 2: disabled (default)
#define RADIOLIB_NRF24_P2_ON                                    0b00000100  //  2     2                   enabled
#define RADIOLIB_NRF24_P1_OFF                                   0b00000000  //  1     1   receive pipe 1: disabled
#define RADIOLIB_NRF24_P1_ON                                    0b00000010  //  1     1                   enabled (default)
#define RADIOLIB_NRF24_P0_OFF                                   0b00000000  //  0     0   receive pipe 0: disabled
#define RADIOLIB_NRF24_P0_ON                                    0b00000001  //  0     0                   enabled (default)

// RADIOLIB_NRF24_REG_SETUP_AW
#define RADIOLIB_NRF24_ADDRESS_2_BYTES                          0b00000000  //  1     0   address width: 2 bytes
#define RADIOLIB_NRF24_ADDRESS_3_BYTES                          0b00000001  //  1     0                  3 bytes
#define RADIOLIB_NRF24_ADDRESS_4_BYTES                          0b00000010  //  1     0                  4 bytes
#define RADIOLIB_NRF24_ADDRESS_5_BYTES                          0b00000011  //  1     0                  5 bytes (default)

// RADIOLIB_NRF24_REG_SETUP_RETR
#define RADIOLIB_NRF24_ARD                                      0b00000000  //  7     4   auto retransmit delay: t[us] = (NRF24_ARD + 1) * 250 us
#define RADIOLIB_NRF24_ARC_OFF                                  0b00000000  //  3     0   auto retransmit count: auto retransmit disabled
#define RADIOLIB_NRF24_ARC                                      0b00000011  //  3     0                          up to 3 retransmits on AA fail (default)

// RADIOLIB_NRF24_REG_RF_CH
#define RADIOLIB_NRF24_RF_CH                                    0b00000010  //  6     0   RF channel: f_CH[MHz] = 2400 MHz + NRF24_RF_CH

// RADIOLIB_NRF24_REG_RF_SETUP
#define RADIOLIB_NRF24_CONT_WAVE_OFF                            0b00000000  //  7     7   continuous carrier transmit: disabled (default)
#define RADIOLIB_NRF24_CONT_WAVE_ON                             0b10000000  //  7     7                                enabled
#define RADIOLIB_NRF24_DR_250_KBPS                              0b00100000  //  5     5   data rate: 250 kbps
#define RADIOLIB_NRF24_DR_1_MBPS                                0b00000000  //  3     3              1 Mbps (default)
#define RADIOLIB_NRF24_DR_2_MBPS                                0b00001000  //  3     3              2 Mbps
#define RADIOLIB_NRF24_PLL_LOCK_ON                              0b00010000  //  4     4   force PLL lock: enabled
#define RADIOLIB_NRF24_PLL_LOCK_OFF                             0b00000000  //  4     4                   disabled (default)
#define RADIOLIB_NRF24_RF_PWR_18_DBM                            0b00000000  //  2     1   output power: -18 dBm
#define RADIOLIB_NRF24_RF_PWR_12_DBM                            0b00000010  //  2     1                 -12 dBm
#define RADIOLIB_NRF24_RF_PWR_6_DBM                             0b00000100  //  2     1                 -6 dBm
#define RADIOLIB_NRF24_RF_PWR_0_DBM                             0b00000110  //  2     1                 0 dBm (default)

// RADIOLIB_NRF24_REG_STATUS
#define RADIOLIB_NRF24_RX_DR                                    0b01000000  //  6     6   Rx data ready
#define RADIOLIB_NRF24_TX_DS                                    0b00100000  //  5     5   Tx data sent
#define RADIOLIB_NRF24_MAX_RT                                   0b00010000  //  4     4   maximum number of retransmits reached (must be cleared to continue)
#define RADIOLIB_NRF24_RX_FIFO_EMPTY                            0b00001110  //  3     1   Rx FIFO is empty
#define RADIOLIB_NRF24_RX_P_NO                                  0b00000000  //  3     1   number of data pipe that received data
#define RADIOLIB_NRF24_TX_FIFO_FULL                             0b00000001  //  0     0   Tx FIFO is full

// RADIOLIB_NRF24_REG_OBSERVE_TX
#define RADIOLIB_NRF24_PLOS_CNT                                 0b00000000  //  7     4   number of lost packets
#define RADIOLIB_NRF24_ARC_CNT                                  0b00000000  //  3     0   number of retransmitted packets

// RADIOLIB_NRF24_REG_RPD
#define RADIOLIB_NRF24_RP_BELOW_64_DBM                          0b00000000  //  0     0   received power in the current channel: less than -64 dBm
#define RADIOLIB_NRF24_RP_ABOVE_64_DBM                          0b00000001  //  0     0                                            more than -64 dBm

// RADIOLIB_NRF24_REG_FIFO_STATUS
#define RADIOLIB_NRF24_TX_REUSE                                 0b01000000  //  6     6   reusing last transmitted payload
#define RADIOLIB_NRF24_TX_FIFO_FULL_FLAG                        0b00100000  //  5     5   Tx FIFO is full
#define RADIOLIB_NRF24_TX_FIFO_EMPTY_FLAG                       0b00010000  //  4     4   Tx FIFO is empty
#define RADIOLIB_NRF24_RX_FIFO_FULL_FLAG                        0b00000010  //  1     1   Rx FIFO is full
#define RADIOLIB_NRF24_RX_FIFO_EMPTY_FLAG                       0b00000001  //  0     0   Rx FIFO is empty

// RADIOLIB_NRF24_REG_DYNPD
#define RADIOLIB_NRF24_DPL_P5_OFF                               0b00000000  //  5     5   dynamic payload length on pipe 5: disabled (default)
#define RADIOLIB_NRF24_DPL_P5_ON                                0b00100000  //  5     5                                     enabled
#define RADIOLIB_NRF24_DPL_P4_OFF                               0b00000000  //  4     4   dynamic payload length on pipe 4: disabled (default)
#define RADIOLIB_NRF24_DPL_P4_ON                                0b00010000  //  4     4                                     enabled
#define RADIOLIB_NRF24_DPL_P3_OFF                               0b00000000  //  3     3   dynamic payload length on pipe 3: disabled (default)
#define RADIOLIB_NRF24_DPL_P3_ON                                0b00001000  //  3     3                                     enabled
#define RADIOLIB_NRF24_DPL_P2_OFF                               0b00000000  //  2     2   dynamic payload length on pipe 2: disabled (default)
#define RADIOLIB_NRF24_DPL_P2_ON                                0b00000100  //  2     2                                     enabled
#define RADIOLIB_NRF24_DPL_P1_OFF                               0b00000000  //  1     1   dynamic payload length on pipe 1: disabled (default)
#define RADIOLIB_NRF24_DPL_P1_ON                                0b00000010  //  1     1                                     enabled
#define RADIOLIB_NRF24_DPL_P0_OFF                               0b00000000  //  0     0   dynamic payload length on pipe 0: disabled (default)
#define RADIOLIB_NRF24_DPL_P0_ON                                0b00000001  //  0     0                                     enabled
#define RADIOLIB_NRF24_DPL_ALL_OFF                              0b00000000  //  5     0   disable all dynamic payloads
#define RADIOLIB_NRF24_DPL_ALL_ON                               0b00111111  //  5     0   enable all dynamic payloads

// RADIOLIB_NRF24_REG_FEATURE
#define RADIOLIB_NRF24_DPL_OFF                                  0b00000000  //  2     2   dynamic payload length: disabled (default)
#define RADIOLIB_NRF24_DPL_ON                                   0b00000100  //  2     2                           enabled
#define RADIOLIB_NRF24_ACK_PAY_OFF                              0b00000000  //  1     1   payload with ACK packets: disabled (default)
#define RADIOLIB_NRF24_ACK_PAY_ON                               0b00000010  //  1     1                             enabled
#define RADIOLIB_NRF24_DYN_ACK_OFF                              0b00000000  //  0     0   payloads without ACK: disabled (default)
#define RADIOLIB_NRF24_DYN_ACK_ON                               0b00000001  //  0     0                         enabled

// RadioLib defaults
#define RADIOLIB_NRF24_DEFAULT_FREQ                             2400
#define RADIOLIB_NRF24_DEFAULT_DR                               1000
#define RADIOLIB_NRF24_DEFAULT_POWER                            -12
#define RADIOLIB_NRF24_DEFAULT_ADDRWIDTH                        5

// RF24 States
enum RF24State {
    RF24_STATE_IDLE,
    RF24_STATE_TEST
};
extern RF24State RF24CurrentState;

// class RF24Class {
// public:
//     RF24Class(uint8_t cePin, uint8_t csPin);

//     bool begin();
//     void send(const void* data, uint8_t len);
//     bool receive(void* data, uint8_t len);
//     void setChannel(uint8_t channel);
//     void setPowerLevel(uint8_t level);
//     void enablePromiscuousMode();

// private:
//     uint8_t _cePin, _csPin;

//     void writeRegister(uint8_t reg, uint8_t value);
//     uint8_t readRegister(uint8_t reg);
//     void flushTx();
//     void flushRx();
// };
void enableRF24();
void testRF24();
#endif // RF24CLASS_H
