#ifndef BUTTON_HELPER_H
#define BUTTON_HELPER_H

#include <lvgl.h>


class ButtonHelper {
public:
    // Vytvoří tlačítko s nastavitelným textem a callback funkcí
    static lv_obj_t* createButton(lv_obj_t* parent, const char* text);
};

#endif // BUTTON_HELPER_H
