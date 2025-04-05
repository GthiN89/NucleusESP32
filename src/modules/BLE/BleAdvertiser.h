#ifndef BLE_ADVERTISER_H
#define BLE_ADVERTISER_H

#include <Arduino.h>
#include <string> // Keep for potential future use, though not strictly needed for raw packets

// ESP-IDF Headers for direct GAP control
#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_err.h"
#include "esp_log.h" // For logging ESP errors/info
#include "esp_random.h" // For esp_random
#include "nvs_flash.h"

// Forward declaration (if needed, though likely handled by includes)
// struct esp_ble_adv_params_t;

// Enum to select advertising type
enum class AdvertisingMode {
    APPLE_PROXIMITY,
    GOOGLE_FAST_PAIR,
    BOTH_RANDOM // Randomly alternates between Apple and Fast Pair
};

class BleAdvertiser {
public:
    // Constructor: Takes the desired advertising *duration* in milliseconds for each packet burst
    BleAdvertiser(uint32_t durationMs = 200); // Defaulting to 200ms as per Fast Pair example

    // Initializes BLE controller.
    // Returns true on success, false on failure.
    bool begin(const char* deviceName = "ESP32_Advertiser"); // Device name less critical now

    // Performs one cycle of advertising based on the current mode.
    void performAdvertisingCycle();

    // Sets the advertising mode
    void setAdvertisingMode(AdvertisingMode mode);

    // Sets a new advertising duration for each burst
    void setAdvertisingDuration(uint32_t durationMs);
    AdvertisingMode currentMode;    // Current advertising mode


private:
    // --- Static Constant Data ---
    // Apple Proximity Data (Original)
    static const uint8_t DEVICES[][31];
    static const uint8_t SHORT_DEVICES[][23];
    static const size_t NUM_DEVICES;
    static const size_t NUM_SHORT_DEVICES;
    static const size_t DEVICE_PAYLOAD_SIZE;
    static const size_t SHORT_DEVICE_PAYLOAD_SIZE;

    // Google Fast Pair Data (New)
    static const uint32_t FAST_PAIR_MODELS[];
    static const size_t NUM_FAST_PAIR_MODELS;
    // Template for Fast Pair raw packet (Model ID and TX Power filled later)
    // Structure: Len,Type,UUID | Len,Type,UUID,ModelID | Len,Type,TXPower
    static const uint8_t FAST_PAIR_PACKET_TEMPLATE[14];
    static const size_t FAST_PAIR_PACKET_SIZE;

    // --- Member Variables ---
    uint32_t advertisingDurationMs; // How long to advertise each packet burst
    bool initialized;               // Flag to track if begin() was successful
    // ESP-IDF Advertising Parameters (shared or mode-specific if needed)
    esp_ble_adv_params_t advParams;

    // --- Private Helper Methods ---
    // Generates and advertises one Apple Proximity packet burst using ESP-IDF GAP
    void advertiseAppleProximityRaw();
    // Generates and advertises one Google Fast Pair packet burst using ESP-IDF GAP
    void advertiseFastPairRaw();
    // Helper to generate random MAC (NRPA Type - Non-Resolvable Private Address)
    void generateRandomNRPA(esp_bd_addr_t& addr);
};

#endif // BLE_ADVERTISER_H