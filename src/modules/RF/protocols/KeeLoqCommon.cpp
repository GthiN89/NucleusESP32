#include "KeeLoqCommon.hpp" // Include the header file

namespace KeeLoqCommon {

// --- Core Encryption/Decryption Function Definitions ---


uint32_t encrypt(uint32_t data, uint64_t key) { // Or inline definition
    uint32_t x = data;
    for (int r = 0; r < 528; ++r) {
        uint32_t nlf_lookup_index = g5(x, 1, 9, 20, 26, 31); // g5 already uses get_bit32
        // V V V Use new function names V V V
        bool feedback = get_bit32(x, 0) ^
                        get_bit32(x, 16) ^
                        get_bit64(key, r & 63) ^ // Use get_bit64 for key access
                        get_bit32(KEELOQ_NLF, nlf_lookup_index); // Use get_bit32 for NLF access

        x = (x >> 1) | (static_cast<uint32_t>(feedback) << 31);
    }
    return x;
}

uint32_t decrypt(uint32_t data, uint64_t key) { // Or inline definition
    uint32_t x = data;
    for (int r = 0; r < 528; ++r) {
        uint32_t nlf_lookup_index = g5(x, 0, 8, 19, 25, 30); // g5 already uses get_bit32
        // V V V Use new function names V V V
        bool feedback = get_bit32(x, 15) ^
                        get_bit32(x, 31) ^
                        get_bit64(key, (15 - r) & 63) ^ // Use get_bit64 for key access
                        get_bit32(KEELOQ_NLF, nlf_lookup_index); // Use get_bit32 for NLF access

        x = (x << 1) | static_cast<uint32_t>(feedback);
    }
    return x;
}

// --- Key Learning Algorithm Function Definitions ---

uint64_t normal_learning(uint32_t serial, uint64_t mf_key) {
    uint32_t k1, k2;
    uint32_t masked_serial = serial & 0x0FFFFFFF;

    // Decrypt with FUNC = 1
    k1 = decrypt(masked_serial | 0x20000000, mf_key);

    // Decrypt with FUNC = 3
    k2 = decrypt(masked_serial | 0x60000000, mf_key);

    return (static_cast<uint64_t>(k2) << 32) | k1;
}

uint64_t secure_learning(uint32_t serial, uint32_t seed, uint64_t mf_key) {
    uint32_t k1, k2;
    uint32_t masked_serial = serial & 0x0FFFFFFF;

    // Decrypt serial (FUNC = 0 implied)
    k1 = decrypt(masked_serial, mf_key);

    // Decrypt seed (FUNC = 0 implied)
    k2 = decrypt(seed, mf_key);

    return (static_cast<uint64_t>(k1) << 32) | k2;
}

uint64_t magic_xor_type1_learning(uint32_t serial, uint64_t xor_key) {
    uint32_t masked_serial = serial & 0x0FFFFFFF;
    return ((static_cast<uint64_t>(masked_serial) << 32) | masked_serial) ^ xor_key;
}

uint64_t faac_learning(uint32_t seed, uint64_t mf_key) {
    uint16_t hs = static_cast<uint16_t>(seed >> 16);
    const uint16_t ending = 0x544D;
    uint32_t lsb_part = (static_cast<uint32_t>(hs) << 16) | ending;

    uint32_t high_key_part = encrypt(seed, mf_key);
    uint32_t low_key_part = encrypt(lsb_part, mf_key);

    return (static_cast<uint64_t>(high_key_part) << 32) | low_key_part;
}

uint64_t magic_serial_type1_learning(uint32_t serial, uint64_t magic_man) {
     uint32_t masked_serial = serial & 0x0FFFFFFF;
     uint64_t serial_part = static_cast<uint64_t>(masked_serial) << 40;
     uint8_t byte0 = masked_serial & 0xFF;
     uint8_t byte1 = (masked_serial >> 8) & 0xFF;
     uint64_t checksum_part = static_cast<uint64_t>(static_cast<uint8_t>(byte0 + byte1)) << 32;

    return (magic_man & 0xFFFFFFFFULL) | serial_part | checksum_part;
}

uint64_t magic_serial_type2_learning(uint32_t serial_and_btn, uint64_t magic_man) {
    // Use reinterpret_cast carefully
    uint8_t* p_data = reinterpret_cast<uint8_t*>(&serial_and_btn);
    uint8_t* p_man = reinterpret_cast<uint8_t*>(&magic_man);

    // Original C code mapping: p[0]->m[7], p[1]->m[6], p[2]->m[5], p[3]->m[4]
    p_man[7] = p_data[0];
    p_man[6] = p_data[1];
    p_man[5] = p_data[2];
    p_man[4] = p_data[3];

    return magic_man; // Return the modified value
}

uint64_t magic_serial_type3_learning(uint32_t serial24, uint64_t magic_man) {
    return (magic_man & 0xFFFFFFFFFF000000ULL) | (serial24 & 0x00FFFFFFUL);
}


} // namespace KeeLoqCommon