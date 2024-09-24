// File: MainMenuScreen.cpp

#include "MainMenuScreen.h"

// Constructor
MainMenuScreen::MainMenuScreen()
    : screenManager_(ScreenManager::getInstance()) {
}

// Destructor
MainMenuScreen::~MainMenuScreen() {
    // Cleanup if necessary
}

void MainMenuScreen::initialize() {
    screenManager_.createmainMenu();
}
