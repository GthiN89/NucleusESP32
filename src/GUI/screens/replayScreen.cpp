#include "replayScreen.h"
#include "modules/RF/CC1101.h"

ReplayScreen::ReplayScreen()
    : screenManager_(ScreenManager::getInstance()) {
}

ReplayScreen::~ReplayScreen() {
    // Cleanup pokud je potřeba
}

void ReplayScreen::initialize() {
    CC1101_CLASS CC1101;
          Serial.print("Initializing CC1101...");

  if (CC1101.init())
  {
    Serial.print("CC1101 initialized.");
  }
  else
  {
    Serial.print("CC1101 not initialized.");
  }
   // Inicializace RF Modules
    screenManager_.createReplayScreen();
}
