#ifndef SPINBOX_H
#define SPINBOX_H

#include <lvgl.h>


    class SpinBox {
public:
    static lv_obj_t* createSpinbox(lv_obj_t* parent, int16_t valueStore, int16_t min, int16_t max, int16_t step, lv_event_cb_t event_cb, const void* user_data);

private:
    static void spinbox_increment_event_cb(lv_event_t* e);
    static void spinbox_decrement_event_cb(lv_event_t* e);

    static lv_obj_t* spinbox_obj; 
};



#endif 