// File: src/GUI/ContainerHelper.cpp

#include "ContainerHelper.h"
#include <lvgl.h>
#include <cstring>
#include <Arduino.h>
#include <string>
// // Validator funkce pro numerický vstup
// static bool number_validator(lv_obj_t* ta, const char* txt) {
//     // Loop through the characters in the input string
//     for (size_t i = 0; i < strlen(txt); i++) {
//         // Check if the character is not a digit or a dot
//         if (!((txt[i] >= '0' && txt[i] <= '9') || txt[i] == '.')) {
//             return false; // Invalid character
//         }
//     }
//     return true; // All characters are valid
// }
#include "lvgl.h"

// // Updated number validator function to check if input only contains numbers or a dot
// static bool number_validator(const char* txt) {
//     for (size_t i = 0; i < strlen(txt); i++) {
//         if (!((txt[i] >= '0' && txt[i] <= '9') || txt[i] == '.')) {
//             return false; // Invalid character
//         }
//     }
//     return true; // All characters are valid
// }

// // Event handler for validating text input in a text area
// static void textarea_event_handler(lv_event_t* e) {
//     lv_obj_t* ta = lv_event_get_target(e);
//     const char* txt = lv_textarea_get_text(ta);

//     // Validate the text when it changes
//     if (!number_validator(txt)) {
//         // Optionally, modify this to handle invalid inputs (e.g., revert changes)
//         // For now, this example just prevents the insertion of invalid text
//         lv_textarea_del_char(ta);  // Deletes the last character if invalid
//     }
// }

void ContainerHelper::createContainer(lv_obj_t** containerRef, lv_obj_t* parent, lv_flex_flow_t flex_flow, lv_coord_t height, lv_coord_t width) {
    // Vytvoření kontejneru a uložení reference na něj
    *containerRef = lv_obj_create(parent);
    lv_obj_set_flex_flow(*containerRef, flex_flow);
    lv_obj_set_size(*containerRef, width, height);
    lv_obj_set_flex_align(*containerRef, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(*containerRef, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(*containerRef, LV_OBJ_FLAG_SCROLLABLE);

    // Nastavení flex alignment podle typu rozvržení
    switch (flex_flow) {
        case LV_FLEX_FLOW_ROW:
            lv_obj_set_flex_align(*containerRef, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            break;
        case LV_FLEX_FLOW_COLUMN:
            lv_obj_set_flex_align(*containerRef, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
            break;
        default:
            // Pro další rozvržení
            break;
    }
}

void ContainerHelper::fillTopContainer(lv_obj_t* parent, const char* labelText, InputType inputType,
                                       lv_obj_t** input, const char* placeholder, const char* initialText,
                                       size_t maxLength, lv_obj_t* kb, lv_event_cb_t event_cb,
                                       const char* options[], int optionCount) {
    // Create label
    lv_obj_t* label = lv_label_create(parent);
    lv_label_set_text(label, labelText);
    

    // Create input field based on type
    switch (inputType) {
        case TEXT_AREA: {
            *input = lv_textarea_create(parent);
            lv_textarea_set_placeholder_text(*input, placeholder);
            lv_textarea_set_text(*input, initialText);
            lv_textarea_set_max_length(*input, maxLength);
            lv_obj_clear_flag(*input, LV_OBJ_FLAG_SCROLLABLE); // Double ensure

            if (event_cb && kb) {
                // Assign the event callback and pass kb as user data
                lv_obj_add_event_cb(*input, event_cb, LV_EVENT_ALL, kb);
            }
        }
        break;

        case BUTTON: {
            *input = lv_btn_create(parent);
            lv_obj_set_size(*input, 90, 30);

            // Create label inside the button, making the button parent of the label
            lv_obj_t* labelButton = lv_label_create(*input); 
            lv_label_set_text(labelButton, placeholder);  // Set label text as the placeholder
            lv_obj_center(labelButton);  // Center the label inside the button

            lv_obj_clear_flag(*input, LV_OBJ_FLAG_SCROLLABLE); // Double ensure

            if (event_cb) {
                lv_obj_add_event_cb(*input, event_cb, LV_EVENT_ALL, NULL);
            }
            break;
        }

        case DROPDOWN: {
            *input = lv_dropdown_create(parent);
            
            // Check if options are provided and set them
            if (options && optionCount > 0) {
                std::string optionsString;
                for (int i = 0; i < optionCount; ++i) {
                    if (i > 0) {
                        optionsString += "\n";  // Add a newline separator between options
                    }
                    optionsString += options[i];
                }
                lv_dropdown_set_options(*input, optionsString.c_str());
            } else {
                // Use initialText if no options are provided
                lv_dropdown_set_options(*input, initialText);
            }

            if (event_cb && kb) {
                lv_obj_add_event_cb(*input, event_cb, LV_EVENT_ALL, kb);
            }
        }
            break;

        // Handle other input types if necessary

        default:
            // Optional: Handle unknown input types
            break;
    }
}

// // Function to create a container with a label and input fields
// void ContainerHelper::createContainer(lv_obj_t* parent, const char* label_text, InputType type, const char* placeholder, const char* initial_text, size_t max_length, DropdownCallback dropdown_cb) {
//     // Create the container and label
//     lv_obj_t* container = lv_obj_create(parent);
//     lv_obj_set_flex_flow(container, LV_FLEX_FLOW_ROW);
//     lv_obj_set_flex_align(container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
//     lv_obj_set_size(container, LV_PCT(100), LV_SIZE_CONTENT);

//     lv_obj_t* label = lv_label_create(container);
//     lv_label_set_text(label, label_text);
//     lv_obj_set_width(label, 80); // Set label width

//     lv_obj_t* input = nullptr;

//     switch (type) {
//         case TEXT_AREA:
//             input = lv_textarea_create(container);
//             lv_textarea_set_one_line(input, true);
//             break;
//         case NUMBER:
//             input = lv_textarea_create(container);
//             lv_textarea_set_one_line(input, true);
//             // Add event handler for numeric input validation
//             lv_obj_add_event_cb(input, textarea_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
//             break;
//         case DROPDOWN:
//             // Implementation of dropdown menu creation
//             // ...
//             break;
//     }

//     if (input) {
//         lv_textarea_set_placeholder_text(input, placeholder);
//         lv_textarea_set_text(input, initial_text);
//         lv_textarea_set_max_length(input, max_length);
//         lv_obj_set_width(input, 140); // Set input width
//         // Additional settings as needed
//     }

//     // If a dropdown callback is needed
//     if (type == DROPDOWN && dropdown_cb) {
//         // Implementation of dropdown callback
//         // ...
//     }
// }
