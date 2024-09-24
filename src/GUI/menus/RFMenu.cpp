#include "MainMenuScreen.h"
#include "../events.h"
#include "../../globals.h"
//#include "modules/RF/subGhz.h"
#include "GUI/screens/replay.h"
#include "GUI/screens/playRecorded.h"

#include "../screens/c1101Scan.h"


#include "RFmenu.h"

RFMenuScreen::RFMenuScreen()
    : screenManager_(ScreenManager::getInstance()) {
}

RFMenuScreen::~RFMenuScreen() {
    // Cleanup pokud je potřeba
}

void RFMenuScreen::initialize() {
    screenManager_.createRFMenu();
}



