#ifndef KEYBOARD_HELPER_H
#define KEYBOARD_HELPER_H

#include <lvgl.h>


class KeyboardHelper {
public:
    static lv_obj_t* createKeyboard(lv_obj_t* parent, lv_keyboard_mode_t mode);
};
#endif 
