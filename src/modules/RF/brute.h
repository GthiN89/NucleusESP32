#ifndef C1101_BRUTE
#define C1101_BRUTE

#include "protocols/CameProtocol.h"
#include "globals.h"
//#include "CC1101.h"

namespace BRUTE {

class CC1101_BRUTE {
public:
    int16_t counter;

    bool Came12BitBrute(); 

private:
    CameProtocol cameProtocol;
};

} 

#endif