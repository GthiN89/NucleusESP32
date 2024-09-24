// File: src/GUI/menus/RFMenu.h

#ifndef RF_MENU_H
#define RF_MENU_H

#include <lvgl.h>
#include "../ScreenManager.h"

class RFMenuScreen {
public:
    RFMenuScreen();
    ~RFMenuScreen();

    void initialize();

private:
    ScreenManager& screenManager_;
};

#endif 