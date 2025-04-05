#include "BleAdvertiser.h"

// --- Static Data Definitions ---

// Apple Proximity Data (Keep as before)
const uint8_t BleAdvertiser::DEVICES[][31] = { /* ... paste DEVICES array here ... */ };
const uint8_t BleAdvertiser::SHORT_DEVICES[][23] = { /* ... paste SHORT_DEVICES array here ... */ };
const size_t BleAdvertiser::NUM_DEVICES = sizeof(BleAdvertiser::DEVICES) / sizeof(BleAdvertiser::DEVICES[0]);
const size_t BleAdvertiser::NUM_SHORT_DEVICES = sizeof(BleAdvertiser::SHORT_DEVICES) / sizeof(BleAdvertiser::SHORT_DEVICES[0]);
const size_t BleAdvertiser::DEVICE_PAYLOAD_SIZE = sizeof(BleAdvertiser::DEVICES[0]);
const size_t BleAdvertiser::SHORT_DEVICE_PAYLOAD_SIZE = sizeof(BleAdvertiser::SHORT_DEVICES[0]);
#define ESP_ERR_NVS_BASE                    0x1100                     /*!< Starting number of error codes */
#define ESP_ERR_NVS_NOT_INITIALIZED         (ESP_ERR_NVS_BASE + 0x01)  /*!< The storage driver is not initialized */
#define ESP_ERR_NVS_NOT_FOUND               (ESP_ERR_NVS_BASE + 0x02)  /*!< Id namespace doesn’t exist yet and mode is NVS_READONLY */
#define ESP_ERR_NVS_TYPE_MISMATCH           (ESP_ERR_NVS_BASE + 0x03)  /*!< The type of set or get operation doesn't match the type of value stored in NVS */
#define ESP_ERR_NVS_READ_ONLY               (ESP_ERR_NVS_BASE + 0x04)  /*!< Storage handle was opened as read only */
#define ESP_ERR_NVS_NOT_ENOUGH_SPACE        (ESP_ERR_NVS_BASE + 0x05)  /*!< There is not enough space in the underlying storage to save the value */
#define ESP_ERR_NVS_INVALID_NAME            (ESP_ERR_NVS_BASE + 0x06)  /*!< Namespace name doesn’t satisfy constraints */
#define ESP_ERR_NVS_INVALID_HANDLE          (ESP_ERR_NVS_BASE + 0x07)  /*!< Handle has been closed or is NULL */
#define ESP_ERR_NVS_REMOVE_FAILED           (ESP_ERR_NVS_BASE + 0x08)  /*!< The value wasn’t updated because flash write operation has failed. The value was written however, and update will be finished after re-initialization of nvs, provided that flash operation doesn’t fail again. */
#define ESP_ERR_NVS_KEY_TOO_LONG            (ESP_ERR_NVS_BASE + 0x09)  /*!< Key name is too long */
#define ESP_ERR_NVS_PAGE_FULL               (ESP_ERR_NVS_BASE + 0x0a)  /*!< Internal error; never returned by nvs API functions */
#define ESP_ERR_NVS_INVALID_STATE           (ESP_ERR_NVS_BASE + 0x0b)  /*!< NVS is in an inconsistent state due to a previous error. Call nvs_flash_init and nvs_open again, then retry. */
#define ESP_ERR_NVS_INVALID_LENGTH          (ESP_ERR_NVS_BASE + 0x0c)  /*!< String or blob length is not sufficient to store data */
#define ESP_ERR_NVS_NO_FREE_PAGES           (ESP_ERR_NVS_BASE + 0x0d)  /*!< NVS partition doesn't contain any empty pages. This may happen if NVS partition was truncated. Erase the whole partition and call nvs_flash_init again. */
#define ESP_ERR_NVS_VALUE_TOO_LONG          (ESP_ERR_NVS_BASE + 0x0e)  /*!< Value doesn't fit into the entry or string or blob length is longer than supported by the implementation */
#define ESP_ERR_NVS_PART_NOT_FOUND          (ESP_ERR_NVS_BASE + 0x0f)  /*!< Partition with specified name is not found in the partition table */

#define ESP_ERR_NVS_NEW_VERSION_FOUND       (ESP_ERR_NVS_BASE + 0x10)  /*!< NVS partition contains data in new format and cannot be recognized by this version of code */
#define ESP_ERR_NVS_XTS_ENCR_FAILED         (ESP_ERR_NVS_BASE + 0x11)  /*!< XTS encryption failed while writing NVS entry */
#define ESP_ERR_NVS_XTS_DECR_FAILED         (ESP_ERR_NVS_BASE + 0x12)  /*!< XTS decryption failed while reading NVS entry */
#define ESP_ERR_NVS_XTS_CFG_FAILED          (ESP_ERR_NVS_BASE + 0x13)  /*!< XTS configuration setting failed */
#define ESP_ERR_NVS_XTS_CFG_NOT_FOUND       (ESP_ERR_NVS_BASE + 0x14)  /*!< XTS configuration not found */
#define ESP_ERR_NVS_ENCR_NOT_SUPPORTED      (ESP_ERR_NVS_BASE + 0x15)  /*!< NVS encryption is not supported in this version */
#define ESP_ERR_NVS_KEYS_NOT_INITIALIZED    (ESP_ERR_NVS_BASE + 0x16)  /*!< NVS key partition is uninitialized */
#define ESP_ERR_NVS_CORRUPT_KEY_PART        (ESP_ERR_NVS_BASE + 0x17)  /*!< NVS key partition is corrupt */
#define ESP_ERR_NVS_WRONG_ENCRYPTION        (ESP_ERR_NVS_BASE + 0x19)  /*!< NVS partition is marked as encrypted with generic flash encryption. This is forbidden since the NVS encryption works differently. */

#define ESP_ERR_NVS_CONTENT_DIFFERS         (ESP_ERR_NVS_BASE + 0x18)  /*!< Internal error; never returned by nvs API functions.  NVS key is different in comparison */

#define NVS_DEFAULT_PART_NAME               "nvs"   /*!< Default partition name of the NVS partition in the partition table */

#define NVS_PART_NAME_MAX_SIZE              16   /*!< maximum length of partition name (excluding null terminator) */
#define NVS_KEY_NAME_MAX_SIZE               16   /*!< Maximal length of NVS key name (including null terminator) */

// Google Fast Pair Data
// Uncomment this if you only want debug devices
//#define DEBUG_DEVICES_ONLY
const uint32_t BleAdvertiser::FAST_PAIR_MODELS[] = {
#ifdef DEBUG_DEVICES_ONLY
    // Custom debug popups
    0xD99CA1,  // Flipper Zero"
    0x77FF67,  // Free Robux"
    // ... (rest of debug models)
#else
    // Genuine non-production/forgotten (good job Google)
    0x0001F0,  // Bisto CSR8670 Dev Board
    // ... (paste the full models array from the provided code here) ...
    0xCB2FE7,  // soundcore Motion X500
#endif
};
const size_t BleAdvertiser::NUM_FAST_PAIR_MODELS = sizeof(BleAdvertiser::FAST_PAIR_MODELS) / sizeof(BleAdvertiser::FAST_PAIR_MODELS[0]);

// Fast Pair Raw Packet Template
const uint8_t BleAdvertiser::FAST_PAIR_PACKET_TEMPLATE[14] = {
    0x03,       // Length of this Data
    0x03,       // AD Type: Service UUID List (16-bit)
    0x2C, 0xFE, // Service UUID: Google Fast Pair (0xFE2C) LSB first

    0x06,       // Length of this Data
    0x16,       // AD Type: Service Data (16-bit UUID)
    0x2C, 0xFE, // Service UUID: Google Fast Pair (0xFE2C) LSB first
    0x00, 0x00, 0x00, // Model ID Placeholder (Bytes 8, 9, 10) MSB first

    0x02,       // Length of this Data
    0x0A,       // AD Type: TX Power Level
    0x00        // TX Power Placeholder (Byte 13)
};
const size_t BleAdvertiser::FAST_PAIR_PACKET_SIZE = sizeof(BleAdvertiser::FAST_PAIR_PACKET_TEMPLATE);

// --- Logging Tag ---
static const char* TAG_BLE_ADV = "BleAdvertiser"; // Tag for ESP_LOG


// --- Constructor ---
BleAdvertiser::BleAdvertiser(uint32_t durationMs) :
    advertisingDurationMs(durationMs),
    currentMode(AdvertisingMode::BOTH_RANDOM), // Default mode
    initialized(false)
{
    // Initialize advertising parameters struct
    advParams = {}; // Zero initialize
    advParams.adv_int_min       = 0x20; // 20ms interval min (32 * 0.625ms)
    advParams.adv_int_max       = 0x40; // 40ms interval max (64 * 0.625ms)
    advParams.adv_type          = ADV_TYPE_IND; // Default type (can be changed per packet)
    advParams.own_addr_type     = BLE_ADDR_TYPE_RANDOM;
    advParams.channel_map       = ADV_CHNL_ALL;
    advParams.adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY;
    // peer_addr and peer_addr_type are zeroed, direct_addr_type defaults ok
}

// --- begin ---
bool BleAdvertiser::begin(const char* deviceName /*= "ESP32_Advertiser"*/) {
    if (initialized) {
        return true;
    }

    esp_err_t ret;

    // Initialize NVS - Required by BLE stack
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret); // Check for other NVS errors

    // Release classic BT memory (optional, saves RAM if only BLE is used)
    // ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    // Initialize Bluetooth controller
    esp_bt_controller_config_t  bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_BLE_ADV, "Initialize controller failed: %s", esp_err_to_name(ret));
        return false;
    }

    // Enable Bluetooth controller (BLE mode)
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_BLE_ADV, "Enable controller failed: %s", esp_err_to_name(ret));
        // Consider calling esp_bt_controller_deinit(&cfg) here on failure
        return false;
    }

    // Initialize Bluedroid stack (needed for GAP API)
    ret = esp_bluedroid_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_BLE_ADV, "Init Bluedroid failed: %s", esp_err_to_name(ret));
        // Consider controller deinit/disable
        return false;
    }

    // Enable Bluedroid stack
    ret = esp_bluedroid_enable();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_BLE_ADV, "Enable Bluedroid failed: %s", esp_err_to_name(ret));
        // Consider bluedroid deinit / controller deinit/disable
        return false;
    }

    // Set max TX power (example, adjust if needed)
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9); // Max Power for Advertising
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9); // Set default as well

    // Note: deviceName is not explicitly set when using raw GAP advertising this way.
    // It *could* be added as a separate AD element in the raw data if needed.

    ESP_LOGI(TAG_BLE_ADV, "BLE Initialized Successfully");
    initialized = true;
    return true;
}

// --- setAdvertisingMode ---
void BleAdvertiser::setAdvertisingMode(AdvertisingMode mode) {
    currentMode = mode;
}

// --- setAdvertisingDuration ---
void BleAdvertiser::setAdvertisingDuration(uint32_t durationMs) {
    advertisingDurationMs = durationMs;
}

// --- generateRandomNRPA ---
// Generates a Non-Resolvable Private Address (first byte MSb = 00)
// Note: The Fast Pair example uses 0xF0 (1111....), which is technically incorrect for NRPA.
// It creates a Static address with high bits set. We'll follow the example for compatibility.
void BleAdvertiser::generateRandomNRPA(esp_bd_addr_t& addr) {
     for (int i = 0; i < 6; i++) {
        addr[i] = esp_random() % 256; // Use ESP-IDF random source
     }
     // Set high bits as per Fast Pair example (Static Address)
     addr[0] |= 0xC0; // Set two most significant bits (11), standard for static random
     // addr[0] |= 0xF0; // As per the example code, potentially non-standard but maybe required? Let's stick to example.
     addr[0] |= 0xF0; // Keep consistent with provided Fast Pair code's intent
}

// --- advertiseAppleProximityRaw ---
void BleAdvertiser::advertiseAppleProximityRaw() {
    if (!initialized) return;

    esp_err_t ret;
    esp_bd_addr_t random_addr;
    generateRandomNRPA(random_addr);

    // Set the random address for this advertising cycle
    ret = esp_ble_gap_set_rand_addr(random_addr);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_BLE_ADV, "Apple: Set random address failed: %s", esp_err_to_name(ret));
        return;
    }

    // Prepare the raw advertising data payload
    uint8_t raw_adv_data[31]; // Max BLE legacy advertising payload size
    size_t payload_len = 0;

    // Randomly pick device type and data
    int device_type_choice = random(2); // 0 = Long (DEVICES), 1 = Short (SHORT_DEVICES)

    if (device_type_choice == 0 && NUM_DEVICES > 0) {
        int index = random(NUM_DEVICES);
        payload_len = DEVICE_PAYLOAD_SIZE;
        memcpy(raw_adv_data, DEVICES[index], payload_len);
    } else if (NUM_SHORT_DEVICES > 0) {
        int index = random(NUM_SHORT_DEVICES);
        payload_len = SHORT_DEVICE_PAYLOAD_SIZE;
        memcpy(raw_adv_data, SHORT_DEVICES[index], payload_len);
    } else {
        ESP_LOGW(TAG_BLE_ADV, "Apple: No advertising data available.");
        return; // Nothing to advertise
    }

    // Ensure payload doesn't exceed max length (shouldn't with current data)
    if (payload_len > sizeof(raw_adv_data)) {
        ESP_LOGW(TAG_BLE_ADV, "Apple: Payload too long (%d bytes)", payload_len);
        payload_len = sizeof(raw_adv_data);
    }

    // Configure the raw advertising data
    ret = esp_ble_gap_config_adv_data_raw(raw_adv_data, payload_len);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_BLE_ADV, "Apple: Config raw adv data failed: %s", esp_err_to_name(ret));
        return;
    }

    // Scan Response Data (Not typically used for this type of beacon, set empty)
    ret = esp_ble_gap_config_scan_rsp_data_raw(NULL, 0);
     if (ret != ESP_OK) {
        ESP_LOGE(TAG_BLE_ADV, "Apple: Config raw scan response data failed: %s", esp_err_to_name(ret));
        // Continue anyway, scan response not critical here
    }


    // Set advertising parameters (potentially change type randomly like original)
    esp_ble_adv_params_t currentAdvParams = advParams; // Copy base params
    int adv_type_choice = random(3);
    if (adv_type_choice == 0) currentAdvParams.adv_type = ADV_TYPE_IND;
    else if (adv_type_choice == 1) currentAdvParams.adv_type = ADV_TYPE_SCAN_IND;
    else currentAdvParams.adv_type = ADV_TYPE_NONCONN_IND; // Non-connectable might be best

    // Start advertising
    ret = esp_ble_gap_start_advertising(&currentAdvParams);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_BLE_ADV, "Apple: Start advertising failed: %s", esp_err_to_name(ret));
        // Don't stop if it didn't start
        return;
    }

    // Advertise for the specified duration
    // Use FreeRTOS delay for better multitasking if called from a task
    // Use Arduino delay otherwise
    #ifdef ESP_PLATFORM // Check if running in ESP-IDF context (likely true for Arduino-ESP32)
        vTaskDelay(pdMS_TO_TICKS(advertisingDurationMs));
    #else
        delay(advertisingDurationMs);
    #endif


    // Stop advertising
    ret = esp_ble_gap_stop_advertising();
    if (ret != ESP_OK) {
        // This might happen if advertising stopped due to timeout or other reasons
        ESP_LOGW(TAG_BLE_ADV, "Apple: Stop advertising failed: %s", esp_err_to_name(ret));
    }
}


// --- advertiseFastPairRaw ---
void BleAdvertiser::advertiseFastPairRaw() {
    if (!initialized || NUM_FAST_PAIR_MODELS == 0) return;

    esp_err_t ret;
    esp_bd_addr_t random_addr;
    generateRandomNRPA(random_addr);

    // Set the random address for this advertising cycle
    ret = esp_ble_gap_set_rand_addr(random_addr);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_BLE_ADV, "FastPair: Set random address failed: %s", esp_err_to_name(ret));
        return;
    }

    // Prepare the raw advertising data payload by modifying the template
    uint8_t current_fp_packet[FAST_PAIR_PACKET_SIZE];
    memcpy(current_fp_packet, FAST_PAIR_PACKET_TEMPLATE, FAST_PAIR_PACKET_SIZE);

    // Select random model ID
    int index = esp_random() % NUM_FAST_PAIR_MODELS;
    uint32_t model_id = FAST_PAIR_MODELS[index];

    // Inject Model ID into the packet (Bytes 8, 9, 10) - MSB First
    current_fp_packet[8] = (model_id >> 16) & 0xFF; // MSB
    current_fp_packet[9] = (model_id >> 8) & 0xFF;
    current_fp_packet[10] = model_id & 0xFF;        // LSB

    // Inject random TX Power into the packet (Byte 13)
    // Range: -100 to +20 dBm (as per example)
    current_fp_packet[13] = (int8_t)((esp_random() % 121) - 100);

    // Configure the raw advertising data
    ret = esp_ble_gap_config_adv_data_raw(current_fp_packet, FAST_PAIR_PACKET_SIZE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_BLE_ADV, "FastPair: Config raw adv data failed: %s", esp_err_to_name(ret));
        return;
    }

    // Scan Response Data (Not used for Fast Pair beaconing)
    ret = esp_ble_gap_config_scan_rsp_data_raw(NULL, 0);
     if (ret != ESP_OK) {
        ESP_LOGE(TAG_BLE_ADV, "FastPair: Config raw scan response data failed: %s", esp_err_to_name(ret));
        // Continue anyway
    }

    // Set advertising parameters (Use the class default ADV_TYPE_IND for Fast Pair)
    esp_ble_adv_params_t currentAdvParams = advParams;
    currentAdvParams.adv_type = ADV_TYPE_IND; // Fast pair usually uses connectable indication


    // Start advertising
    ret = esp_ble_gap_start_advertising(&currentAdvParams);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_BLE_ADV, "FastPair: Start advertising failed: %s", esp_err_to_name(ret));
        return; // Don't stop if it didn't start
    }

    // Advertise for the specified duration
    #ifdef ESP_PLATFORM
        vTaskDelay(pdMS_TO_TICKS(advertisingDurationMs));
    #else
        delay(advertisingDurationMs);
    #endif

    // Stop advertising
    ret = esp_ble_gap_stop_advertising();
     if (ret != ESP_OK) {
        ESP_LOGW(TAG_BLE_ADV, "FastPair: Stop advertising failed: %s", esp_err_to_name(ret));
    }
}

// --- performAdvertisingCycle ---
void BleAdvertiser::performAdvertisingCycle() {
     if (!initialized) {
        ESP_LOGE(TAG_BLE_ADV, "Advertiser not initialized.");
        delay(1000); // Prevent spamming logs if called in loop before successful begin()
        return;
    }

    switch (currentMode) {
        case AdvertisingMode::APPLE_PROXIMITY:
            advertiseAppleProximityRaw();
            break;
        case AdvertisingMode::GOOGLE_FAST_PAIR:
            advertiseFastPairRaw();
            break;
        case AdvertisingMode::BOTH_RANDOM:
            if (random(2) == 0) {
                 advertiseAppleProximityRaw();
            } else {
                 advertiseFastPairRaw();
            }
            break;
    }
     // Add a small optional delay *between* advertising bursts if needed
     // vTaskDelay(pdMS_TO_TICKS(10)); // e.g., 10ms delay

     Serial.println("BLE");
}