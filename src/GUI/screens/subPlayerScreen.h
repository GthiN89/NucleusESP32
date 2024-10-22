#ifndef SUB_PLAYER_SCREEN_H
#define SUB_PLAYER_SCREEN_H

#include <lvgl.h>
#include "../ScreenManager.h"
#include <lvgl.h>              // Include LVGL library
#include <SD.h>                // Include SD card library


class SubPlayerScreen {
public:
    SubPlayerScreen();
    ~SubPlayerScreen();

    void initialize();
    void back_btn_event_cb(lv_event_t* e);
    void file_btn_event_cb(lv_event_t* e);
    void useSelectedFile(const char* filepath);
    void load_btn_event_cb(lv_event_t* e);
    int getFilteredFileList(const char* directory);
    void updateFileList(const String directory);

private:
    ScreenManager& screenManager_;
};

#endif // SUB_PLAYER_SCREEN_H
