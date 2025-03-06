#include "brute.h"
#include "modules/RF/CC1101.h"
#define DEBUG_ENABLED 1



namespace BRUTE {
CC1101_CLASS cc1101;
int16_t counter = 0;
bool sendingFlag = false;

void CC1101_BRUTE::sendBuffer(const std::vector<uint16_t>& buffer) {
    bool levelFlag = false;
    for (size_t j = 0; j < buffer.size(); ++j) {
        gpio_set_level(CC1101_CCGDO0A, levelFlag);
        levelFlag = !levelFlag;
        delayMicroseconds(buffer[j]);
    }
}
void CC1101_BRUTE::firstModulation(const std::bitset<2048>& debrujinNumber) {
    buffer.clear();
    buffer.reserve(2048 * 2);
    
    for (size_t i = 0; i < debrujinNumber.size(); ++i) {
        if (i % 12 == 0) {
            // Insert header for first modulation
            buffer.push_back(555);
            buffer.push_back(555);
            buffer.push_back(1111);
            buffer.push_back(1111);
            buffer.push_back(19425);
            buffer.push_back(555);
        }
        // Process bit modulation
        if (debrujinNumber[i]) {
            buffer.push_back(1111);
            buffer.push_back(555);
        } else {
            buffer.push_back(555);
            buffer.push_back(1111);
        }
        // Optionally, send and clear if the buffer grows too large.
        if (buffer.size() > 256) {
            sendBuffer(buffer);
            buffer.clear();
        }
    }
    if (!buffer.empty()) {
        sendBuffer(buffer);
        buffer.clear();
    }
    Serial.println(F("First modulation complete"));
}

// Second modulation: header = {320,320,640,640,11520,320}
// Bit mapping: 1 -> {640,320}, 0 -> {320,640}
void CC1101_BRUTE::secondModulation(const std::bitset<2048>& debrujinNumber) {
    buffer.clear();
    buffer.reserve(2048 * 2);
    
    for (size_t i = 0; i < debrujinNumber.size(); ++i) {
        if (i % 12 == 0) {
            // Insert header for second modulation
            buffer.push_back(320);
            buffer.push_back(320);
            buffer.push_back(640);
            buffer.push_back(640);
            buffer.push_back(11520);
            buffer.push_back(320);
        }
        if (debrujinNumber[i]) {
            buffer.push_back(640);
            buffer.push_back(320);
        } else {
            buffer.push_back(320);
            buffer.push_back(640);
        }
        if (buffer.size() > 256) {
            sendBuffer(buffer);
            buffer.clear();
        }
    }
    if (!buffer.empty()) {
        sendBuffer(buffer);
        buffer.clear();
    }
    Serial.println(F("Second modulation complete"));
}

// Third modulation: header = {400,400,800,800,14400,400}
// Bit mapping: 1 -> {800,400}, 0 -> {400,800}
void CC1101_BRUTE::thirdModulation(const std::bitset<2048>& debrujinNumber) {
    buffer.clear();
    buffer.reserve(2048 * 2);
    
    for (size_t i = 0; i < debrujinNumber.size(); ++i) {
        if (i % 12 == 0) {
            // Insert header for third modulation
            buffer.push_back(400);
            buffer.push_back(400);
            buffer.push_back(800);
            buffer.push_back(800);
            buffer.push_back(14400);
            buffer.push_back(400);
        }
        if (debrujinNumber[i]) {
            buffer.push_back(800);
            buffer.push_back(400);
        } else {
            buffer.push_back(400);
            buffer.push_back(800);
        }
        if (buffer.size() > 256) {
            sendBuffer(buffer);
            buffer.clear();
        }
    }
    if (!buffer.empty()) {
        sendBuffer(buffer);
        buffer.clear();
    }
    Serial.println(F("Third modulation complete"));
}

// Fourth modulation: header = {700,700,1400,1400,25200,700}
// Bit mapping: 1 -> {1400,700}, 0 -> {700,1400}
void CC1101_BRUTE::fourthModulation(const std::bitset<2048>& debrujinNumber) {
    buffer.clear();
    buffer.reserve(2048 * 2);
    
    for (size_t i = 0; i < debrujinNumber.size(); ++i) {
        if (i % 12 == 0) {
            // Insert header for fourth modulation
            buffer.push_back(700);
            buffer.push_back(700);
            buffer.push_back(1400);
            buffer.push_back(1400);
            buffer.push_back(25200);
            buffer.push_back(700);
        }
        if (debrujinNumber[i]) {
            buffer.push_back(1400);
            buffer.push_back(700);
        } else {
            buffer.push_back(700);
            buffer.push_back(1400);
        }
        if (buffer.size() > 256) {
            sendBuffer(buffer);
            buffer.clear();
        }
    }
    if (!buffer.empty()) {
        sendBuffer(buffer);
        buffer.clear();
    }
    Serial.println(F("Fourth modulation complete"));
}

// Now the main debrujin() function calls each modulation method in turn:
bool CC1101_BRUTE::debrujin()
{
    Serial.println("debrujin() start");
    sendingFlag = true;
    pinMode(CC1101_CCGDO0A, OUTPUT);
    digitalWrite(CC1101_CCGDO0A, LOW);
    cc1101.setFrequency(433.92);
    cc1101.setCC1101Preset(AM650);
    cc1101.loadPreset();
    cc1101.initRaw();

    // Create the de Bruijn bitset
    std::string binaryString = 
        "0000000000001000000000011000000000101000000000111000000001001000000001011000000001101000000001111000000010001000000010011000000010101000000010111000000011001000000011011000000011101000000011111000000100001000000100011000000100101000000100111000000101001000000101011000000101101000000101111000000110001000000110011000000110101000000110111000000111001000000111011000000111101000000111111000001000001000011000001000101000001000111000001001001000001001011000001001101000001001111000001010001000001010011000001010101000001010111000001011001000001011011000001011101000001011111000001100001000001100011000001100101000001100111000001101001000001101011000001101101000001101111000001110001000001110011000001110101000001110111000001111001000001111011000001111101000001111111000010000101000010000111000010001001000010001011000010001101000010001111000010010001000010010011000010010101000010010111000010011001000010011011000010011101000010011111000010100011000010100101000010100111000010101001000010101011000010101101000010101111000010110001000010110011000010110101000010110111000010111001000010111011000010111101000010111111000011000011000101000011000111000011001001000011001011000011001101000011001111000011010001000011010011000011010101000011010111000011011001000011011011000011011101000011011111000011100011000011100101000011100111000011101001000011101011000011101101000011101111000011110001000011110011000011110101000011110111000011111001000011111011000011111101000011111111000100010001001100010001010100010001011100010001100100010001101100010001110100010001111100010010001100010010010100010010011100010010100100010010101100010010110100010010111100010011001100010011010100010011011100010011100100010011101100010011110100010011111100010100010100011100010100100100010100101100010100110100010100111100010101001100010101010100010101011100010101100100010101101100010101110100010101111100010110001100010110010100010110011100010110100100010110101100010110110100010110111100010111001100010111010100010111011100010111100100010111101100010111110100010111111100011000110100011000111100011001001100011001010100011001011100011001100100011001101100011001110100011001111100011010010100011010011100011010100100011010101100011010110100011010111100011011001100011011010100011011011100011011100100011011101100011011110100011011111100011100011100100100011100101100011100110100011100111100011101001100011101010100011101011100011101100100011101101100011101110100011101111100011110010100011110011100011110100100011110101100011110110100011110111100011111001100011111010100011111011100011111100100011111101100011111110100011111111100100100100101100100100110100100100111100100101001100100101010100100101011100100101101100100101110100100101111100100110010100100110011100100110101100100110110100100110111100100111001100100111010100100111011100100111101100100111110100100111111100101001010100101001011100101001101100101001110100101001111100101010011100101010101100101010110100101010111100101011001100101011010100101011011100101011101100101011110100101011111100101100101100110100101100111100101101001100101101010100101101011100101101101100101101110100101101111100101110011100101110101100101110110100101110111100101111001100101111010100101111011100101111101100101111110100101111111100110011001101010011001101110011001110110011001111010011001111110011010011010011110011010101010011010101110011010110110011010111010011010111110011011001110011011010110011011011010011011011110011011101010011011101110011011110110011011111010011011111110011100111010011100111110011101010110011101011010011101011110011101101010011101101110011101110110011101111010011101111110011110011110101010011110101110011110110110011110111010011110111110011111010110011111011010011111011110011111101010011111101110011111110110011111111010011111111110101010101011101010101101101010101111101010110101101010110111101010111011101010111101101010111111101011010111101011011011101011011101101011011111101011101011101101101011101111101011110111101011111011101011111101101011111111101101101101111101101110111101101111011101101111111101110111011111110111101111110111110111111111111";
    
    std::bitset<2048> debrujinNumber(binaryString);

    // Call separate modulation methods
    firstModulation(debrujinNumber);
    secondModulation(debrujinNumber);
    thirdModulation(debrujinNumber);
    fourthModulation(debrujinNumber);

    counter++;
    sendingFlag = false;
    Serial.println("debrujin() complete");
    buffer.clear();
    return true;
}

bool CC1101_BRUTE::Came12BitBrute()
{
    sendingFlag = true;
    pinMode(CC1101_CCGDO0A, OUTPUT);
    digitalWrite(CC1101_CCGDO0A, LOW);
    cc1101.setFrequency(433.92);
    cc1101.setCC1101Preset(AM650);
    cc1101.loadPreset();
    cc1101.initRaw();
    int i = 0;
        counter = 0;

    while(i < 4095 ) {
    i++;


    cameProtocol.yield(i);


    for(int k = 0; k < repeats; k++) {
        bool levelFlag = false;
        for (size_t j = 0; j < samplesToSend.size(); j++) {
            gpio_set_level(CC1101_CCGDO0A, levelFlag);
            levelFlag = !levelFlag; 
            delayMicroseconds(samplesToSend[j]);                
    }
    gpio_set_level(CC1101_CCGDO0A, LOW);
    delayMicroseconds(11520);
    } 
    counter++;
    }
    sendingFlag = false;
   return true;
}

bool CC1101_BRUTE::Nice12BitBrute()
{
    sendingFlag = true;
    pinMode(CC1101_CCGDO0A, OUTPUT);
    digitalWrite(CC1101_CCGDO0A, LOW);
    cc1101.setFrequency(433.92);
    cc1101.setCC1101Preset(AM650);
    cc1101.loadPreset();
    cc1101.initRaw();
    int i = 0;
        counter = 0;

    while(i < 4095 ) {
    i++;

    niceFloProtocol.yield(i);


    for(int k = 0; k < repeats; k++) {
        bool levelFlag = false;
        for (size_t j = 0; j < samplesToSend.size(); j++) {
            gpio_set_level(CC1101_CCGDO0A, levelFlag);
            levelFlag = !levelFlag; 
            delayMicroseconds(samplesToSend[j]);                
    }
    gpio_set_level(CC1101_CCGDO0A, LOW);
    delayMicroseconds(25200);
    }

    counter++;
}
    sendingFlag = false;
   return true;
}

bool CC1101_BRUTE::Ansonic12BitBrute()
{
    sendingFlag = true;
    pinMode(CC1101_CCGDO0A, OUTPUT);
    digitalWrite(CC1101_CCGDO0A, LOW);
    cc1101.setFrequency(433.92);
    cc1101.setCC1101Preset(AM650);
    cc1101.loadPreset();
    cc1101.initRaw();
    encoderState = EncoderStepStart;    
    int i = 0;
    counter = 0;

    while(i < 4095) {
        i++;

            ansonicProtocol.yield(i);

        
            for(int k = 0; k < repeats; k++) {
                bool levelFlag = false;
                for (size_t j = 0; j < samplesToSend.size(); j++) {
                    gpio_set_level(CC1101_CCGDO0A, levelFlag);
                    levelFlag = !levelFlag; 
                    delayMicroseconds(samplesToSend[j]);                
            }
            gpio_set_level(CC1101_CCGDO0A, LOW);
            delayMicroseconds(19425);
            }
        
        counter++;
    }
    sendingFlag = false;
    return true;
}





bool CC1101_BRUTE::Holtek12BitBrute() {
    sendingFlag = true;
    pinMode(CC1101_CCGDO0A, OUTPUT);
    digitalWrite(CC1101_CCGDO0A, LOW);
    cc1101.setFrequency(433.92);
    cc1101.setCC1101Preset(AM650);
    cc1101.loadPreset();
    cc1101.initRaw();
    int i = 0;
    counter = 0;
    // Brute-force all 12-bit keys (0 .. 4095)
    while(i < 4095) {
        i++;

            holtekProtocol.yield(i);

            for(int k = 0; k < repeats; k++) {
                bool levelFlag = false;
                for (size_t j = 0; j < samplesToSend.size(); j++) {
                    gpio_set_level(CC1101_CCGDO0A, levelFlag);
                    levelFlag = !levelFlag; 
                    delayMicroseconds(samplesToSend[j]);                
            }
            gpio_set_level(CC1101_CCGDO0A, LOW);
            delayMicroseconds(14400);    
            }
        
        counter++;
    }
    sendingFlag = false;
    return true;
}

bool CC1101_BRUTE::ChamberlainCodeBrute(uint8_t bitCount) {
    sendingFlag = true;
    pinMode(CC1101_CCGDO0A, OUTPUT);
    // Ensure transmission starts HIGH (like in Came)
    digitalWrite(CC1101_CCGDO0A, HIGH);
    // For Chamberlain codes, use an appropriate frequency (e.g. 315 MHz)
    cc1101.setFrequency(315.0);
    cc1101.setCC1101Preset(AM650);
    cc1101.loadPreset();
    cc1101.initRaw();

    // Maximum code depends on bitCount: (2^bitCount)
    uint32_t maxCode = 1UL << bitCount;
    for (uint32_t code = 0; code < maxCode; code++) {
        encoderState = EncoderStepStart;
        // Repeatedly call yield() until the protocol is ready.
        while (encoderState != EncoderStepReady) {
            chamberlainProtocol.yield(code, bitCount);
        }
        // Now the samples are ready in samplesToSend.
        Serial.print("Sending Chamberlain ");
        Serial.print(bitCount);
        Serial.print("-bit code: 0x");
        Serial.print(code, HEX);
        Serial.print(" Samples: ");
        for (size_t j = 0; j < samplesToSend.size(); j++) {
            Serial.print(samplesToSend[j]);
            Serial.print(",");
        }
        Serial.println();

        // Transmit pulses: each pair of durations represents a HIGH then LOW pulse.
        for (size_t j = 0; j < samplesToSend.size(); j += 2) {
            gpio_set_level(CC1101_CCGDO0A, HIGH);
            delayMicroseconds(samplesToSend[j]);
            gpio_set_level(CC1101_CCGDO0A, LOW);
            delayMicroseconds(samplesToSend[j + 1]);
        }
        gpio_set_level(CC1101_CCGDO0A, LOW);
        delayMicroseconds(10);
    }
    sendingFlag = false;
    return true;
}

    
    bool CC1101_BRUTE::Chamberlain7BitBrute() {
    #if DEBUG_ENABLED
        Serial.println("Chamberlain7BitBrute: Invoking ChamberlainCodeBrute with 7 bits");
    #endif
        return ChamberlainCodeBrute(7);
    }
    
    bool CC1101_BRUTE::Chamberlain8BitBrute() {
    #if DEBUG_ENABLED
        Serial.println("Chamberlain8BitBrute: Invoking ChamberlainCodeBrute with 8 bits");
    #endif
        return ChamberlainCodeBrute(8);
    }
    
    bool CC1101_BRUTE::Chamberlain9BitBrute() {
    #if DEBUG_ENABLED
        Serial.println("Chamberlain9BitBrute: Invoking ChamberlainCodeBrute with 9 bits");
    #endif
        return ChamberlainCodeBrute(9);
    }
    
    bool CC1101_BRUTE::Linear10BitBrute() {
    #if DEBUG_ENABLED
        Serial.println("Linear10BitBrute: Starting brute force for 10-bit Linear protocol");
    #endif
        sendingFlag = true;
        pinMode(CC1101_CCGDO0A, OUTPUT);
        digitalWrite(CC1101_CCGDO0A, LOW);
    #if DEBUG_ENABLED
        Serial.println("Linear10BitBrute: Pin configured and set LOW");
    #endif
        cc1101.setFrequency(433.92);
        cc1101.setCC1101Preset(AM650);
        cc1101.loadPreset();
        cc1101.initRaw();
    #if DEBUG_ENABLED
        Serial.println("Linear10BitBrute: CC1101 configuration complete");
    #endif
    
        // For 10-bit linear protocol, brute-force codes from 0 to 1023.
        uint32_t maxCode = 1U << 10;
        for (uint32_t code = 0; code < maxCode; code++) {
    #if DEBUG_ENABLED
            Serial.print("Linear10BitBrute: Sending code ");
            Serial.println(code, DEC);
    #endif
            linearProtocol.startEncoding(code, 10);
    #if DEBUG_ENABLED
            Serial.print("Linear10BitBrute: Encoded sample count: ");
            Serial.println(linearProtocol.getEncodedSamples().size(), DEC);
    #endif
            const std::vector<long long int>& samples = linearProtocol.getEncodedSamples();
            for (size_t j = 0; j < samples.size(); j += 2) {
    #if DEBUG_ENABLED
                Serial.print("Linear10BitBrute: Pulse pair [");
                Serial.print(j);
                Serial.print("]: high=");
                Serial.print(samples[j]);
                Serial.print(", low=");
                Serial.println(samples[j + 1]);
    #endif
                gpio_set_level(CC1101_CCGDO0A, HIGH);
                delayMicroseconds(samples[j]);
                gpio_set_level(CC1101_CCGDO0A, LOW);
                delayMicroseconds(samples[j + 1]);
            }
            gpio_set_level(CC1101_CCGDO0A, LOW);
            delayMicroseconds(1000);
    #if DEBUG_ENABLED
            Serial.println("Linear10BitBrute: Code sent");
    #endif
            counter++;
        }
        sendingFlag = false;
    #if DEBUG_ENABLED
        Serial.print("Linear10BitBrute: Completed brute force, total codes sent: ");
        Serial.println(counter);
    #endif
        return true;
    }
}
    