#ifndef LCD_PWM_H
#define LCD_PWM_H

#include <stdint.h>
#include "driver/ledc.h"
#include "esp_err.h"



// LEDC configuration macros
#define LEDC_TIMER      LEDC_TIMER_0
#define LEDC_MODE       LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL    LEDC_CHANNEL_0
#define LEDC_OUTPUT_IO  21           // LCD backlight GPIO pin
#define LEDC_DUTY_RES   LEDC_TIMER_13_BIT
#define LEDC_FREQUENCY  5000         // 5 kHz PWM frequency

class backLight_ {
    public:
    void init_pwm(void);
    void set_backlight(uint32_t duty);
};




#endif // LCD_PWM_H
