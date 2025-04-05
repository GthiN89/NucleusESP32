// nfc.h
#ifndef NFC_H
#define NFC_H

#include <Arduino.h>
#include <Adafruit_PN532.h> // Include library header
#include "protocols/nfc_card_defs.h" // Include your generic card definitions

// Forward declaration for global nfc object for potential use by other files including nfc.h
// If only nfc.cpp uses it, this extern isn't strictly needed here, but doesn't hurt.
namespace NFC {
    extern Adafruit_PN532 nfc; // Declare global object as external
}

namespace NFC {

    /**
     * @brief Main class to manage NFC functionality using PN532.
     * Handles initialization, card detection, and specific card interactions.
     */
    class NFC_CLASS {
    public:
        /**
         * @brief Constructor. Initializes internal state.
         */
        NFC_CLASS();

        /**
         * @brief Destructor. Calls deinit if necessary.
         */
        ~NFC_CLASS();

        /**
         * @brief Initializes the SPI communication and the PN532 module.
         * Detects the chip and configures it for card reading.
         * @return true if initialization was successful, false otherwise.
         */
        bool init();

        /**
         * @brief Deinitializes the NFC module (optional cleanup). Sets CS high.
         */
        void deinit();

        /**
         * @brief Checks if the NFC module is initialized.
         * @return true if initialized, false otherwise.
         */
        bool isInitialized() const;

        /**
         * @brief Detects a card, identifies its type (Classic, UL, Type4 etc.),
         *        and attempts to dump readable sectors using ONLY Mifare Classic default keys.
         * Stores results in an internal CardData structure. Prints results block-by-block for Classic.
         * @return true if a compatible card (any type identified) was detected, false otherwise or on error.
         */
        bool mifareDumpDefaultKeys();

        /**
         * @brief Attempts Mifare Classic dictionary attack ONLY on sectors where default keys failed.
         * Assumes mifareDumpDefaultKeys() has already been run successfully on a Mifare Classic card
         * to populate the internal CardData structure.
         * Prints results block-by-block for newly cracked sectors.
         * @return true if the process completed (doesn't guarantee keys were found), false if preconditions not met or on error.
         */
        bool mifareDumpDictionaryAttack();

        /**
         * @brief Prints a summary of the currently stored card data (UID, Type, and block data if Classic).
         */
        void printStoredCardData();


    private:
        bool _initialized = false; // Initialization status flag

        // Note: Uses global 'nfc' object (defined in nfc.cpp) for hardware access.
        // Note: Uses global 'currentNfcCard' object (defined in nfc.cpp) to store card data.
        //       Consider making currentNfcCard a member if multiple NFC_CLASS instances
        //       or better encapsulation is needed in the future.
    };

} // namespace NFC

#endif // NFC_H