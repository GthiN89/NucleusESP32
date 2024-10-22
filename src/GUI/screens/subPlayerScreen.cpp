#include "subPlayerScreen.h"
#include "modules/RF/CC1101.h"
#include "globals.h"

SubPlayerScreen::SubPlayerScreen()
    : screenManager_(ScreenManager::getInstance()) {
}

SubPlayerScreen::~SubPlayerScreen() {
    // Cleanup pokud je potřeba
}

void SubPlayerScreen::initialize() {
    screenManager_.createReplayScreen();
}