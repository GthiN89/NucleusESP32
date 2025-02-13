// #include "HormannHSMEncoder.h"

// std::vector<long long int> HormannHSMEncoder::encode(uint64_t key, uint32_t bit_count) const {
//     std::vector<long long int> pulses;

//     // Preamble: high pulse (TE_SHORT * 24) then low pulse (TE_SHORT)
//     pulses.push_back(static_cast<long long int>(HormannHSMDecoder::TE_SHORT * 24));
//     pulses.push_back(-static_cast<long long int>(HormannHSMDecoder::TE_SHORT));

//     // Encode bits (MSB first)
//     for (int i = bit_count - 1; i >= 0; i--) {
//         uint8_t bit = (key >> i) & 0x1;
//         if (bit == 0) {
//             pulses.push_back(static_cast<long long int>(HormannHSMDecoder::TE_SHORT));  // high pulse for '0'
//             pulses.push_back(-static_cast<long long int>(HormannHSMDecoder::TE_LONG));   // low pulse for '0'
//         } else {
//             pulses.push_back(static_cast<long long int>(HormannHSMDecoder::TE_LONG));   // high pulse for '1'
//             pulses.push_back(-static_cast<long long int>(HormannHSMDecoder::TE_SHORT));  // low pulse for '1'
//         }
//     }

//     // Termination: high pulse of duration >= TE_SHORT * 5.
//     pulses.push_back(static_cast<long long int>(HormannHSMDecoder::TE_SHORT * 5));

//     return pulses;
// }
