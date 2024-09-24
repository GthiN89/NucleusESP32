#ifndef MENU_H
#define MENU_H

#include <lvgl.h>
#include "../ScreenManager.h"

class MainMenuScreen {
public:
    MainMenuScreen();
    ~MainMenuScreen();

    void initialize();

private:
    ScreenManager& screenManager_;
};

#endif // MENU_H