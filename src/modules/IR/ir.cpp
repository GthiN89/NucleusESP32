#include <Arduino.h>

/*
 * Set library modifiers first to set output pin etc.
 */
#include "PinDefinitionsAndMore.h"
#define IRSND_IR_FREQUENCY          38000

#if ! (defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__))
#define IRSND_PROTOCOL_NAMES        1
#endif
//#define IRSND_GENERATE_NO_SEND_RF // for back to back tests

#include <irsndSelectAllProtocols.h>
/*
 * After setting the definitions we can include the code and compile it.
 */
#include <irsnd.hpp>

IRMP_DATA irsnd_data;

void setupIR()
{
    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) /*stm32duino*/|| defined(USBCON) /*STM32_stm32*/ \
    || defined(SERIALUSB_PID)  || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif
#if defined(ESP8266)
    Serial.println(); // to separate it from the internal boot output
#endif

    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRMP));

    Serial.print(F("Send sample frequency="));
    Serial.print(F_INTERRUPTS);
    Serial.println(F(" Hz"));

    irsnd_init();
    irmp_irsnd_LEDFeedback(true); // Enable send signal feedback at LED_BUILTIN

    Serial.println(F("Send IR signals at pin " STR(IRSND_OUTPUT_PIN)));
    delay(1000);
}

void SendAll()
{
//     static uint8_t sAddress = 1;
//     static uint8_t sCommand = 1;
//     static uint8_t sRepeats = 0;

    irsnd_data.protocol = IRMP_RC6_PROTOCOL;
        irsnd_data.address = 0x0;
        irsnd_data.command = 0x1;
        irsnd_data.flags = 5;

irsnd_send_data(&irsnd_data, true);


    irsnd_data.protocol = IRMP_RC6_PROTOCOL;
        irsnd_data.address = 0x0;
        irsnd_data.command = 0x2;
        irsnd_data.flags = 5;

irsnd_send_data(&irsnd_data, true);


    irsnd_data.protocol = IRMP_RC6_PROTOCOL;
        irsnd_data.address = 0x0;
        irsnd_data.command = 0x3;
        irsnd_data.flags = 5;

irsnd_send_data(&irsnd_data, true);

//     for (uint_fast8_t i = 0; i < sizeof(irsnd_used_protocol_index); ++i)
//     {
//         irsnd_data.protocol = pgm_read_byte(&irsnd_used_protocol_index[i]);
//         irsnd_data.address = sAddress;
//         irsnd_data.command = sCommand;
//         irsnd_data.flags = sRepeats;

//         // true = wait for frame and trailing space/gap to end. This stores timer state and restores it after sending.
//         if (!irsnd_send_data(&irsnd_data, true))
//         {
//             Serial.println(F("Protocol not found")); // name of protocol is printed by irsnd_data_print()
//         }

//         irsnd_data_print(&Serial, &irsnd_data);

//         sAddress++;
//         sCommand++;
//         delay(2000);
//     }
//     Serial.println();
//     Serial.println();

//     sRepeats++;

//     // we have 0x27 protocols now start with next number range
//     sAddress = (sAddress & 0xC0) + 0x40;
//     sCommand = sAddress;
//     Serial.print(F("Now sending all with number of repeats="));
//     Serial.println(sRepeats);
}
