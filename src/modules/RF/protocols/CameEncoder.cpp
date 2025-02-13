// #include "CameEncoder.h"
// #include "CameDecoder.h"

// std::vector<long long int> CameEncoder::encode(uint64_t key, uint32_t bit_count) const {
//     std::vector<long long int> pulses;

//     // Preamble: low pulse of TE_SHORT*56 then high pulse of TE_SHORT.
//     pulses.push_back(-static_cast<long long int>(CameDecoder::TE_SHORT * 56));
//     pulses.push_back(static_cast<long long int>(CameDecoder::TE_SHORT));

//     // Encode bits (MSB first)
//     for (int i = bit_count - 1; i >= 0; i--) {
//         uint8_t bit = (key >> i) & 0x1;
//         if (bit == 0) {
//             // Bit 0: low pulse = TE_SHORT, high pulse = TE_LONG.
//             pulses.push_back(-static_cast<long long int>(CameDecoder::TE_SHORT));
//             pulses.push_back(static_cast<long long int>(CameDecoder::TE_LONG));
//         } else {
//             // Bit 1: low pulse = TE_LONG, high pulse = TE_SHORT.
//             pulses.push_back(-static_cast<long long int>(CameDecoder::TE_LONG));
//             pulses.push_back(static_cast<long long int>(CameDecoder::TE_SHORT));
//         }
//     }

//     // Termination: low pulse with duration >= TE_SHORT*4.
//     pulses.push_back(-static_cast<long long int>(CameDecoder::TE_SHORT * 4));

//     return pulses;
// }
