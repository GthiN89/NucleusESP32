#include "replayScreen.h"
#include "modules/RF/CC1101.h"
#include "globals.h"

ReplayScreen::ReplayScreen()
    : screenManager_(ScreenManager::getInstance()) {
}

ReplayScreen::~ReplayScreen() {
    // Cleanup pokud je potřeba
}

void ReplayScreen::initialize() {
//   if(!CC1101_init) {
//   CC1101_CLASS CC1101;
//   Serial.print("Initializing CC1101...");
//   if (CC1101.init())
//   {
//     Serial.print("CC1101 initialized.");
//     CC1101_init = true;
//     screenManager_.createReplayScreen();
//   }
//   else
//   {
//     Serial.print("CC1101 not initialized.");
//   } 
// } else {
//   screenManager_.createReplayScreen();
// }

    screenManager_.createReplayScreen();
}
