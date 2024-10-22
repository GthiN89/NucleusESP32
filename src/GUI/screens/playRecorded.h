// #ifndef PLAYRECORDED_H
// #define PLAYRECORDED_H

// #include <lvgl.h>

// // Constants
// #define MAX_PATH_LENGTH 256

// // Function Declarations

// // Main screen creation function
// void playRecordedScreen();

// // Create file browser
// void playRecordedCreateFileBrowser(lv_obj_t *parent);

// // Update file list in the browser
// void playRecordedUpdateFileList(const char *directory);

// // Event callback for load button
// void playRecordedLoadBtnEventCb(lv_event_t* e);

// // Event callback for back button
// void playRecordedBackBtnEventCb(lv_event_t* e);

// // Use selected file for transmission
// void playRecordedUseSelectedFile(const char* filepath);

// // Event callback for file button
// void playRecordedFileBtnEventCb(lv_event_t* e);

// // Function to get filtered file list (only .rf files)
// int playRecordedGetFilteredFileList(const char* directory);

// // Global Variables

// extern char* playRecordedCurrentDir;
// extern char* playRecordedSelectedFile;
// extern char** playRecordedFileList;
// extern int playRecordedFileCount;
// extern lv_obj_t* playRecordedList;
// extern lv_obj_t* playRecordedSelectedLabel;
// extern lv_obj_t* playRecordedSelectedBtn;
// //extern File playRecordedFile;

// #endif // PLAYRECORDED_H
