#ifndef MFRC522_READER_H
#define MFRC522_READER_H

#include "infc_reader.h"
#include <MFRC522.h>

/**
 * @brief MFRC522 implementation of the NFC reader interface.
 *
 * This class provides a concrete implementation of the INFCReader interface
 * using the MFRC522 RFID reader module. It handles SPI communication and
 * card detection/reading operations.
 */
class MFRC522Reader : public INFCReader {
public:
    /**
     * @brief Construct a new MFRC522Reader object
     *
     * @param ssPin The Slave Select (SS) pin for SPI communication
     * @param rstPin The Reset pin for the MFRC522 module
     */
    MFRC522Reader(uint8_t ssPin, uint8_t rstPin);

    /**
     * @brief Initialize the MFRC522 reader and SPI bus
     * @throws std::runtime_error if initialization fails
     */
    void initialize() override;

    /**
     * @brief Put the reader into power-down mode
     */
    void powerDown() override;

    /**
     * @brief Check if a new card is present in the field
     * @return true if a new card is detected
     */
    bool isNewCardPresent() override;

    /**
     * @brief Read the serial number of the detected card
     * @return true if serial number was successfully read
     */
    bool readCardSerial() override;

    /**
     * @brief Get the UID of the currently detected card
     * @return UID structure containing the card's unique identifier
     */
    UID getUID() const override;

    /**
     * @brief Halt the current card and stop crypto operations
     */
    void halt() override;

private:
    MFRC522 mfrc;
    bool initialized;  // Tracks initialization state of the reader
};

#endif  // MFRC522_READER_H 