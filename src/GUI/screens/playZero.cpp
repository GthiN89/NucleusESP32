#include "screens.h"
#include "modules/ETC/SDcard.h"
#include "modules/RF/subGhz.h"
#include <lvgl.h>
#include <iostream>
#include <cstring>
#include "playZero.h"
#include "modules/RF/CC1101.h"
#include "modules/dataProcessing/SubGHzParser.h"
#include "modules/dataProcessing/dataProcessing.h"

#define MAX_PATH_LENGTH 256

void createFileBrowser(lv_obj_t *parent);
void updateFileList(const char *directory);
void load_btn_event_cb(lv_event_t* e);
void back_btn_event_cb(lv_event_t* e);
void useSelectedFile(const char* filepath);
void file_btn_event_cb(lv_event_t* e);

char* current_dir;
char* selected_file;

char** file_list;  // Array of strings for file names
int file_count = 0;
lv_obj_t* list;
lv_obj_t* selected_label;
lv_obj_t* selected_btn = NULL;
File flipperFile;




const char* pathBUffer;

lv_obj_t* previous_screen = NULL;  // To store the previous screen



int getFilteredFileList(const char* directory);

void playZeroScreen() {
    Serial.println("Initializing playZeroScreen...");
    
    SDInit();
    
    // Initialize dynamic memory
    current_dir = new char[MAX_PATH_LENGTH];
    strcpy(current_dir, "/");
    Serial.print("Initialized current_dir: ");
    Serial.println(current_dir);
    
    selected_file = new char[MAX_PATH_LENGTH];
    Serial.println("Initialized selected_file.");

    file_list = nullptr;  // Initially, no files are allocated
    Serial.println("file_list set to nullptr.");
    
    // Store the current screen before switching
    previous_screen = lv_scr_act();

    // Create a new screen
    lv_obj_t* new_screen = lv_obj_create(NULL);  // Create a new screen
    lv_scr_load(new_screen);  // Load the new screen to clear the previous screen

    createFileBrowser(new_screen);  // Create the file browser on the new screen
    Serial.println("Created file browser on new screen.");
}

void createFileBrowser(lv_obj_t* parent) {
    Serial.println("Setting up file browser...");

    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    updateFileList(current_dir);  // Update the list with files and folders

    // Label to show selected file
    selected_label = lv_label_create(parent);
    lv_label_set_text(selected_label, "No file selected");
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN); // Arrange children in a row
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER); // Align buttons in the container

    //  // Create a horizontal container for the buttons
    lv_obj_t* button_container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(button_container, LV_PCT(100), 50); // Set container width to 100% of the parent and height to 50px
    lv_obj_set_flex_flow(button_container, LV_FLEX_FLOW_ROW); // Arrange children in a row
    lv_obj_set_flex_align(button_container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER); // Align buttons in the container

 // Button to load the selected file
    lv_obj_t* load_btn = lv_btn_create(button_container);
    lv_obj_add_event_cb(load_btn, load_btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t* label = lv_label_create(load_btn);
    lv_label_set_text(label, "Load File");

    // Back button for navigating to the parent directory
    lv_obj_t* back_btn = lv_btn_create(button_container);
    lv_obj_add_event_cb(back_btn, back_btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t* back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, LV_SYMBOL_LEFT "Back");

    Serial.println("File browser setup complete.");
}

void updateFileList(const char* directory) {
    Serial.print("Updating file list for directory: ");
    Serial.println(directory);

    if (list == NULL) {
        list = lv_list_create(lv_scr_act());  // Create list on the current active screen
        lv_obj_set_size(list, 240, 240);
        Serial.println("Created new list object.");
    } else {
        lv_obj_clean(list);  // Clear the existing buttons
        Serial.println("Cleared existing list.");
    }

    // Get the list of files and folders
    file_count = getFilteredFileList(directory);
    Serial.print("File count: ");
    Serial.println(file_count);

    // Populate the list with file names
    if (file_count > 0) {
        for (int i = 0; i < file_count; i++) {
            lv_obj_t* btn = lv_list_add_btn(list, 
                            file_list[i][strlen(file_list[i]) - 1] == '/' ? LV_SYMBOL_DIRECTORY : LV_SYMBOL_FILE, 
                            file_list[i]);
            lv_obj_set_user_data(btn, (void*)i);  // Store the file index in user data
            lv_obj_add_event_cb(btn, file_btn_event_cb, LV_EVENT_CLICKED, file_list[i]);
            Serial.print("Added button for file: ");
            Serial.println(file_list[i]);
        }
    } else {
        Serial.println("No files or folders found.");
    }
}

int getFilteredFileList(const char* directory) {
    Serial.print("Opening directory: ");
    Serial.println(directory);

    File dir = SD.open(directory);
    if (!dir) {
        Serial.print("Failed to open directory: ");
        Serial.println(directory);
        return 0;
    }

    int count = 0;
    while (true) {
        File entry = dir.openNextFile();
        if (!entry) {
            break; 
        }
        count++;
        entry.close();
    }
    Serial.print("Number of entries found: ");
    Serial.println(count);

    // Allocate memory for file_list after counting files
    file_list = new char*[count];

    dir.rewindDirectory();  // Reset directory reading to the start

    count = 0;
    while (true) {
        File entry = dir.openNextFile();
        if (!entry) {
            break;
        }

        int name_length = strlen(entry.name()) + (entry.isDirectory() ? 2 : 1); // Account for "/" if directory
        file_list[count] = new char[name_length];
        if (entry.isDirectory()) {
            snprintf(file_list[count], name_length, "%s/", entry.name());
        } else {
            snprintf(file_list[count], name_length, "%s", entry.name());
        }
        Serial.print("Added entry to file_list[");
        Serial.print(count);
        Serial.print("]: ");
        Serial.println(file_list[count]);

        count++;
        entry.close();
    }

    dir.close();
    Serial.println("Directory closed.");
    return count;
}

void load_btn_event_cb(lv_event_t* e) {
    CC1101_CLASS CC1101;
    Serial.println("Load button clicked.");
    if (strlen(selected_file) > 0) {
    detachInterrupt(CC1101_CCGDO0A);
    CC1101.initrRaw();
    ELECHOUSE_cc1101.setCCMode(0); 
    ELECHOUSE_cc1101.setPktFormat(3);
    ELECHOUSE_cc1101.SetTx();
    pinMode(CC1101_CCGDO0A, OUTPUT);
        Serial.print("Loading file: ");
        Serial.println(selected_file);
        useSelectedFile(selected_file);
    } else {
        Serial.println("No file selected.");
    }
}


void file_btn_event_cb(lv_event_t* e) {
    delay(500);
    Serial.println("File button clicked.");
    
 //   lv_obj_t* btn = lv_event_get_target(e);  // Get the button that was clicked THIS NEED TO FIX
    const char* file_or_folder_name = (const char*)lv_event_get_user_data(e);  // Get the file/folder name from user data

    Serial.print("File/Folder selected: ");
    Serial.println(file_or_folder_name);

    if (file_or_folder_name[strlen(file_or_folder_name) - 1] == '/') {  // If it's a folder
        Serial.println("Detected as a folder.");

        size_t dir_len = strlen(current_dir);
        size_t item_len = strlen(file_or_folder_name);

        if (dir_len + item_len < MAX_PATH_LENGTH) {
            // Append folder name to current directory
            // Ensure that the path is relative to the mount point
            strcat(current_dir, file_or_folder_name);
            Serial.print("Updated current_dir (after entering folder): ");
            Serial.println(current_dir);
            size_t length = strlen(current_dir);
            updateFileList(std::string(current_dir, length - 1).c_str());
        } else {
            Serial.println("Directory path too long, can't navigate.");
        }
    } else {  // If it's a file
        Serial.println("Detected as a file.");

        if (selected_btn) {
            lv_obj_set_style_bg_color(selected_btn, lv_color_white(), 0);  // Reset previous selection
            Serial.println("Reset previous selection.");
        }
       // selected_btn = btn;
     //   lv_obj_set_style_bg_color(selected_btn, lv_color_hex(0x88DD88), 0);  // Highlight selected file
        Serial.println("Highlighted selected file.");

        snprintf(selected_file, MAX_PATH_LENGTH, "%s%s", current_dir, file_or_folder_name);
        lv_label_set_text_fmt(selected_label, "Selected: %s", file_or_folder_name);
        Serial.print("Updated selected_file: ");
        Serial.println(selected_file);
    }
}

void back_btn_event_cb(lv_event_t* e) {
    Serial.println("Back button clicked.");

    char* last_slash = strrchr(current_dir, '/');
    if (last_slash && last_slash != current_dir) {
        *last_slash = '\0';  // Remove the last directory from the path

        // Ensure the path ends with '/'
        last_slash = strrchr(current_dir, '/');
        if (last_slash) {
            *(last_slash + 1) = '\0';
        } else {
            current_dir[0] = '/';
            current_dir[1] = '\0';
        }

        Serial.print("Updated current_dir (after going back): ");
        Serial.println(current_dir);
        updateFileList(current_dir);
    }
}



void useSelectedFile(const char* filepath) {
    SubGHzParser parser;
    parser.loadFile(filepath);
    SubGHzData data = parser.parseContent();
    parser.printParsedData(data);

    SDInit();
    lv_label_set_text(selected_label, "Transmitting");
    String fullPath = String(filepath);
    Serial.print("Using file at path: ");
    Serial.println(fullPath);

    if (SD.exists(fullPath.c_str())) {
        read_sd_card_flipper_file(fullPath.c_str());
        C1101CurrentState = STATE_SEND_FLIPPER;
        delay(1000);
    } else {
        Serial.println("File does not exist.");
        lv_label_set_text(selected_label, "File not found");
        return;  // Exit if file does not exist
    }

        // Signal transmitted, so let's refresh the screen
    lv_label_set_text(selected_label, "Signal transmitted");
}
