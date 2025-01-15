#include "spinbox.h"

lv_obj_t* SpinBox::spinbox_obj = nullptr;

void SpinBox::spinbox_increment_event_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t* spinbox = static_cast<lv_obj_t*>(lv_event_get_user_data(e));
        lv_spinbox_increment(spinbox);
    }
}

void SpinBox::spinbox_decrement_event_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t* spinbox = static_cast<lv_obj_t*>(lv_event_get_user_data(e));
        lv_spinbox_decrement(spinbox);
    }
}

lv_obj_t* SpinBox::createSpinbox(lv_obj_t* parent, int16_t valueStore, int16_t min, int16_t max, int16_t step, lv_event_cb_t event_cb, const void* user_data) {
    spinbox_obj = lv_spinbox_create(parent);
    lv_spinbox_set_range(spinbox_obj, min, max);
    lv_spinbox_set_step(spinbox_obj, step);
    lv_spinbox_set_digit_format(spinbox_obj, 3, 0); 
    lv_spinbox_set_value(spinbox_obj, valueStore);
    lv_obj_add_event_cb(spinbox_obj, event_cb, LV_EVENT_ALL, const_cast<void*>(user_data));
    lv_obj_set_size(spinbox_obj, 60, 50);
    lv_obj_set_style_bg_color(spinbox_obj, lv_color_hex(0x000000), 0); 

    lv_obj_t* increment_btn = lv_btn_create(parent);
    lv_obj_set_size(increment_btn, 25, 50);
    lv_obj_align_to(increment_btn, spinbox_obj, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_obj_add_event_cb(increment_btn, spinbox_increment_event_cb, LV_EVENT_CLICKED, spinbox_obj);
    lv_obj_set_style_bg_color(increment_btn, lv_color_hex(0x00FF00), 0); 
    lv_obj_set_style_text_color(increment_btn, lv_color_hex(0x000000), 0); 

    lv_obj_t* increment_label = lv_label_create(increment_btn);
    lv_label_set_text(increment_label, "+");
    lv_obj_align(increment_label, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t* decrement_btn = lv_btn_create(parent);
    lv_obj_set_size(decrement_btn, 25, 50);
    lv_obj_align_to(decrement_btn, spinbox_obj, LV_ALIGN_OUT_LEFT_MID, -5, 0);
    lv_obj_add_event_cb(decrement_btn, spinbox_decrement_event_cb, LV_EVENT_CLICKED, spinbox_obj);
    lv_obj_set_style_bg_color(decrement_btn, lv_color_hex(0x00FF00), 0); 
    lv_obj_set_style_text_color(decrement_btn, lv_color_hex(0x000000), 0);  

    lv_obj_t* decrement_label = lv_label_create(decrement_btn);
    lv_label_set_text(decrement_label, "-");
    lv_obj_align(decrement_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_flex_align(spinbox_obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER);

    return spinbox_obj;
}
