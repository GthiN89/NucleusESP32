#ifndef KEELOQ_COMMON_HPP
#define KEELOQ_COMMON_HPP

#include <cstdint> // For uint32_t, uint64_t

namespace KeeLoqCommon {

// KeeLoq Non-Linear Feedback Function constant
constexpr uint32_t KEELOQ_NLF = 0x3A5C742E;

// KeeLoq learning types enumeration
enum class KeeLoqLearningType : uint8_t {
    Unknown = 0,
    Simple = 1,
    Normal = 2,
    Secure = 3,
    MagicXorType1 = 4,
    Faac = 5,
    MagicSerialType1 = 6,
    MagicSerialType2 = 7,
    MagicSerialType3 = 8
};

// --- Helper Functions (Defined inline in header for potential optimization) ---

/**
 * @brief Extracts the nth bit from a 64-bit value. (Renamed)
 */
inline bool get_bit64(uint64_t x, unsigned int n) { // RENAMED
    if (n >= 64) return false;
    return ((x >> n) & 1ULL) != 0;
}

/**
 * @brief Extracts the nth bit from a 32-bit value. (Renamed)
 */
inline bool get_bit32(uint32_t x, unsigned int n) { // RENAMED
    if (n >= 32) return false;
    return ((x >> n) & 1UL) != 0;
}

/**
 * @brief KeeLoq NLF lookup helper function. (Updated to use new names)
 */
inline uint32_t g5(uint32_t x, unsigned int a, unsigned int b, unsigned int c, unsigned int d, unsigned int e) {
    // V V V Use new function names V V V
    return (get_bit32(x, a) << 0) |
           (get_bit32(x, b) << 1) |
           (get_bit32(x, c) << 2) |
           (get_bit32(x, d) << 3) |
           (get_bit32(x, e) << 4);
}


// --- Core Encryption/Decryption Function Declarations ---
// (Declarations remain the same)
uint32_t encrypt(uint32_t data, uint64_t key);
uint32_t decrypt(uint32_t data, uint64_t key);

// --- Key Learning Algorithm Function Declarations ---
// (Declarations remain the same)
uint64_t normal_learning(uint32_t serial, uint64_t mf_key);
uint64_t secure_learning(uint32_t serial, uint32_t seed, uint64_t mf_key);
uint64_t magic_xor_type1_learning(uint32_t serial, uint64_t xor_key);
uint64_t faac_learning(uint32_t seed, uint64_t mf_key);
uint64_t magic_serial_type1_learning(uint32_t serial, uint64_t magic_man);
uint64_t magic_serial_type2_learning(uint32_t serial_and_btn, uint64_t magic_man);
uint64_t magic_serial_type3_learning(uint32_t serial24, uint64_t magic_man);


} // namespace KeeLoqCommon

#endif // KEELOQ_COMMON_HPP