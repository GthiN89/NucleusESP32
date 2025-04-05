// Add these defines at the top of the file or pass them as build flags
 #define DEBUG_KEELOQ_DECODER
 #define DEBUG_KEELOQ_ENCODER
 #define DEBUG_KEELOQ_DECODER_VERBOSE // For more detailed feed prints

#include "KeeLoqProtocol.hpp"
#include "KeeLoqCommon.hpp"
#include <vector>
#include <cassert>
#include <iostream> // For std::cerr (used as fallback if Serial missing, keep for now)
#include <cmath>
#include <string>

// Include Arduino header for Serial, only if compiling for Arduino targets
#if defined(ARDUINO)
  #include <Arduino.h>
#else
  // Provide fallback macros if not on Arduino (e.g., print to cerr)
  #define Serial std::cerr
  #define HEX 16
  #define DEC 10
  // Define println equivalents if needed
  namespace std {
      // Simple helper for fallback println
      template<typename T>
      void println(T val, int base = 10) {
          if (base == 16) cerr << std::hex;
          cerr << val << std::endl;
           if (base == 16) cerr << std::dec; // Reset to decimal
      }
       void println(const char* str) {
           cerr << str << std::endl;
       }
        void println() {
           cerr << std::endl;
       }
       // Add overloads as needed...
  }
  using namespace std; // Use std::println for fallback
#endif


// Helper macro for duration comparison
#define DURATION_DIFF(a, b) (std::abs(static_cast<int32_t>(a) - static_cast<int32_t>(b)))

// --- KeeLoqProtocolDecoder Implementation ---

KeeLoqProtocolDecoder::KeeLoqProtocolDecoder() {
    reset();
}

KeeLoqProtocolDecoder::~KeeLoqProtocolDecoder() {}

void KeeLoqProtocolDecoder::reset() {
#ifdef DEBUG_KEELOQ_DECODER
    Serial.println("KL_Dec: Resetting state.");
#endif
    parser_step_ = DecoderStep::Reset;
    header_count_ = 0;
    current_data_ = 0;
    decoded_bit_count_ = 0;
    last_duration_ = 0;
    has_valid_packet_ = false;
    last_decoded_packet_ = {};
}

bool KeeLoqProtocolDecoder::isDurationShort(uint32_t duration) const {
     return DURATION_DIFF(duration, TE_SHORT) < TE_DELTA;
}

bool KeeLoqProtocolDecoder::isDurationLong(uint32_t duration) const {
     return DURATION_DIFF(duration, TE_LONG) < TE_DELTA * 2;
}

void KeeLoqProtocolDecoder::addBit(bool bit) {
    if (decoded_bit_count_ < MIN_COUNT_BIT) {
         current_data_ = (current_data_ << 1) | bit;
    }
    if(decoded_bit_count_ < MAX_COUNT_BIT) {
        decoded_bit_count_++;
#ifdef DEBUG_KEELOQ_DECODER_VERBOSE
        Serial.print("KL_Dec: Added bit "); Serial.print(bit ? '1':'0');
        Serial.print(" (Count: "); Serial.print(decoded_bit_count_); Serial.println(")");
#endif
    } else {
#ifdef DEBUG_KEELOQ_DECODER
        Serial.println("KL_Dec: Error - Exceeded max bit count, resetting.");
#endif
        parser_step_ = DecoderStep::Reset;
    }
}


void KeeLoqProtocolDecoder::feed(bool level, uint32_t duration) {
    #ifdef DEBUG_KEELOQ_DECODER_VERBOSE
        Serial.print("KL_Dec Feed: L="); Serial.print(level); Serial.print(" D="); Serial.print(duration);
        Serial.print(" State="); Serial.print((int)parser_step_); Serial.print(" HdrCnt="); Serial.println(header_count_);
    #endif
        // Define the minimum acceptable preamble pulse pairs
        const uint16_t MIN_PREAMBLE_COUNT = 8;
    
        DecoderStep prev_step = parser_step_;
    
        switch (parser_step_) {
        case DecoderStep::Reset:
            // Look for the first Short High pulse of a potential preamble
            if (level && isDurationShort(duration)) {
                parser_step_ = DecoderStep::CheckPreambula; // Move to check preamble state
                last_duration_ = duration; // Store this high pulse duration
                header_count_ = 1;       // Start count at 1 pulse
                has_valid_packet_ = false;
            }
            // Ignore other pulses in Reset state
            break;
    
        case DecoderStep::CheckPreambula:
            // Expecting a low pulse (gap) after a high pulse
            if (!level) {
                // Check if the previous HIGH pulse was short (part of preamble)
                if (isDurationShort(last_duration_)) {
                    // Now check the current LOW gap duration
                    if (isDurationShort(duration)) {
                        // Valid Short Low gap -> Part of preamble confirmed.
                        // Stay in CheckPreambula state, wait for next HIGH pulse.
                        header_count_++; // Increment count of *full pairs* processed.
                        // We don't change state here, just wait for the next level=1 pulse.
    #ifdef DEBUG_KEELOQ_DECODER_VERBOSE
                        Serial.print("KL_Dec: Preamble pair OK, count = "); Serial.println(header_count_);
    #endif
                    }
                    // Check if this LOW gap is the long header gap
                    else if (header_count_ >= MIN_PREAMBLE_COUNT &&
                             DURATION_DIFF(duration, TE_SHORT * 10) < TE_DELTA * 10)
                    {
                        // Found Header Gap after enough preamble pairs!
                        parser_step_ = DecoderStep::SaveDuration; // Transition to data saving state
                        current_data_ = 0;
                        decoded_bit_count_ = 0;
    #ifdef DEBUG_KEELOQ_DECODER
                        Serial.print("KL_Dec: Header Gap Found after "); Serial.print(header_count_); Serial.println(" preamble pairs.");
    #endif
                    } else {
                        // Invalid gap duration -> Reset
    #ifdef DEBUG_KEELOQ_DECODER
                         if(header_count_ < MIN_PREAMBLE_COUNT) Serial.print("KL_Dec: Reset - Insufficient header pairs (");
                         else Serial.print("KL_Dec: Reset - Invalid gap after preamble (");
                         Serial.print(header_count_); Serial.print(" pairs, gap "); Serial.print(duration); Serial.println("us)");
    #endif
                        reset();
                    }
                } else {
                    // Previous high pulse was not short -> Reset (shouldn't happen if logic is right)
    #ifdef DEBUG_KEELOQ_DECODER
                     Serial.println("KL_Dec: Reset - Low gap received but previous High pulse was not Short.");
    #endif
                    reset();
                }
            }
            // Expecting a high pulse (start of next preamble pair) after a low gap
            else { // level is true
                 // Store this high pulse duration for the *next* check when the low gap arrives
                 last_duration_ = duration;
                 // Stay in CheckPreambula state.
                 // Do NOT increment header_count here, only after a complete pair (H+L) is validated.
            }
            break;
    
        // States SaveDuration and CheckDuration remain the same
        case DecoderStep::SaveDuration:
            if (level) {
                last_duration_ = duration;
                parser_step_ = DecoderStep::CheckDuration;
            } else {
    #ifdef DEBUG_KEELOQ_DECODER
                Serial.println("KL_Dec: Reset - Unexpected Low when expecting data pulse High.");
    #endif
                reset();
            }
            break;
    
        case DecoderStep::CheckDuration:
            if (!level) {
                 if (duration >= TE_SHORT * 30) { // Footer gap check
                     DecoderStep next_step = DecoderStep::Reset;
    #ifdef DEBUG_KEELOQ_DECODER
                     Serial.print("KL_Dec: Footer Gap Found ("); Serial.print(duration); Serial.println("us). Checking packet...");
    #endif
                     if (decoded_bit_count_ >= MIN_COUNT_BIT && decoded_bit_count_ <= MAX_COUNT_BIT) {
                         last_decoded_packet_.data = current_data_;
                         last_decoded_packet_.data_count_bit = MIN_COUNT_BIT;
                         last_decoded_packet_.updateDerivedPartsFromData();
                         has_valid_packet_ = true;
    #ifdef DEBUG_KEELOQ_DECODER
                          Serial.print("KL_Dec: Packet potentially valid ("); Serial.print(decoded_bit_count_); Serial.println(" bits). Ready for getResult().");
                         uint32_t high = (uint32_t)(last_decoded_packet_.data >> 32); uint32_t low = (uint32_t)(last_decoded_packet_.data & 0xFFFFFFFF);
                         Serial.print("  Raw Data: 0x"); Serial.print(high, HEX); Serial.println(low, HEX);
                         Serial.print("  Fix: 0x"); Serial.print(last_decoded_packet_.fix_part, HEX); Serial.print(" Hop: 0x"); Serial.println(last_decoded_packet_.hop_part, HEX);
    #endif
                     } else {
    #ifdef DEBUG_KEELOQ_DECODER
                          Serial.print("KL_Dec: Invalid bit count: "); Serial.print(decoded_bit_count_); Serial.println(". Packet discarded.");
    #endif
                         has_valid_packet_ = false;
                     }
                     header_count_ = 0;
                     parser_step_ = next_step;
    
                 } else if (isDurationShort(last_duration_) && isDurationLong(duration)) { // Manchester '1'
                    addBit(1);
                    parser_step_ = DecoderStep::SaveDuration;
                 } else if (isDurationLong(last_duration_) && isDurationShort(duration)) { // Manchester '0'
                    addBit(0);
                    parser_step_ = DecoderStep::SaveDuration;
                 } else { // Invalid data timing
    #ifdef DEBUG_KEELOQ_DECODER
                    Serial.print("KL_Dec: Reset - Invalid data timing. H="); Serial.print(last_duration_); Serial.print(" L="); Serial.print(duration); Serial.println("us");
    #endif
                    reset();
                 }
            } else { // Unexpected high pulse
    #ifdef DEBUG_KEELOQ_DECODER
                Serial.println("KL_Dec: Reset - Unexpected High when expecting data gap Low.");
    #endif
                reset();
            }
            break;
        } // End switch
    
    #ifdef DEBUG_KEELOQ_DECODER_VERBOSE
        if(parser_step_ != prev_step) {
            Serial.print("KL_Dec: State -> "); Serial.println((int)parser_step_);
        }
    #endif
    }
bool KeeLoqProtocolDecoder::hasResult() const {
    // Optional: Add print here if needed, but usually just returns bool
    // Serial.print("KL_Dec: hasResult() -> "); Serial.println(has_valid_packet_);
    return has_valid_packet_;
}

// Enhanced internal check helper with manufacturer hint
bool KeeLoqProtocolDecoder::checkDecryptedData(uint32_t decrypted_hop, uint8_t expected_btn, uint32_t expected_serial_part, uint16_t& out_counter, const std::string& manufacturer_name_hint) {
    uint8_t decrypted_btn = static_cast<uint8_t>(decrypted_hop >> 28);
    uint16_t decrypted_discriminator = static_cast<uint16_t>((decrypted_hop >> 16) & 0xFFF); // 12 bits for discriminator
    uint16_t decrypted_cnt = static_cast<uint16_t>(decrypted_hop & 0xFFFF);

#ifdef DEBUG_KEELOQ_DECODER
    Serial.print("KL_Dec: checkDecryptedData: DecHop=0x"); Serial.print(decrypted_hop, HEX);
    Serial.print(" ExpBtn="); Serial.print(expected_btn, HEX);
    Serial.print(" ExpSn=0x"); Serial.print(expected_serial_part, HEX);
    Serial.print(" Hint='"); Serial.print(manufacturer_name_hint.c_str()); Serial.println("'");
    Serial.print("  -> DecBtn="); Serial.print(decrypted_btn, HEX);
    Serial.print(" DecDisc=0x"); Serial.print(decrypted_discriminator, HEX);
    Serial.print(" DecCnt="); Serial.println(decrypted_cnt);
#endif

    if (decrypted_btn != expected_btn) {
#ifdef DEBUG_KEELOQ_DECODER
        Serial.println("  -> Btn Mismatch!");
#endif
        return false;
    }

    // Manufacturer-specific discriminator checks
    bool check_passed = false;
    if (manufacturer_name_hint == "Centurion") {
        check_passed = (decrypted_discriminator == 0x1CE);
    } else if (manufacturer_name_hint == "Beninca") {
          check_passed = (decrypted_discriminator == 0x000);
    } else if (manufacturer_name_hint == "Monarch") {
        check_passed = (decrypted_discriminator == 0x100);
    } else {
        // Generic Check
        uint16_t serial_lsb = static_cast<uint16_t>(expected_serial_part & 0x3FF);
        uint16_t decrypted_lsb = decrypted_discriminator & 0x3FF;
        check_passed = (decrypted_lsb == serial_lsb || decrypted_discriminator == 0);
#ifdef DEBUG_KEELOQ_DECODER
        if(!check_passed) {
            Serial.print("  -> Generic Disc Check Failed (ExpLSB=0x"); Serial.print(serial_lsb, HEX);
            Serial.print(", DecLSB=0x"); Serial.print(decrypted_lsb, HEX); Serial.println(")");
        }
#endif
    }

    if(check_passed) {
        out_counter = decrypted_cnt;
#ifdef DEBUG_KEELOQ_DECODER
        Serial.println("  -> Check OK!");
#endif
        return true;
    } else {
#ifdef DEBUG_KEELOQ_DECODER
         Serial.println("  -> Check FAILED!");
#endif
        return false;
    }
}


KeeLoqStatus KeeLoqProtocolDecoder::getResult(
    KeeLoqData& result,
    uint64_t provided_mf_key,
    KeeLoqCommon::KeeLoqLearningType provided_learning_type,
    const std::string& manufacturer_name_hint /*= ""*/)
{
#ifdef DEBUG_KEELOQ_DECODER
    Serial.println("KL_Dec: getResult() called.");
#endif
    if (!has_valid_packet_) {
#ifdef DEBUG_KEELOQ_DECODER
        Serial.println("  -> No valid packet available.");
#endif
        return KeeLoqStatus::Error;
    }

    result = last_decoded_packet_; // Start with the raw decoded data
#ifdef DEBUG_KEELOQ_DECODER
    Serial.print("  Using raw packet data: 0x"); Serial.print((uint32_t)(result.data >> 32), HEX); Serial.println((uint32_t)(result.data & 0xFFFFFFFF), HEX);
    Serial.print("  Provided Key: 0x"); Serial.print((uint32_t)(provided_mf_key >> 32), HEX); Serial.println((uint32_t)(provided_mf_key & 0xFFFFFFFF), HEX);
    Serial.print("  Provided Type: "); Serial.println((int)provided_learning_type);
    Serial.print("  Provided Hint: '"); Serial.print(manufacturer_name_hint.c_str()); Serial.println("'");
#endif

    uint64_t key_to_use = 0;
    uint32_t decrypted_hop = 0;
    bool decryption_attempted = false;

    switch(provided_learning_type) {
        case KeeLoqCommon::KeeLoqLearningType::Simple:
            key_to_use = provided_mf_key;
            if(key_to_use != 0) {
#ifdef DEBUG_KEELOQ_DECODER
                 Serial.print("  Attempting Simple decrypt with key 0x"); Serial.print((uint32_t)(key_to_use >> 32), HEX); Serial.println((uint32_t)(key_to_use & 0xFFFFFFFF), HEX);
#endif
                decrypted_hop = KeeLoqCommon::decrypt(result.hop_part, key_to_use); decryption_attempted = true;
            } else {
#ifdef DEBUG_KEELOQ_DECODER
                 Serial.println("  Simple decrypt skipped (key is 0).");
#endif
            }
            break;
        case KeeLoqCommon::KeeLoqLearningType::Normal:
             if(provided_mf_key != 0) {
                 key_to_use = KeeLoqCommon::normal_learning(result.fix_part, provided_mf_key);
#ifdef DEBUG_KEELOQ_DECODER
                 Serial.print("  Derived Normal key: 0x"); Serial.print((uint32_t)(key_to_use >> 32), HEX); Serial.println((uint32_t)(key_to_use & 0xFFFFFFFF), HEX);
#endif
                 if(key_to_use != 0) { decrypted_hop = KeeLoqCommon::decrypt(result.hop_part, key_to_use); decryption_attempted = true; }
             } else {
#ifdef DEBUG_KEELOQ_DECODER
                 Serial.println("  Normal decrypt skipped (MF key is 0).");
#endif
            }
            break;
        // Add similar debug prints for other learning types if needed...
        case KeeLoqCommon::KeeLoqLearningType::Secure:
             if(provided_mf_key != 0) {
                 key_to_use = KeeLoqCommon::secure_learning(result.fix_part, result.seed, provided_mf_key);
#ifdef DEBUG_KEELOQ_DECODER
                 Serial.print("  Derived Secure key: 0x"); Serial.print((uint32_t)(key_to_use >> 32), HEX); Serial.println((uint32_t)(key_to_use & 0xFFFFFFFF), HEX);
#endif
                 if(key_to_use != 0) { decrypted_hop = KeeLoqCommon::decrypt(result.hop_part, key_to_use); decryption_attempted = true; }
             } else {
#ifdef DEBUG_KEELOQ_DECODER
                 Serial.println("  Secure decrypt skipped (MF key is 0).");
#endif
            }
             break;
        // ... other cases ...

        case KeeLoqCommon::KeeLoqLearningType::Unknown:
#ifdef DEBUG_KEELOQ_DECODER
            Serial.println("  Type is Unknown. Checking for non-encrypted formats.");
#endif
             if (manufacturer_name_hint == "AN-Motors") {
                  uint8_t btn_check = static_cast<uint8_t>((result.hop_part >> 12) & 0x0F);
                  uint16_t cnt_check = static_cast<uint16_t>((result.hop_part >> 16) & 0xFF);
                  uint8_t cnt_check_low = static_cast<uint8_t>((result.hop_part >> 24) & 0xFF);
                  uint16_t tail_check = static_cast<uint16_t>(result.hop_part & 0xFFF);
                  if(cnt_check == cnt_check_low && btn_check == result.btn && tail_check == 0x404) {
                      result.cnt = cnt_check; result.manufacturer_name = "AN-Motors"; result.learning_type = KeeLoqCommon::KeeLoqLearningType::Unknown;
#ifdef DEBUG_KEELOQ_DECODER
                      Serial.println("  -> Matched AN-Motors format.");
#endif
                      has_valid_packet_ = false; return KeeLoqStatus::Ok;
                  }
             } else if (manufacturer_name_hint == "HCS101") {
                  uint8_t btn_check = static_cast<uint8_t>((result.hop_part >> 12) & 0x0F);
                  uint16_t tail_check = static_cast<uint16_t>(result.hop_part & 0xFFF);
                   if(btn_check == result.btn && tail_check == 0x000) {
                        result.cnt = static_cast<uint16_t>(result.hop_part >> 16); result.manufacturer_name = "HCS101"; result.learning_type = KeeLoqCommon::KeeLoqLearningType::Unknown;
#ifdef DEBUG_KEELOQ_DECODER
                        Serial.println("  -> Matched HCS101 format.");
#endif
                        has_valid_packet_ = false; return KeeLoqStatus::Ok;
                   }
             }
#ifdef DEBUG_KEELOQ_DECODER
             Serial.println("  -> Unknown type, returning raw data (Cnt=0).");
#endif
             result.cnt = 0; has_valid_packet_ = false; return KeeLoqStatus::Ok;

        default:
#ifdef DEBUG_KEELOQ_DECODER
            Serial.print("  -> Unsupported learning type for getResult: "); Serial.println((int)provided_learning_type);
#endif
            return KeeLoqStatus::ErrorNotSupported;
    }

    if (decryption_attempted) {
#ifdef DEBUG_KEELOQ_DECODER
        Serial.print("  Decryption attempted. Decrypted Hop = 0x"); Serial.println(decrypted_hop, HEX);
#endif
        if (checkDecryptedData(decrypted_hop, result.btn, result.serial, result.cnt, manufacturer_name_hint)) {
            result.manufacturer_key = key_to_use;
            result.learning_type = provided_learning_type;
            if (!manufacturer_name_hint.empty()) { result.manufacturer_name = manufacturer_name_hint; }
#ifdef DEBUG_KEELOQ_DECODER
            Serial.println("  -> Decryption and check OK.");
#endif
             has_valid_packet_ = false;
            return KeeLoqStatus::Ok;
        } else {
#ifdef DEBUG_KEELOQ_DECODER
            Serial.println("  -> Decryption check FAILED.");
#endif
            return KeeLoqStatus::ErrorDecrypt;
        }
    } else {
#ifdef DEBUG_KEELOQ_DECODER
         Serial.println("  -> Decryption not attempted (e.g., key missing).");
#endif
         return KeeLoqStatus::ErrorNoKey;
    }
}


KeeLoqStatus KeeLoqProtocolDecoder::serialize(std::ostream& outputStream) const {
     if(has_valid_packet_) {
#ifdef DEBUG_KEELOQ_DECODER
         Serial.println("KL_Dec: Serializing last packet.");
#endif
        return last_decoded_packet_.serialize(outputStream);
     }
#ifdef DEBUG_KEELOQ_DECODER
         Serial.println("KL_Dec: Serialize failed - no valid packet.");
#endif
     return KeeLoqStatus::Error;
}

KeeLoqStatus KeeLoqProtocolDecoder::deserialize(std::istream& inputStream) {
      KeeLoqData loaded_data;
      KeeLoqStatus status = loaded_data.deserialize(inputStream);
#ifdef DEBUG_KEELOQ_DECODER
      Serial.print("KL_Dec: Deserialize attempt status: "); Serial.println((int)status);
#endif
      if(status == KeeLoqStatus::Ok) {
          reset();
          last_decoded_packet_ = loaded_data;
          has_valid_packet_ = true;
          return KeeLoqStatus::Ok;
      }
     reset();
     return status;
}


// --- KeeLoqProtocolEncoder Implementation ---

KeeLoqProtocolEncoder::KeeLoqProtocolEncoder() {
    stop();
}

KeeLoqProtocolEncoder::~KeeLoqProtocolEncoder() {}

KeeLoqStatus KeeLoqProtocolEncoder::loadData(const KeeLoqData& data, uint32_t repeats) {
#ifdef DEBUG_KEELOQ_ENCODER
    Serial.println("KL_Enc: loadData called.");
    Serial.print("  Input Sn=0x"); Serial.print(data.serial, HEX); Serial.print(" Btn="); Serial.print(data.btn, HEX);
    Serial.print(" Cnt="); Serial.print(data.cnt); Serial.print(" Seed=0x"); Serial.println(data.seed, HEX);
    Serial.print("  MF='"); Serial.print(data.manufacturer_name.c_str()); Serial.print("' Type="); Serial.print((int)data.learning_type);
    Serial.print(" Key=0x"); Serial.print((uint32_t)(data.manufacturer_key >> 32), HEX); Serial.println((uint32_t)(data.manufacturer_key & 0xFFFFFFFF), HEX);
    Serial.print("  Repeats="); Serial.println(repeats);
#endif
    data_to_encode_ = data;

    if (!data_to_encode_.isValid()) {
#ifdef DEBUG_KEELOQ_ENCODER
         Serial.println("  -> Error: Invalid bit count.");
#endif
        return KeeLoqStatus::ErrorFormat;
    }

    bool needs_key = (data_to_encode_.learning_type != KeeLoqCommon::KeeLoqLearningType::Unknown ||
                     (data_to_encode_.manufacturer_name != "Unknown" &&
                      data_to_encode_.manufacturer_name != "AN-Motors" &&
                      data_to_encode_.manufacturer_name != "HCS101"));

    if (needs_key && data_to_encode_.manufacturer_key == 0) {
#ifdef DEBUG_KEELOQ_ENCODER
        Serial.println("  -> Error: Key required but is zero.");
#endif
        return KeeLoqStatus::ErrorNoKey;
    }

    repeats_remaining_ = repeats;
    is_running_ = (repeats > 0);
    current_pulse_index_ = pulse_buffer_.size();
    data_to_encode_.fix_part = (static_cast<uint32_t>(data_to_encode_.btn & 0xF) << 28) | (data_to_encode_.serial & 0x0FFFFFFF);
#ifdef DEBUG_KEELOQ_ENCODER
    Serial.print("  Calculated Fix Part: 0x"); Serial.println(data_to_encode_.fix_part, HEX);
    Serial.println("  Load OK.");
#endif
    return KeeLoqStatus::Ok;
}

void KeeLoqProtocolEncoder::stop() {
#ifdef DEBUG_KEELOQ_ENCODER
    if(is_running_) Serial.println("KL_Enc: stop() called.");
#endif
    is_running_ = false;
    repeats_remaining_ = 0;
    pulse_buffer_.clear();
    current_pulse_index_ = 0;
}

Pulse KeeLoqProtocolEncoder::yield() {
    if (!is_running_) {
        return {false, 0};
    }
    if (current_pulse_index_ >= pulse_buffer_.size()) {
        if (repeats_remaining_ == 0) {
#ifdef DEBUG_KEELOQ_ENCODER
            Serial.println("KL_Enc: Yield - All repeats done.");
#endif
            is_running_ = false;
            return {false, 0};
        }
#ifdef DEBUG_KEELOQ_ENCODER
        Serial.print("KL_Enc: Yield - Generating payload for repeat "); Serial.println(repeats_remaining_);
#endif
        if (!generateNextPayload()) {
            stop(); return {false, 0};
        }
        if (!generateUpload()) {
             stop(); return {false, 0};
        }
        repeats_remaining_--;
        current_pulse_index_ = 0;
        if(pulse_buffer_.empty()) {
             stop(); return {false, 0};
        }
    }
    Pulse p = pulse_buffer_[current_pulse_index_];
#ifdef DEBUG_KEELOQ_ENCODER_VERBOSE
    Serial.print("KL_Enc: Yield pulse ["); Serial.print(current_pulse_index_); Serial.print("]: L="); Serial.print(p.level); Serial.print(" D="); Serial.println(p.duration);
#endif
    current_pulse_index_++;
    return p;
}

bool KeeLoqProtocolEncoder::generateNextPayload() {
#ifdef DEBUG_KEELOQ_ENCODER
    Serial.println("KL_Enc: generateNextPayload");
#endif
    uint16_t increment = 1;
    uint16_t old_cnt = data_to_encode_.cnt;
    if (data_to_encode_.cnt > 0xFFFF - increment) {
        data_to_encode_.cnt = 0;
    } else {
        data_to_encode_.cnt += increment;
    }
#ifdef DEBUG_KEELOQ_ENCODER
    Serial.print("  Counter incremented: "); Serial.print(old_cnt); Serial.print(" -> "); Serial.println(data_to_encode_.cnt);
#endif

    uint32_t serial_part_for_hop = 0;
    const std::string& mf_name = data_to_encode_.manufacturer_name;

    // Calculate serial_part_for_hop based on mf_name (same logic as before)
    if(mf_name == "Aprimatic") { // ... and other mf checks ...
        uint32_t apri_serial = data_to_encode_.serial & 0x0FFFFFFF; uint8_t apr1 = 0;
        for(uint16_t i = 1; i != 0x800; i <<= 1) { if(apri_serial & i) apr1++; }
        apri_serial &= 0xFFF; if(apr1 % 2 == 0) { apri_serial |= 0xC00; }
        serial_part_for_hop = apri_serial & 0xFFF;
    } else if (mf_name == "DTM_Neo" || mf_name == "FAAC_RC,XT" || mf_name == "Mutanco_Mutancode" ||
               mf_name == "Came_Space" || mf_name == "Genius_Bravo" || mf_name == "GSN") {
        serial_part_for_hop = data_to_encode_.serial & 0xFFF;
    } else if (mf_name == "NICE_Smilo" || mf_name == "NICE_MHOUSE" || mf_name == "JCM_Tech") {
        serial_part_for_hop = data_to_encode_.serial & 0xFF;
    } else if (mf_name == "Beninca") { serial_part_for_hop = 0x000;
    } else if (mf_name == "Centurion") { serial_part_for_hop = 0x1CE;
    } else if (mf_name == "Monarch") { serial_part_for_hop = 0x100;
    } else if (mf_name == "Dea_Mio") {
        uint8_t first_disc_num = (data_to_encode_.serial >> 8) & 0xF; uint8_t result_disc = (0xC + (first_disc_num % 4));
        uint32_t dea_serial_part = (data_to_encode_.serial & 0xFF) | (static_cast<uint32_t>(result_disc) << 8);
        serial_part_for_hop = dea_serial_part & 0xFFF;
    } else { serial_part_for_hop = data_to_encode_.serial & 0x3FF; }

    uint32_t plaintext = (static_cast<uint32_t>(data_to_encode_.btn & 0xF) << 28) |
                         (serial_part_for_hop << 16) | data_to_encode_.cnt;
#ifdef DEBUG_KEELOQ_ENCODER
    Serial.print("  Plaintext (BSSSCCCC): 0x"); Serial.println(plaintext, HEX);
    Serial.print("    (Using SerHop=0x"); Serial.print(serial_part_for_hop, HEX); Serial.println(")");
#endif

    uint64_t key_to_use = 0;
    bool use_encryption = true;

    if (mf_name == "Unknown") {
        data_to_encode_.updateDerivedPartsFromData(); use_encryption = false;
#ifdef DEBUG_KEELOQ_ENCODER
        Serial.println("  MF Unknown: Using raw replay data.");
#endif
    } else if (mf_name == "AN-Motors") {
         data_to_encode_.hop_part = (static_cast<uint32_t>(data_to_encode_.cnt & 0xFF) << 24) | /*...*/ 0x404; use_encryption = false;
#ifdef DEBUG_KEELOQ_ENCODER
         Serial.println("  MF AN-Motors: Using formula.");
#endif
    } else if (mf_name == "HCS101") {
         data_to_encode_.hop_part = (static_cast<uint32_t>(data_to_encode_.cnt) << 16) | /*...*/ 0x000; use_encryption = false;
#ifdef DEBUG_KEELOQ_ENCODER
         Serial.println("  MF HCS101: Using formula.");
#endif
    } else {
        uint64_t mf_or_xor_key = data_to_encode_.manufacturer_key;
        uint32_t fix_full = data_to_encode_.fix_part;

        switch(data_to_encode_.learning_type) {
             case KeeLoqCommon::KeeLoqLearningType::Simple: key_to_use = mf_or_xor_key; break;
             case KeeLoqCommon::KeeLoqLearningType::Normal: key_to_use = KeeLoqCommon::normal_learning(fix_full, mf_or_xor_key); break;
             case KeeLoqCommon::KeeLoqLearningType::Secure: key_to_use = KeeLoqCommon::secure_learning(fix_full, data_to_encode_.seed, mf_or_xor_key); break;
             // ... other cases ...
             default: return false;
        }
#ifdef DEBUG_KEELOQ_ENCODER
         Serial.print("  Using Encryption Type: "); Serial.println((int)data_to_encode_.learning_type);
         Serial.print("  Derived/Used Key: 0x"); Serial.print((uint32_t)(key_to_use >> 32), HEX); Serial.println((uint32_t)(key_to_use & 0xFFFFFFFF), HEX);
#endif
        if (key_to_use == 0) return false;
    }

    if(use_encryption) {
        data_to_encode_.hop_part = KeeLoqCommon::encrypt(plaintext, key_to_use);
#ifdef DEBUG_KEELOQ_ENCODER
        Serial.print("  Encrypted Hop Part: 0x"); Serial.println(data_to_encode_.hop_part, HEX);
#endif
    } else {
#ifdef DEBUG_KEELOQ_ENCODER
         Serial.print("  Using Unencrypted Hop Part: 0x"); Serial.println(data_to_encode_.hop_part, HEX);
#endif
    }

    uint64_t combined_normal_order = (static_cast<uint64_t>(data_to_encode_.fix_part) << 32) | data_to_encode_.hop_part;
    data_to_encode_.data = KeeLoqData::reverseBits(combined_normal_order);
#ifdef DEBUG_KEELOQ_ENCODER
    Serial.print("  Final Reversed Data for Tx: 0x"); Serial.print((uint32_t)(data_to_encode_.data >> 32), HEX); Serial.println((uint32_t)(data_to_encode_.data & 0xFFFFFFFF), HEX);
#endif
    return true;
}


bool KeeLoqProtocolEncoder::generateUpload() {
#ifdef DEBUG_KEELOQ_ENCODER
    Serial.println("KL_Enc: generateUpload");
#endif
    pulse_buffer_.clear();
    pulse_buffer_.reserve(22 + 2 + 128 + 2 + 2);

    // Header
    for (int i = 0; i < 11; ++i) { pulse_buffer_.push_back({true, TE_SHORT}); pulse_buffer_.push_back({false, TE_SHORT}); }
    pulse_buffer_.push_back({true, TE_SHORT}); pulse_buffer_.push_back({false, TE_SHORT * 10});

    // Data
    for (int i = 63; i >= 0; --i) {
        bool bit = (data_to_encode_.data >> i) & 1ULL;
        if (bit) { pulse_buffer_.push_back({true, TE_SHORT}); pulse_buffer_.push_back({false, TE_LONG}); }
        else { pulse_buffer_.push_back({true, TE_LONG}); pulse_buffer_.push_back({false, TE_SHORT}); }
    }

    // Footer trailer bit '1'
    pulse_buffer_.push_back({true, TE_SHORT}); pulse_buffer_.push_back({false, TE_LONG});
    // Final footer gap
    pulse_buffer_.push_back({true, TE_SHORT}); pulse_buffer_.push_back({false, TE_SHORT * 40});

#ifdef DEBUG_KEELOQ_ENCODER
    Serial.print("  Generated "); Serial.print(pulse_buffer_.size()); Serial.println(" pulses.");
#endif
    return !pulse_buffer_.empty();
}