#ifndef FILEBROWSER_HELPER
#define FILEBROWSER_HELPER
#include "lvgl.h"

class FileBrowserHelper {
public:
        void useSelectedFile(const char* filepath);
        int getFilteredFileList(const char* directory);
        void updateFileList(const char* directory);
        static void back_btn_event_cb(lv_event_t* e);
        static void load_btn_event_cb(lv_event_t* e);

private:




};







#endif 