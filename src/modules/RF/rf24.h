#ifndef NRF24_H
#define NRF24_H

#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


enum RF24State {
    RF24_STATE_IDLE,
    RF24_STATE_TEST
};
extern RF24State RF24CurrentState;

extern RF24 radio;

void enableRF24();          
void testRF24();     

#endif 
