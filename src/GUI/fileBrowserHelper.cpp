#include "fileBrowserHelper.h"
#include "lvgl.h"

void FileBrowserHelper::load_btn_event_cb(lv_event_t* e) {
    // const char* selected_file = (const char*)lv_event_get_user_data(e);

    // if (strlen(selected_file) > 0) {
    //     FileBrowserHelper::useSelectedFile(selected_file);
    // }
}

void FileBrowserHelper::back_btn_event_cb(lv_event_t* e) {
    // Serial.println("Back button clicked.");

    // char* last_slash = strrchr(current_dir, '/');
    // if (last_slash && last_slash != current_dir) {
    //     *last_slash = '\0';  // Remove the last directory from the path

    //     // Ensure the path ends with '/'
    //     last_slash = strrchr(current_dir, '/');
    //     if (last_slash) {
    //         *(last_slash + 1) = '\0';
    //     } else {
    //         current_dir[0] = '/';
    //         current_dir[1] = '\0';
    //     }

    //     Serial.print("Updated current_dir (after going back): ");
    //     Serial.println(current_dir);
    //     updateFileList(current_dir);
    // }
}

void FileBrowserHelper::updateFileList(const char* directory) {
    // Serial.print("Updating file list for directory: ");
    // Serial.println(directory);

    // if (list == NULL) {
    //     list = lv_list_create(lv_scr_act());  // Create list on the current active screen
    //     lv_obj_set_size(list, 240, 240);
    //     Serial.println("Created new list object.");
    // } else {
    //     lv_obj_clean(list);  // Clear the existing buttons
    //     Serial.println("Cleared existing list.");
    // }

    // // Get the list of files and folders
    // file_count = getFilteredFileList(directory);
    // Serial.print("File count: ");
    // Serial.println(file_count);

    // // Populate the list with file names
    // if (file_count > 0) {
    //     for (int i = 0; i < file_count; i++) {
    //         lv_obj_t* btn = lv_list_add_btn(list, 
    //                         file_list[i][strlen(file_list[i]) - 1] == '/' ? LV_SYMBOL_DIRECTORY : LV_SYMBOL_FILE, 
    //                         file_list[i]);
    //         lv_obj_set_user_data(btn, (void*)i);  // Store the file index in user data
    //         lv_obj_add_event_cb(btn, file_btn_event_cb, LV_EVENT_CLICKED, file_list[i]);
    //         Serial.print("Added button for file: ");
    //         Serial.println(file_list[i]);
    //     }
    // } else {
    //     Serial.println("No files or folders found.");
    // }
}

int FileBrowserHelper::getFilteredFileList(const char* directory) {
    // Serial.print("Opening directory: ");
    // Serial.println(directory);

    // File dir = SD.open(directory);
    // if (!dir) {
    //     Serial.print("Failed to open directory: ");
    //     Serial.println(directory);
    //     return 0;
    // }

    // int count = 0;
    // while (true) {
    //     File entry = dir.openNextFile();
    //     if (!entry) {
    //         break; 
    //     }
    //     count++;
    //     entry.close();
    // }
    // Serial.print("Number of entries found: ");
    // Serial.println(count);

    // // Allocate memory for file_list after counting files
    // file_list = new char*[count];

    // dir.rewindDirectory();  // Reset directory reading to the start

    // count = 0;
    // while (true) {
    //     File entry = dir.openNextFile();
    //     if (!entry) {
    //         break;
    //     }

    //     int name_length = strlen(entry.name()) + (entry.isDirectory() ? 2 : 1); // Account for "/" if directory
    //     file_list[count] = new char[name_length];
    //     if (entry.isDirectory()) {
    //         snprintf(file_list[count], name_length, "%s/", entry.name());
    //     } else {
    //         snprintf(file_list[count], name_length, "%s", entry.name());
    //     }
    //     Serial.print("Added entry to file_list[");
    //     Serial.print(count);
    //     Serial.print("]: ");
    //     Serial.println(file_list[count]);

    //     count++;
    //     entry.close();
    // }

    // dir.close();
    // Serial.println("Directory closed.");
    // return count;
    return 1;
}

void FileBrowserHelper::useSelectedFile(const char* filepath) {
    // lv_label_set_text(selected_label, "Transmitting");
    // String fullPath = String(filepath);
    // Serial.print("Using file at path: ");
    // Serial.println(fullPath);

    // if (SD.exists(fullPath.c_str())) {
    //     Serial.print("Using file at path: ");
    //     Serial.println(fullPath);
    //     File flipperFile = SD.open(fullPath);
    //     Serial.println("isOpen");

    //     if (flipperFile) {
    //         transmitFlipperFile(fullPath, 1);
    //         flipperFile.close();
    //     } else {
    //         Serial.println("Failed to open file.");
    //     }
    // } else {
    //     Serial.println("File does not exist.");
    // }

    // lv_label_set_text(selected_label, "Signal transmitted");
    // SDInit(); 
}
