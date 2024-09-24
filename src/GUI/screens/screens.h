#include <LVGL.h>
#include <esp32_smartdisplay.h>
#include <FS.h>
extern const char* pathBUffer;
extern File flipperFile;

void playZeroScreen();
void createFileBrowser(lv_obj_t *parent);
void updateFileList(const char *directory);
void load_btn_event_cb(lv_event_t* e);
void back_btn_event_cb(lv_event_t* e);
void useSelectedFile(const char* filepath);


void file_btn_event_cb(lv_event_t* e);
