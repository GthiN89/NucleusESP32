// In felica.cpp, update definitions to use esp_aes_context* consistently:

#include "felica.h"

//––– Helper: Reverse-copy a block (for DES3 this was 8 bytes; for AES we reverse 16 bytes) –––
static void reverse_block(const uint8_t* in, uint8_t* out, size_t len) {
    for (size_t i = 0; i < len; i++) {
        out[i] = in[len - 1 - i];
    }
}


//––– Dummy Macros and Stubs –––
#define  check(x)   if(!(x)) { while(1); }
#define UNUSED(x)       (void)(x)
#define NFC_UNIFIED_FORMAT_VERSION 1

// Dummy implementations of the format read/write functions.
bool format_read_uint32( Format* ff, const char* key, uint32_t* value, int count) {
  UNUSED(ff); UNUSED(key); UNUSED(count);
  *value = 1;  // dummy value
  return true;
}
bool format_read_hex( Format* ff, const char* key, uint8_t* buffer, size_t length) {
  UNUSED(ff); UNUSED(key);
  memset(buffer, 0xAB, length); // fill with dummy data
  return true;
}
bool format_write_uint32( Format* ff, const char* key, const uint32_t* value, int count) {
  UNUSED(ff); UNUSED(key); UNUSED(value); UNUSED(count);
  return true;
}
bool format_write_hex( Format* ff, const char* key, const uint8_t* buffer, size_t length) {
  UNUSED(ff); UNUSED(key); UNUSED(buffer); UNUSED(length);
  return true;
}
bool format_write_comment_cstr( Format* ff, const char* comment) {
  UNUSED(ff); UNUSED(comment);
  return true;
}


/**
 * felica_calculate_session_key_aes()
 *
 * Uses ESP-IDF AES CBC encryption.
 */
bool felica_calculate_session_key_aes(esp_aes_context* ctx,
                                      const uint8_t* ck,
                                      const uint8_t* rc,
                                      uint8_t* out) {
    CHECK(ctx); CHECK(ck); CHECK(rc); CHECK(out);
    uint8_t iv[16] = {0};
    if (esp_aes_setkey(ctx, ck, 128) != ESP_OK)
        return false;
    if (esp_aes_crypt_cbc(ctx, ESP_AES_ENCRYPT, FELICA_DATA_BLOCK_SIZE, iv, rc, out) != ESP_OK)
        return false;
    return true;
}

/**
 * felica_calculate_mac()
 *
 * Helper function that performs AES CBC encryption for MAC calculation.
 * Reverses the rc and first_block before processing.
 */
static bool felica_calculate_mac(esp_aes_context* ctx,
                                 const uint8_t* session_key,
                                 const uint8_t* rc,
                                 const uint8_t* first_block,
                                 const uint8_t* data,
                                 const size_t length,
                                 uint8_t* mac) {
    CHECK((length % 16) == 0);
    uint8_t reverse_data[16];
    uint8_t iv[16];
    uint8_t out[16];

    if (esp_aes_setkey(ctx, session_key, 128) != ESP_OK)
        return false;

    reverse_block(rc, iv, 16);
    reverse_block(first_block, reverse_data, 16);

    size_t i = 0;
    bool error = false;
    do {
        if (esp_aes_crypt_cbc(ctx, ESP_AES_ENCRYPT, 16, iv, reverse_data, out) == ESP_OK) {
            memcpy(iv, out, 16);
            reverse_block(data + i, reverse_data, 16);
            i += 16;
        } else {
            error = true;
            break;
        }
    } while(i <= length);

    if (!error)
        reverse_block(out, mac, 16);
    return !error;
}

void felica_calculate_mac_read(esp_aes_context* ctx,
                               const uint8_t* session_key,
                               const uint8_t* rc,
                               const uint8_t* blocks,
                               const uint8_t block_count,
                               const uint8_t* data,
                               uint8_t* mac) {
  CHECK(ctx); CHECK(session_key); CHECK(rc); CHECK(blocks); CHECK(data); CHECK(mac);
  uint8_t first_block[16];
  memset(first_block, 0xFF, 16);
  for(uint8_t i = 0, j = 0; i < block_count && (j + 1) < 16; i++, j += 2) {
    first_block[j] = blocks[i];
    first_block[j + 1] = 0;
  }
  size_t data_size_without_mac = FELICA_DATA_BLOCK_SIZE * (block_count - 1);
  felica_calculate_mac(ctx, session_key, rc, first_block, data, data_size_without_mac, mac);
}

void felica_calculate_mac_write(esp_aes_context* ctx,
                                const uint8_t* session_key,
                                const uint8_t* rc,
                                const uint8_t* wcnt,
                                const uint8_t* data,
                                uint8_t* mac) {
  CHECK(ctx); CHECK(session_key); CHECK(rc); CHECK(wcnt); CHECK(data); CHECK(mac);
  const uint8_t WCNT_length = 3;
  uint8_t block_data[WCNT_length + 1];
  uint8_t first_block[16];
  memcpy(block_data, wcnt, WCNT_length);
  block_data[3] = FELICA_BLOCK_INDEX_MAC_A;  // Use predefined index value
  memset(first_block, 0, 16);
  first_block[0] = block_data[0];
  first_block[1] = block_data[1];
  first_block[2] = block_data[2];
  first_block[4] = block_data[3];
  first_block[6] = FELICA_BLOCK_INDEX_MAC_A;
  uint8_t session_swapped[FELICA_DATA_BLOCK_SIZE];
  // Swap the first and second halves of the session key (for demonstration)
  memcpy(session_swapped, session_key + 8, 8);
  memcpy(session_swapped + 8, session_key, 8);
  felica_calculate_mac(ctx, session_swapped, rc, first_block, data, FELICA_DATA_BLOCK_SIZE, mac);
}


#include "felica.h"

FelicaData* felica_alloc(void) {
    FelicaData* data = (FelicaData*)malloc(sizeof(FelicaData));
    if (data) felica_reset(data);
    return data;
}

void felica_free(FelicaData* data) {
    free(data);
}

void felica_reset(FelicaData* data) {
    memset(data, 0, sizeof(FelicaData));
}

bool felica_set_uid(FelicaData* data, const uint8_t* uid, size_t uid_len) {
    if(uid_len != FELICA_IDM_SIZE)
        return false;
    memcpy(data->idm.data, uid, uid_len);
    return true;
}

const uint8_t* felica_get_uid(const FelicaData* data, size_t* uid_len) {
    if(uid_len) *uid_len = FELICA_IDM_SIZE;
    return data->idm.data;
}

bool felica_load(FelicaData* data,  Format* ff, uint32_t version) {
  bool parsed = false;
  do {
    if (version < NFC_UNIFIED_FORMAT_VERSION) {
      Serial.println("Version too low");
      break;
    }
    uint32_t data_format_version = 0;
    Serial.print("Data format version: ");
    if (!format_read_uint32(ff, "Data format version", &data_format_version, 1)) {
      Serial.println("Failed to read");
      break;
    }
    Serial.println(data_format_version);
    if (data_format_version != 1) {
      Serial.println("Unsupported data format version");
      break;
    }
    Serial.print("Manufacture id: ");
    if (!format_read_hex(ff, "Manufacture id", data->idm.data, FELICA_IDM_SIZE)) {
      Serial.println("Failed to read");
      break;
    }
    for (uint8_t i = 0; i < FELICA_IDM_SIZE; i++) {
      if (data->idm.data[i] < 0x10)
        Serial.print("0");
      Serial.print(data->idm.data[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
    Serial.print("Manufacture parameter: ");
    if (!format_read_hex(ff, "Manufacture parameter", data->pmm.data, FELICA_PMM_SIZE)) {
      Serial.println("Failed to read");
      break;
    }
    for (uint8_t i = 0; i < FELICA_PMM_SIZE; i++) {
      if (data->pmm.data[i] < 0x10)
        Serial.print("0");
      Serial.print(data->pmm.data[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
    parsed = true;
    uint32_t blocks_total = 0;
    uint32_t blocks_read = 0;
    Serial.print("Blocks total: ");
    if (!format_read_uint32(ff, "Blocks total", &blocks_total, 1)) {
      Serial.println("Failed to read");
      break;
    }
    Serial.println(blocks_total);
    Serial.print("Blocks read: ");
    if (!format_read_uint32(ff, "Blocks read", &blocks_read, 1)) {
      Serial.println("Failed to read");
      break;
    }
    Serial.println(blocks_read);
    data->blocks_total = (uint8_t)blocks_total;
    data->blocks_read = (uint8_t)blocks_read;
    for (uint8_t i = 0; i < data->blocks_total; i++) {
      Serial.print("Block ");
      Serial.print(i);
      Serial.print(": ");
      for (uint8_t j = 0; j < FELICA_DATA_BLOCK_SIZE; j++) {
        uint8_t byteVal = data->data.dump[i * FELICA_DATA_BLOCK_SIZE + j];
        if (byteVal < 0x10)
          Serial.print("0");
        Serial.print(byteVal, HEX);
        Serial.print(" ");
      }
      Serial.println();
    }
  } while (false);
  return parsed;
}


bool felica_save(const FelicaData* data,  Format* ff) {
  if (!data) return false;
  bool saved = true;
  // Print header comment
  Serial.println("FeliCa specific data");
  
  // Print data format version
  uint32_t data_format_version = 1;
  Serial.print("Data format version: ");
  Serial.println(data_format_version);
  
  // Print Manufacture id
  Serial.print("Manufacture id: ");
  for (uint8_t i = 0; i < FELICA_IDM_SIZE; i++) {
    if (data->idm.data[i] < 0x10)
      Serial.print("0");
    Serial.print(data->idm.data[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  
  // Print Manufacture parameter
  Serial.print("Manufacture parameter: ");
  for (uint8_t i = 0; i < FELICA_PMM_SIZE; i++) {
    if (data->pmm.data[i] < 0x10)
      Serial.print("0");
    Serial.print(data->pmm.data[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  
  // Print blocks total and blocks read
  uint32_t blocks_total = data->blocks_total;
  uint32_t blocks_read = data->blocks_read;
  Serial.print("Blocks total: ");
  Serial.println(blocks_total);
  Serial.print("Blocks read: ");
  Serial.println(blocks_read);
  
  // Print each data block in HEX
  for (uint8_t i = 0; i < blocks_total; i++) {
    Serial.print("Block ");
    Serial.print(i);
    Serial.print(": ");
    for (uint8_t j = 0; j < FELICA_DATA_BLOCK_SIZE; j++) {
      uint8_t byteVal = data->data.dump[i * FELICA_DATA_BLOCK_SIZE + j];
      if (byteVal < 0x10)
        Serial.print("0");
      Serial.print(byteVal, HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
  return saved;
}

