#include "brute.h"
#include "modules/RF/CC1101.h"

#include "esp_task_wdt.h"
#include "driver/gpio.h"
#include "esp_timer.h"


namespace BRUTE {
CC1101_CLASS cc1101;
int16_t counter = 0;
bool sendingFlag = false;


#define WDT_TIMEOUT 60  // Disable watchdog timeout on Core 1

TaskHandle_t bruteTaskHandle = NULL;
volatile bool BitSend = false;

void bitSendTask(void *param) {
    esp_task_wdt_delete(NULL); // Disable watchdog on this core
    int i = 0;
    while (i < 4097) {
        int64_t start_time = esp_timer_get_time();

        // Generate waveform
        for (size_t j = 0; j < samplesToSend.size(); j += 2) {
            gpio_set_level(CC1101_CCGDO0A, LOW);
            int64_t target_time = start_time + samplesToSend[j];
            while (esp_timer_get_time() < target_time);  // Busy-wait for precise timing

            gpio_set_level(CC1101_CCGDO0A, HIGH);
            target_time = start_time + samplesToSend[j] + samplesToSend[j+1];
            while (esp_timer_get_time() < target_time);  // Busy-wait for precise timing
        }

        // gpio_set_level(CC1101_CCGDO0A, LOW);
        // int64_t end_time = start_time + 10;  // Additional delay
        // while (esp_timer_get_time() < end_time);  // Busy-wait for precise timing
        
        i++;
    }

    BitSend = true;
    vTaskDelete(NULL);
}





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

    while(true ) {
    i++;
   // i=5;
 //   Serial.println(i);
    encoderState = EncoderStepStart;
    while(encoderState != EncoderStepReady){
    cameProtocol.yield(i);
    }
hw_timer_t *timer = NULL;
timer = timerBegin(0, 80, true); // Timer 0, prescaler 80 → 1 tick = 1 µs

    for (size_t j = 0; j < samplesToSend.size(); j += 2) {
        timerRestart(timer);
        int32_t start_time = esp_timer_get_time();
        // Compute the target times in microseconds.
        int32_t sample1 = start_time + samplesToSend[j];    // Add 5 µs offset if needed
        int32_t sample2 = sample1 + samplesToSend[j + 1];
    
        gpio_set_level(CC1101_CCGDO0A, LOW);
        while (esp_timer_get_time() < sample1);  // Wait until sample1 time (in µs)
    
        gpio_set_level(CC1101_CCGDO0A, HIGH);
        while (esp_timer_get_time() < sample2);  // Wait until sample2 time (in µs)
    }

            gpio_set_level(CC1101_CCGDO0A, LOW);
   // delayMicroseconds(1000);   
    counter++;
    }
    void timerEnd(hw_timer_t * timer);
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

    int i = 0;
        counter = 0;

    while(i < 4097 ) {
    i++;
 //   Serial.println(i);
    encoderState = EncoderStepStart;
    while(encoderState != EncoderStepReady){
    niceFloProtocol.yield(i);
    }

    for (size_t j = 0; j < samplesToSend.size(); j=j+2) {
                
                gpio_set_level(CC1101_CCGDO0A, LOW);
                delayMicroseconds(samplesToSend[j]); 
                gpio_set_level(CC1101_CCGDO0A, HIGH);
                delayMicroseconds(samplesToSend[j+1]);                 
        }
            gpio_set_level(CC1101_CCGDO0A, LOW);
    delayMicroseconds(10);   
    counter++;
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
                gpio_set_level(CC1101_CCGDO0A, LOW);
                delayMicroseconds(samplesToSend[j]); 
                gpio_set_level(CC1101_CCGDO0A, HIGH);
                delayMicroseconds(samplesToSend[j+1]);              
        }
    gpio_set_level(CC1101_CCGDO0A, LOW);
    delayMicroseconds(10);    
    counter++;
    }

   return true;
}




}