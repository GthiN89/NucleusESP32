// #ifndef FILE_BROWSER_H
// #define FILE_BROWSER_H

// #include <lvgl.h>              // Include LVGL library
// #include <SD.h>                // Include SD card library
// #include "modules/RF/subGhz.h" // Include your specific module


// // Function declarations

// /**
//  * @brief Creates the file browser UI.
//  * 
//  * @param parent The parent LVGL object to which the file browser will be added.
//  */
// void createFileBrowser(lv_obj_t* parent);


// void playZeroScreen();

// /**
//  * @brief Updates the file list based on the given directory.
//  * 
//  * @param directory The directory path to list files from.
//  */
// void updateFileList(const String directory);

// /**
//  * @brief Retrieves a filtered list of files from the specified directory.
//  * 
//  * @param directory The directory path to filter files from.
//  * @return int The number of files found.
//  */
// int getFilteredFileList(const char* directory);

// /**
//  * @brief Callback function for handling the load button event.
//  * 
//  * @param e The event object from LVGL.
//  */
// void load_btn_event_cb(lv_event_t* e);

// /**
//  * @brief Processes the selected file.
//  * 
//  * @param filepath The path of the file to be processed.
//  */
// void useSelectedFile(const char* filepath);

// /**
//  * @brief Callback function for handling file button click events.
//  * 
//  * @param e The event object from LVGL.
//  */
// void file_btn_event_cb(lv_event_t* e);

// /**
//  * @brief Callback function for handling the back button click event.
//  * 
//  * @param e The event object from LVGL.
//  */
// void back_btn_event_cb(lv_event_t* e);

// #endif // FILE_BROWSER_H
