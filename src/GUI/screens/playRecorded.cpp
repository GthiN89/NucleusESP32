// #include "screens.h"
// #include "modules/ETC/SDcard.h"
// //#include "modules/RF/subGHz.h"
// #include <lvgl.h>
// #include <iostream>
// #include <cstring>

// #define MAX_PATH_LENGTH 256

// void playRecordedCreateFileBrowser(lv_obj_t *parent);
// void playRecordedUpdateFileList(const char *directory);
// void playRecordedLoadBtnEventCb(lv_event_t* e);
// void playRecordedBackBtnEventCb(lv_event_t* e);
// void playRecordedUseSelectedFile(const char* filepath);
// void playRecordedFileBtnEventCb(lv_event_t* e);

// char* playRecordedCurrentDir;
// char* playRecordedSelectedFile;

// char** playRecordedFileList;  // Array of strings for file names
// int playRecordedFileCount = 0;
// lv_obj_t* playRecordedList;
// lv_obj_t* playRecordedSelectedLabel;
// lv_obj_t* playRecordedSelectedBtn = NULL;
// File playRecordedFile;

// lv_obj_t* playRecordedPreviousScreen = NULL;  // To store the previous screen

// unsigned long recordedValue = 0; 
// int recordedBitLength = 0;       
// int recordedProtocol = 0;       
// unsigned long frequency = 0;  
// const int rssi_threshold = -75; 

// // RCSwitch rfSwitch = RCSwitch();
// // CC1101 cc1101_1(CC1101_SCLK, CC1101_MISO, CC1101_MOSI, CC1101_CS, CCGDO2A, CCGDO0A);


// int playRecordedGetFilteredFileList(const char* directory);

// void playRecordedScreen() {
//     // Serial.println("Initializing playRecordedScreen...");

//     // SDInit();

//     // // Initialize dynamic memory
//     // playRecordedCurrentDir = new char[MAX_PATH_LENGTH];
//     // strcpy(playRecordedCurrentDir, "/");  // Start the file browser in the recordedRF folder
//     // Serial.print("Initialized playRecordedCurrentDir: ");
//     // Serial.println(playRecordedCurrentDir);

//     // playRecordedSelectedFile = new char[MAX_PATH_LENGTH];
//     // Serial.println("Initialized playRecordedSelectedFile.");

//     // playRecordedFileList = nullptr;  // Initially, no files are allocated
//     // Serial.println("playRecordedFileList set to nullptr.");

//     // // Store the current screen before switching
//     // playRecordedPreviousScreen = lv_scr_act();

//     // // Create a new screen
//     // lv_obj_t* new_screen = lv_obj_create(NULL);  // Create a new screen
//     // lv_scr_load(new_screen);  // Load the new screen to clear the previous screen

//     // playRecordedCreateFileBrowser(new_screen);  // Create the file browser on the new screen
//     // Serial.println("Created file browser on new screen.");
// }

// void playRecordedCreateFileBrowser(lv_obj_t* parent) {
//     Serial.println("Setting up file browser...");

//     // lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
//     // playRecordedUpdateFileList(playRecordedCurrentDir);  // Update the list with files and folders

//     // // Label to show selected file
//     // playRecordedSelectedLabel = lv_label_create(parent);
//     // lv_label_set_text(playRecordedSelectedLabel, "No file selected");
//     // lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);  // Arrange children in a row
//     // lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);  // Align buttons in the container

//     // // Create a horizontal container for the buttons
//     // lv_obj_t* button_container = lv_obj_create(lv_scr_act());
//     // lv_obj_set_size(button_container, LV_PCT(100), 50);  // Set container width to 100% of the parent and height to 50px
//     // lv_obj_set_flex_flow(button_container, LV_FLEX_FLOW_ROW);  // Arrange children in a row
//     // lv_obj_set_flex_align(button_container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);  // Align buttons in the container

//     // // Button to load the selected file
//     // lv_obj_t* load_btn = lv_btn_create(button_container);
//     // lv_obj_add_event_cb(load_btn, playRecordedLoadBtnEventCb, LV_EVENT_CLICKED, NULL);
//     // lv_obj_t* label = lv_label_create(load_btn);
//     // lv_label_set_text(label, "Load File");

//     // // Back button for navigating to the parent directory
//     // lv_obj_t* back_btn = lv_btn_create(button_container);
//     // lv_obj_add_event_cb(back_btn, playRecordedBackBtnEventCb, LV_EVENT_CLICKED, NULL);
//     // lv_obj_t* back_label = lv_label_create(back_btn);
//     // lv_label_set_text(back_label, LV_SYMBOL_LEFT "Back");

//     // Serial.println("File browser setup complete.");
// }

// void playRecordedUpdateFileList(const char* directory) {
//     // Serial.print("Updating file list for directory: ");
//     // Serial.println(directory);

//     // if (playRecordedList == NULL) {
//     //     playRecordedList = lv_list_create(lv_scr_act());  // Create list on the current active screen
//     //     lv_obj_set_size(playRecordedList, 240, 240);
//     //     Serial.println("Created new list object.");
//     // } else {
//     //     lv_obj_clean(playRecordedList);  // Clear the existing buttons
//     //     Serial.println("Cleared existing list.");
//     // }

//     // // Get the list of files and folders
//     // playRecordedFileCount = playRecordedGetFilteredFileList(directory);
//     // Serial.print("File count: ");
//     // Serial.println(playRecordedFileCount);

//     // // Populate the list with file names
//     // if (playRecordedFileCount > 0) {
//     //     for (int i = 0; i < playRecordedFileCount; i++) {
//     //         lv_obj_t* btn = lv_list_add_btn(playRecordedList, 
//     //                         playRecordedFileList[i][strlen(playRecordedFileList[i]) - 1] == '/' ? LV_SYMBOL_DIRECTORY : LV_SYMBOL_FILE, 
//     //                         playRecordedFileList[i]);
//     //         lv_obj_set_user_data(btn, (void*)i);  // Store the file index in user data
//     //         lv_obj_add_event_cb(btn, playRecordedFileBtnEventCb, LV_EVENT_CLICKED, playRecordedFileList[i]);
//     //         Serial.print("Added button for file: ");
//     //         Serial.println(playRecordedFileList[i]);
//     //     }
//     // } else {
//     //     Serial.println("No files or folders found.");
//     // }
// }

// int playRecordedGetFilteredFileList(const char* directory) {
//     // Serial.print("Opening directory: ");
//     // Serial.println(directory);

//     // File dir = SD.open(directory);
//     // if (!dir) {
//     //     Serial.print("Failed to open directory: ");
//     //     Serial.println(directory);
//     //     return 0;
//     // }

//     // int count = 0;
//     // while (true) {
//     //     File entry = dir.openNextFile();
//     //     if (!entry) {
//     //         break;
//     //     }
//     //     count++;
//     //     entry.close();
//     // }
//     // Serial.print("Number of entries found: ");
//     // Serial.println(count);

//     // // Allocate memory for playRecordedFileList after counting files
//     // playRecordedFileList = new char*[count];

//     // dir.rewindDirectory();  // Reset directory reading to the start

//     // count = 0;
//     // while (true) {
//     //     File entry = dir.openNextFile();
//     //     if (!entry) {
//     //         break;
//     //     }

//     //     int name_length = strlen(entry.name()) + (entry.isDirectory() ? 2 : 1);  // Account for "/" if directory
//     //     playRecordedFileList[count] = new char[name_length];
//     //     if (entry.isDirectory()) {
//     //         snprintf(playRecordedFileList[count], name_length, "%s/", entry.name());
//     //     } else if (strstr(entry.name(), ".rf")) {  // Filter only `.rf` files
//     //         snprintf(playRecordedFileList[count], name_length, "%s", entry.name());
//     //     } else {
//     //         continue;
//     //     }

//     //     Serial.print("Added entry to playRecordedFileList[");
//     //     Serial.print(count);
//     //     Serial.print("]: ");
//     //     Serial.println(playRecordedFileList[count]);

//     //     count++;
//     //     entry.close();
//     // }

//     // dir.close();
//     // Serial.println("Directory closed.");
//     // return count;
// }

// void playRecordedLoadBtnEventCb(lv_event_t* e) {
//     Serial.println("Load button clicked.");
//     // if (strlen(playRecordedSelectedFile) > 0) {
//     //     Serial.print("Loading file: ");
//     //     Serial.println(playRecordedSelectedFile);
//     //     playRecordedUseSelectedFile(playRecordedSelectedFile);
//     // } else {
//     //     Serial.println("No file selected.");
//     // }
// }

// void playRecordedFileBtnEventCb(lv_event_t* e) {
//     // delay(500);
//     // Serial.println("File button clicked.");

//     // const char* file_or_folder_name = (const char*)lv_event_get_user_data(e);  // Get the file/folder name from user data

//     // Serial.print("File/Folder selected: ");
//     // Serial.println(file_or_folder_name);

//     // if (file_or_folder_name[strlen(file_or_folder_name) - 1] == '/') {  // If it's a folder
//     //     Serial.println("Detected as a folder.");

//     //     size_t dir_len = strlen(playRecordedCurrentDir);
//     //     size_t item_len = strlen(file_or_folder_name);

//     //     if (dir_len + item_len < MAX_PATH_LENGTH) {
//     //         strcat(playRecordedCurrentDir, file_or_folder_name);  // Append folder name to current directory
//     //         Serial.print("Updated playRecordedCurrentDir (after entering folder): ");
//     //         Serial.println(playRecordedCurrentDir);
//     //         size_t length = strlen(playRecordedCurrentDir);
//     //         playRecordedUpdateFileList(std::string(playRecordedCurrentDir, length - 1).c_str());
//     //     } else {
//     //         Serial.println("Directory path too long, can't navigate.");
//     //     }
//     // } else {  // If it's a file
//     //     Serial.println("Detected as a file.");

//     //     if (playRecordedSelectedBtn) {
//     //         lv_obj_set_style_bg_color(playRecordedSelectedBtn, lv_color_white(), 0);  // Reset previous selection
//     //         Serial.println("Reset previous selection.");
//     //     }

//     //     snprintf(playRecordedSelectedFile, MAX_PATH_LENGTH, "%s%s", playRecordedCurrentDir, file_or_folder_name);
//     //     lv_label_set_text_fmt(playRecordedSelectedLabel, "Selected: %s", file_or_folder_name);
//     //     Serial.print("Updated playRecordedSelectedFile: ");
//     //     Serial.println(playRecordedSelectedFile);
//     // }
// }

// void playRecordedBackBtnEventCb(lv_event_t* e) {
//     // Serial.println("Back button clicked.");

//     // char* last_slash = strrchr(playRecordedCurrentDir, '/');
//     // if (last_slash && last_slash != playRecordedCurrentDir) {
//     //     *last_slash = '\0';  // Remove the last directory from the path

//     //     last_slash = strrchr(playRecordedCurrentDir, '/');
//     //     if (last_slash) {
//     //         *(last_slash + 1) = '\0';  // Ensure the path ends with '/'
//     //     } else {
//     //         playRecordedCurrentDir[0] = '/';
//     //         playRecordedCurrentDir[1] = '\0';
//     //     }

//     //     Serial.print("Updated playRecordedCurrentDir (after going back): ");
//     //     Serial.println(playRecordedCurrentDir);
//     //     playRecordedUpdateFileList(playRecordedCurrentDir);
//     // }
// }

// void playRecordedUseSelectedFile(const char* filepath) {
// //     lv_label_set_text(playRecordedSelectedLabel, "Transmitting");
// //     String fullPath = String(filepath);
// //     Serial.print("Using file at path: ");
// //     Serial.println(fullPath);

// //     if (SD.exists(fullPath.c_str())) {
// //         File playRecordedFile = SD.open(fullPath);
// //         Serial.println("File opened");

// //         if (playRecordedFile) {
// //             char key[17];
// //             // Implement the RF transmission logic based on the file content here
// //             // Example: load signal from the file, send the signal using your RF logic
// //             if (playRecordedFile) {
// //                playRecordedFile.read((uint8_t*)&frequency, sizeof(frequency));  // Read frequency
// //             playRecordedFile.read((uint8_t*)&recordedProtocol, sizeof(recordedProtocol));  // Read protocol
// //             playRecordedFile.read((uint8_t*)&recordedBitLength, sizeof(recordedBitLength));  // Read bit length
// //             playRecordedFile.read((uint8_t*)&recordedValue, sizeof(recordedValue));  // Read the key

// //             // Close the file after reading
// //             playRecordedFile.close();
// //             disconnectSD();

// //             // Debugging Output
// //             Serial.print("Frequency (Hz): ");
// //             Serial.println(frequency);
// //             Serial.print("Protocol: ");
// //             Serial.println(recordedProtocol);
// //             Serial.print("Bit Length: ");
// //             Serial.println(recordedBitLength);
// //             Serial.print("Key: ");
// //             Serial.println(recordedValue, HEX);  // Print the key as hex
// //             }
// //             playRecordedFile.close();

// //             disconnectSD();
// //             Serial.println("code sending");
// //             CC1101_MHZ = frequency; // No conversion needed, using the frequency directly
// //         //  initCC1101();
// //             delay(100);
// //             rfSwitch.enableTransmit(CCGDO0A); 
// //             ELECHOUSE_cc1101.SetTx();
// //             rfSwitch.setProtocol(recordedProtocol);
// //             rfSwitch.send(recordedValue, recordedBitLength); 

// //             delay(500);

// //             ELECHOUSE_cc1101.SetRx(); 
// //             rfSwitch.disableTransmit(); 
// //             delay(100);
// //             rfSwitch.enableReceive(CCGDO2A); 
// //         } else {
// //             Serial.println("Failed to open file.");
// //         }
// //     } else {
// //         Serial.println("File does not exist.");
// //     }

// //     lv_label_set_text(playRecordedSelectedLabel, "Signal transmitted");
// //     SDInit();  // Reinitialize the SD card
//  }

