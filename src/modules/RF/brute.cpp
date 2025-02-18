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
    cc1101.setCC1101Preset(C1101preset);
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
    cc1101.setCC1101Preset(C1101preset);
    cc1101.loadPreset();
    cc1101.initRaw();
    encoderState = EncoderStepStart;
    
 //   Serial.println(F("Nice12BitBrute"));
    int i = 0;
    counter = 0;
    while(i < 4097 ) {
    i++;
    counter++;

 //   Serial.println(i);
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
  

    }
    sendingFlag = false;
   return true;
}

bool CC1101_BRUTE::Ansonic12BitBrute()
{
    pinMode(CC1101_CCGDO0A, OUTPUT);
    digitalWrite(CC1101_CCGDO0A, LOW);
    cc1101.setFrequency(433.92);
    cc1101.setCC1101Preset(C1101preset);
    cc1101.loadPreset();
    cc1101.initRaw();
    encoderState = EncoderStepStart;
    delay(100);
    
//    Serial.println(F("Ansonic12BitBrute"));
    int i = 0;
    while(i < 4097 ) {
    i++;
    Serial.println(i);
    encoderState = EncoderStepStart;
    while(encoderState != EncoderStepReady){
            ansonicProtocol.yield(i);
            if(encoderState == EncoderStepReady){
                break;
            }
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

   return true;
}


}