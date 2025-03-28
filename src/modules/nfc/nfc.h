#pragma once
#include <Adafruit_PN532.h>

namespace NFC {

class NFC_CLASS {
public:
    static NFC_CLASS& getInstance() {
        static NFC_CLASS instance;
        return instance;
    }

    bool init();
    void deinit();
    bool isInitialized() const { return _initialized; }
    void NFCloop();
    void mifaredump();
    void read443AUUID();

private:
    NFC_CLASS();  
    ~NFC_CLASS(); 

    Adafruit_PN532* _nfc = nullptr;
    bool _initialized = false;

};

} // namespace NFC