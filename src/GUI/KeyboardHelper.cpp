#include "KeyboardHelper.h"

lv_obj_t* KeyboardHelper::createKeyboard(lv_obj_t* parent, lv_keyboard_mode_t mode) {
    lv_obj_t* keyboard = lv_keyboard_create(parent);
     lv_obj_set_size(keyboard,  LV_HOR_RES, LV_VER_RES / 2);
     lv_keyboard_set_mode(keyboard, mode);
     lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);  // Hide the keyboard initially  
    return keyboard;
}
