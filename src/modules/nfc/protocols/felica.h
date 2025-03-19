// In felica.h, update the declarations to match the definitions:

#ifndef FELICA_H
#define FELICA_H

#include <Arduino.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <aes/esp_aes.h> 

//––– Macros –––
#ifndef CHECK
#define CHECK(x) if (!(x)) { Serial.print("Assertion failed: "); Serial.println(#x); while(1); }
#endif
#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

//––– Constants –––
#define FELICA_IDM_SIZE           8
#define FELICA_PMM_SIZE           8
#define FELICA_DATA_BLOCK_SIZE    16
#define FELICA_BLOCK_INDEX_MAC_A  0xAA  // Dummy value; adjust as needed

//––– Type definitions –––
typedef struct {
  uint8_t data[FELICA_IDM_SIZE];
} FelicaIDm;

typedef struct {
  uint8_t data[FELICA_PMM_SIZE];
} FelicaPMm;

typedef struct {
  uint8_t dump[256];
} FelicaDataDump;

typedef struct {
  FelicaIDm idm;
  FelicaPMm pmm;
  uint8_t blocks_total;
  uint8_t blocks_read;
  FelicaDataDump data;
} FelicaData;

//–––  Format Replacement –––
typedef struct {
  String data;
}  Format;

//––– Device name type –––
typedef enum {
  NFC_DEVICE_NAME_TYPE_SHORT,
  NFC_DEVICE_NAME_TYPE_LONG
} NfcDeviceNameType;

//––– Function prototypes –––
FelicaData* felica_alloc(void);
void felica_free(FelicaData* data);
void felica_reset(FelicaData* data);
void felica_copy(FelicaData* data, const FelicaData* other);
bool felica_verify(FelicaData* data, const String& device_type);
bool felica_load(FelicaData* data,  Format* ff, uint32_t version);
bool felica_save(const FelicaData* data,  Format* ff);
bool felica_is_equal(const FelicaData* data, const FelicaData* other);
const char* felica_get_device_name(const FelicaData* data, NfcDeviceNameType name_type);
const uint8_t* felica_get_uid(const FelicaData* data, size_t* uid_len);
bool felica_set_uid(FelicaData* data, const uint8_t* uid, size_t uid_len);
FelicaData* felica_get_base_data(const FelicaData* data);

// AES‐based encryption functions replacing DES3
bool felica_calculate_session_key_aes(esp_aes_context* ctx,
                                  const uint8_t* ck,
                                  const uint8_t* rc,
                                  uint8_t* out);
bool felica_check_mac(esp_aes_context* ctx,
                      const uint8_t* session_key,
                      const uint8_t* rc,
                      const uint8_t* blocks,
                      const uint8_t block_count,
                      uint8_t* data);
void felica_calculate_mac_read(esp_aes_context* ctx,
                               const uint8_t* session_key,
                               const uint8_t* rc,
                               const uint8_t* blocks,
                               const uint8_t block_count,
                               const uint8_t* data,
                               uint8_t* mac);
void felica_calculate_mac_write(esp_aes_context* ctx,
                                const uint8_t* session_key,
                                const uint8_t* rc,
                                const uint8_t* wcnt,
                                const uint8_t* data,
                                uint8_t* mac);

#endif  // FELICA_H
