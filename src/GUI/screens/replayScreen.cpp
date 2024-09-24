#include "replayScreen.h"

ReplayScreen::ReplayScreen()
    : screenManager_(ScreenManager::getInstance()) {
}

ReplayScreen::~ReplayScreen() {
    // Cleanup pokud je potřeba
}

void ReplayScreen::initialize() {
    screenManager_.createReplayScreen();
}
