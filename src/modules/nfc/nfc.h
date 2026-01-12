#ifndef NFC_H
#define NFC_H

#include <functional>
#include <memory>
#include "infc_reader.h"

/**
 * @brief Represents the possible states of the NFC module.
 */
enum class NFCState {
    Idle,     ///< No card is being read
    Reading   ///< A card is currently being read
};

/**
 * @brief Main NFC module class that handles card detection and reading.
 * 
 * This class provides a high-level interface for NFC/RFID operations.
 * It follows SOLID principles and uses dependency injection for the
 * actual NFC reader implementation.
 * 
 * Thread Safety: This class is not thread-safe and should be accessed
 * from a single thread only.
 */
class NFC {
public:
    /**
     * @brief Callback type for card detection events.
     * 
     * The callback receives the card's UID as a hexadecimal string.
     * Callbacks should be lightweight and non-blocking as they are
     * called from the update loop.
     */
    using CardDetectedCallback = std::function<void(const String& uid)>;

    /**
     * @brief Construct a new NFC object.
     *
     * @param readerImpl A unique pointer to an INFCReader implementation.
     * @throws std::invalid_argument if readerImpl is null.
     */
    explicit NFC(std::unique_ptr<INFCReader> readerImpl);

    /**
     * @brief Default destructor for NFC.
     */
    ~NFC() = default;

    /**
     * @brief Initialize the NFC module.
     *
     * This method initializes the NFC reader hardware and prepares
     * it for operation.
     * 
     * @throws std::runtime_error if initialization fails or if the reader
     *         is not properly set up.
     */
    void begin();

    /**
     * @brief Update the NFC module.
     *
     * This method should be called periodically in the main loop to check
     * for new cards and process them. If a card is detected, the registered
     * callback (if any) will be invoked with the card's UID.
     * 
     * Any exceptions in the callback will be caught and logged to prevent
     * them from affecting the NFC module's operation.
     */
    void update();

    /**
     * @brief Disable the NFC module.
     *
     * Places the NFC reader in a low-power state. The module can be
     * reactivated by calling begin().
     * 
     * @throws std::runtime_error if shutdown fails.
     */
    void shutdown();

    /**
     * @brief Get the current state of the NFC module.
     *
     * @return NFCState The current operational state.
     */
    [[nodiscard]] auto getState() const -> NFCState;

    /**
     * @brief Set the callback for card detection events.
     *
     * @param callback A function to be called when a card is detected.
     *                 Pass nullptr to remove the callback.
     */
    void setCardDetectedCallback(CardDetectedCallback callback);

    // Prevent copying and moving
    NFC(const NFC&) = delete;
    NFC& operator=(const NFC&) = delete;
    NFC(NFC&&) = delete;
    NFC& operator=(NFC&&) = delete;

private:
    std::unique_ptr<INFCReader> reader;
    CardDetectedCallback callback{nullptr};
    NFCState state{NFCState::Idle};

    /**
     * @brief Format a byte array as a hexadecimal string.
     *
     * @param uidBytes Pointer to the byte array.
     * @param uidSize Size of the array (max 10 bytes).
     * @return String The formatted hexadecimal string.
     */
    auto formatUID(const byte* uidBytes, byte uidSize) const -> String;
};

#endif  // NFC_H
