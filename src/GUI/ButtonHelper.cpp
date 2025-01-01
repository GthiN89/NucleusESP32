#include "ButtonHelper.h"

lv_obj_t* ButtonHelper::createButton(lv_obj_t* parent, const char* text) {
    lv_obj_t* button = lv_btn_create(parent);
    if (!button) {
        return nullptr;
    }

    lv_obj_set_size(button, 90, 30);

    lv_obj_t* label = lv_label_create(button);
    if (!label) {
        lv_obj_del(button);
        return nullptr;
    }

    lv_label_set_text(label, text);
    lv_obj_center(label);

    lv_obj_set_style_bg_color(button, lv_color_hex(0x39FF14), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(button, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(button, lv_color_hex(0x39FF14), LV_PART_MAIN);
    lv_obj_set_style_border_width(button, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(button, 8, LV_PART_MAIN);
    lv_obj_set_style_text_color(button, lv_color_hex(0x000000), LV_PART_MAIN);

    return button;
}
