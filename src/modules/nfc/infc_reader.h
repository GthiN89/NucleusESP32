#ifndef NFC_READER_H
#define NFC_READER_H

#if __has_include(<Arduino.h>)
  #include <Arduino.h>
#else
  // Minimal definitions to satisfy clang-tidy when Arduino.h is not available
  typedef unsigned char byte;
  class String {
  public:
    String() {}
    String(const char*) {}
    String(const String&) {}
    String& operator=(const String&) { return *this; }
    void reserve(size_t) {}
    String& operator+=(char) { return *this; }
    String& operator+=(const String&) { return *this; }
    String toUpperCase() { return *this; }
  };
#endif

#include <array>

struct UID {
    std::array<byte, 10> data{};
    byte size = 0;
};

class INFCReader {
public:
    virtual ~INFCReader() = default;

    INFCReader(const INFCReader&) = delete;
    INFCReader& operator=(const INFCReader&) = delete;
    INFCReader(INFCReader&&) = delete;
    INFCReader& operator=(INFCReader&&) = delete;

    virtual void initialize() = 0;
    virtual bool isNewCardPresent() const = 0;
    virtual bool readCardSerial() = 0;
    [[nodiscard]] virtual UID getUID() const = 0;
    virtual void halt() = 0;
    virtual void powerDown() = 0;

protected:
    INFCReader() = default;
};

#endif // NFC_READER_H 