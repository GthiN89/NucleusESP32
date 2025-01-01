#ifndef BUTTON_HELPER_H
#define BUTTON_HELPER_H

#include <lvgl.h>


class ButtonHelper {
public:
    static lv_obj_t* createButton(lv_obj_t* parent, const char* text);
};

#endif 
