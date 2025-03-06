#ifndef C1101_BRUTE
#define C1101_BRUTE

#include "protocols/CameProtocol.h"
#include "protocols/NiceFloProtocol.h"
#include "protocols/AnsonicProtocol.h"
#include "protocols/Holtek_HT12xProtocol.h"
#include "protocols/ChamberlainCodeProtocol.h"
#include "protocols/LinearProtocol.h"
#include "protocols/HormannProtocol.h"
#include "protocols/Smc5326Protocol.h"
#include "globals.h"

namespace BRUTE {

class CC1101_BRUTE {
public:
    int16_t counter;
    bool sendingFlag;
    int8_t repeat = 3;
    std::string debrujinNumber;

    std::vector<uint16_t> buffer;
    
    bool Came12BitBrute(); 
    bool Nice12BitBrute();
    bool Ansonic12BitBrute();
    bool Holtek12BitBrute();
    bool ChamberlainCodeBrute(uint8_t bitCount); 
    bool Chamberlain7BitBrute();
    bool Chamberlain8BitBrute();
    bool Chamberlain9BitBrute();
    bool debrujin();
    bool Linear10BitBrute();
    void sendBuffer(const std::vector<uint16_t>& buffer);
    void firstModulation(const std::bitset<2048>& debrujinNumber);
    void secondModulation(const std::bitset<2048>& debrujinNumber);
    void thirdModulation(const std::bitset<2048>& debrujinNumber);
    void fourthModulation(const std::bitset<2048>& debrujinNumber);

private:
    CameProtocol    cameProtocol;
    NiceFloProtocol niceFloProtocol;
    AnsonicProtocol ansonicProtocol;
    HoltekProtocol holtekProtocol;
    ChamberlainCodeProtocol chamberlainProtocol;
    LinearProtocol linearProtocol;
    HormannProtocol hormannProtocol;
    SMC5326Protocol smc5326Protocol;


};

} 

#endif