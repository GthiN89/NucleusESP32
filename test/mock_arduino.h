#ifndef MOCK_ARDUINO_H
#define MOCK_ARDUINO_H

#include <cstdint>
#include <string>

// Arduino types
using byte = uint8_t;
using String = std::string;

// Mock SPI class
class SPIClass {
public:
    static void begin() {}
};

extern SPIClass SPI;

// Mock Serial class
class SerialClass {
public:
    void begin(unsigned long) {}
    void print(const char*) {}
    void print(const String&) {}
    void println(const char*) {}
    void println(const String&) {}
    void println() {}
};

extern SerialClass Serial;

#endif // MOCK_ARDUINO_H 