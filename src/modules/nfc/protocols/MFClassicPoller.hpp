#include <Adafruit_PN532.h>
#include <vector>

#define MF_CLASSIC_BLOCK_SIZE 16
#define MF_CLASSIC_SECTOR_TRAILER_BLOCK 3
#define MF_CLASSIC_1K_MAX_SECTOR 15
#define MF_CLASSIC_4K_MAX_SECTOR 39

#define MF_CLASSIC_CMD_AUTH_KEY_A          (0x60U)
#define MF_CLASSIC_CMD_AUTH_KEY_B          (0x61U)
#define MF_CLASSIC_CMD_BACKDOOR_AUTH_KEY_A (0x64U)
#define MF_CLASSIC_CMD_BACKDOOR_AUTH_KEY_B (0x65U)
#define MF_CLASSIC_CMD_READ_BLOCK          (0x30U)
#define MF_CLASSIC_CMD_WRITE_BLOCK         (0xA0U)
#define MF_CLASSIC_CMD_VALUE_DEC           (0xC0U)
#define MF_CLASSIC_CMD_VALUE_INC           (0xC1U)
#define MF_CLASSIC_CMD_VALUE_RESTORE       (0xC2U)
#define MF_CLASSIC_CMD_VALUE_TRANSFER      (0xB0U)

#define MF_CLASSIC_CMD_HALT_MSB                (0x50)
#define MF_CLASSIC_CMD_HALT_LSB                (0x00)
#define MF_CLASSIC_CMD_ACK                     (0x0A)
#define MF_CLASSIC_CMD_NACK                    (0x00)
#define MF_CLASSIC_CMD_NACK_TRANSFER_INVALID   (0x04)
#define MF_CLASSIC_CMD_NACK_TRANSFER_CRC_ERROR (0x01)

#define MF_CLASSIC_TOTAL_SECTORS_MAX (40)
#define MF_CLASSIC_TOTAL_BLOCKS_MAX  (256)
#define MF_CLASSIC_READ_MASK_SIZE    (MF_CLASSIC_TOTAL_BLOCKS_MAX / 32)
#define MF_CLASSIC_BLOCK_SIZE        (16)
#define MF_CLASSIC_KEY_SIZE          (6)
#define MF_CLASSIC_ACCESS_BYTES_SIZE (4)

#define MF_CLASSIC_NT_SIZE (4)
#define MF_CLASSIC_NR_SIZE (4)
#define MF_CLASSIC_AR_SIZE (4)
#define MF_CLASSIC_AT_SIZE (4)

enum MfClassicType {
    MfClassicTypeMini,
    MfClassicType1k,
    MfClassicType4k,
    MfClassicTypeNum
};

enum MfClassicKeyType {
    MfClassicKeyTypeA,
    MfClassicKeyTypeB
};

struct MfClassicKey {
    uint8_t data[6];
};

struct MfClassicBlock {
    uint8_t data[MF_CLASSIC_BLOCK_SIZE];
};

struct MfClassicSectorTrailer {
    MfClassicKey key_a;
    uint8_t access_bits[4];
    MfClassicKey key_b;
};

class MfClassicPoller {
public:
    MfClassicPoller(Adafruit_PN532* reader);

    
    bool begin();
    MfClassicType detectType();
    bool authenticateBlock(uint8_t blockNum, uint8_t key, MfClassicKeyType keyType);
    bool readBlock(uint8_t blockNum, MfClassicBlock* blockData);
    bool writeBlock(uint8_t blockNum, MfClassicBlock* blockData);
    void dumpToSerial();

private:
    Adafruit_PN532* nfc;
    MfClassicType cardType;
    uint8_t uid[7];
    uint8_t uidLength;
};
