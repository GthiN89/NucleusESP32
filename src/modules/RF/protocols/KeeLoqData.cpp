#include "KeeLoqData.hpp"
#include <cstdio>  // For snprintf
#include <vector>  // Include vector for snprintf usage (sometimes needed)

// Define other methods like constructor, updateDerivedPartsFromData, serialize, deserialize here if not inline...

// --- KeeLoqData::getCodeString Implementation ---

std::string KeeLoqData::getCodeString() const {
    char buf[256]; // Buffer for formatting

    // Prepare data for printing
    uint32_t data_hi = static_cast<uint32_t>(data >> 32);
    uint32_t data_lo = static_cast<uint32_t>(data & 0xFFFFFFFFULL);

    // Reverse the raw key data for display
    uint64_t reversed_key = KeeLoqData::reverseBits(data);
    uint32_t rev_key_hi = static_cast<uint32_t>(reversed_key >> 32);
    uint32_t rev_key_lo = static_cast<uint32_t>(reversed_key & 0xFFFFFFFFULL);

    // Note: fix_part and hop_part are already calculated based on the *reversed* key
    // in updateDerivedPartsFromData(). So fix_part corresponds to rev_key_hi,
    // and hop_part corresponds to rev_key_lo.

    // Choose how to display the counter based on manufacturer context
    const char* cnt_str;
    char cnt_buf[6]; // Buffer for numeric counter "nnnnn\0"
    // If MF is unknown or specific non-decrypted types, counter is uncertain
    if (manufacturer_name == "Unknown" || manufacturer_name == "AN-Motors" || manufacturer_name == "HCS101") {
         // Or if learning_type is Unknown and MF isn't one of the non-encrypted ones
        cnt_str = "????";
    } else {
        // Otherwise, display the stored counter value (might be 0 if decryption failed)
        snprintf(cnt_buf, sizeof(cnt_buf), "%u", cnt); // Use %u for uint16_t
        cnt_str = cnt_buf;
    }

    // Format the main string
    // Using snprintf for safety
    int offset = snprintf(buf, sizeof(buf),
             "%s %dbit\r\n"
             "Key:%08lX%08lX\r\n" // Raw data (as received/loaded)
             "Fix:0x%08lX    Cnt:%s\r\n"  // Fix part (from reversed key), Counter
             "Hop:0x%08lX    Btn:%X\r\n"   // Hop part (from reversed key), Button
             "MF:%s",
             "KeeLoq", // Use specific name
             data_count_bit,
             data_hi, data_lo,         // Raw key high/low
             fix_part, cnt_str,        // Fix part, Counter string
             hop_part, btn,            // Hop part, Button (hex)
             manufacturer_name.c_str() // Manufacturer name
    );

    // Add Seed if relevant (e.g., for BFT or if non-zero)
    if (manufacturer_name == "BFT" || seed != 0) {
         // Check remaining buffer space before appending
         if (offset < sizeof(buf) - 20) { // Need space for " Sd:0x........\0"
             snprintf(buf + offset, sizeof(buf) - offset, " Sd:%08lX", seed);
         }
    }

    // Could add reversed key for debugging if desired:
    // snprintf(buf + offset, sizeof(buf) - offset, "\r\nRev:%08lX%08lX", rev_key_hi, rev_key_lo);


    return std::string(buf); // Convert C-string buffer to std::string
}


