

#ifndef CONTAINER_HELPER_H
#define CONTAINER_HELPER_H

#include <lvgl.h>

// Definice typů vstupů
enum InputType {
    TEXT_AREA,
    NUMBER,
    DROPDOWN,
    BUTTON
};



// Typ callbacku pro dropdown menu
typedef void (*DropdownCallback)(lv_event_t* e);

class ContainerHelper {
public:
    // Metoda pro vytvoření kontejneru s flexibilním rozložením a rozměry
    static void createContainer(lv_obj_t** containerRef, lv_obj_t* parent, lv_flex_flow_t flex_flow, lv_coord_t height, lv_coord_t width);
  void fillTopContainer(lv_obj_t* parent, const char* labelText, InputType inputType,
                                       lv_obj_t** input, const char* placeholder, const char* initialText,
                                       size_t maxLength, lv_obj_t* kb, lv_event_cb_t event_cb,
                                       const char* options[] = nullptr, int optionCount = 0);


};

#endif // CONTAINER_HELPER_H
