#ifndef C1101_BRUTE
#define C1101_BRUTE

#include "protocols/CameProtocol.h"
#include "protocols/NiceFloProtocol.h"
#include "protocols/AnsonicProtocol.h"
#include "protocols/Holtek_HT12xDecoder.h"
#include "protocols/HormannProtocol.h"
#include "protocols/Smc5326Protocol.h"
#include "globals.h"

namespace BRUTE {

class CC1101_BRUTE {
public:
    int16_t counter;
    bool sendingFlag;
    bool Came12BitBrute(); 
    bool Nice12BitBrute();
    bool Ansonic12BitBrute();

private:
    CameProtocol    cameProtocol;
    NiceFloProtocol niceFloProtocol;
    AnsonicProtocol ansonicProtocol;
    HormannProtocol hormannProtocol;
    Smc5326Protocol smc5326Protocol;

};

} 

#endif