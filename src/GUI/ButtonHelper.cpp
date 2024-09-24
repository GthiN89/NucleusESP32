#include "ButtonHelper.h"

lv_obj_t* ButtonHelper::createButton(lv_obj_t* parent, const char* text) {
    // Create the button
    lv_obj_t* button = lv_btn_create(parent);
    if (!button) {

        return nullptr;
    }

    // Set the size of the button
    lv_obj_set_size(button, 90, 30); // Adjust size as needed

    // Create a label for the button
    lv_obj_t* label = lv_label_create(button);
    if (!label) {

        lv_obj_del(button);  // Clean up if label creation fails
        return nullptr;
    }

    // Set the text of the label and center it
    lv_label_set_text(label, text);
    lv_obj_center(label);

    return button;
}
