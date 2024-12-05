#ifndef IR_H
#define IR_H

#include <stdint.h>
#include <IRrecv.h>
#include <IRutils.h>

// IR TX and RX Pins
#define IR_TX 26        
#define IR_RX 34       

// EU/NA region settings
#define EU 1  
#define NA 0  

// Macro to calculate the size of the NA/EU databases
#define NUM_ELEM(x) (sizeof(x) / sizeof(*(x)))

// Debugging
#define DEBUG 0
#define DEBUGP(x) if (DEBUG == 1) { x ; }

// Frequency to timer value conversion (in milliseconds)
#define freq_to_timerval(x) (x / 1000)

extern IRrecv irrecv;
extern decode_results results;  // Somewhere to store the results

// IR code structure definition
struct IrCode {
  uint8_t timer_val;
  uint8_t numpairs;
  uint8_t bitcompression;
  uint16_t const *times;
  uint8_t const *codes;
};

enum IRState { STATE_IDDLE, STATE_READ, STATE_TV_B_GONE };
extern IRState IRCurrentState;

extern decode_results results;   
extern IRrecv irrecv;      
class ir
{
private:
  /* data */
public:
  void sendReceived();
};




#endif // IR_H
