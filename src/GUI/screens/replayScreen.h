#ifndef C1101_SCREEN_H
#define C1101_SCREEN_H

#include <lvgl.h>
#include "../ScreenManager.h"

class ReplayScreen {
public:
    ReplayScreen();
    ~ReplayScreen();

    void initialize();

private:
    ScreenManager& screenManager_;
};

#endif // C1101_SCREEN_H
