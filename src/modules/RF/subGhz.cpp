#include "modules/ETC/SDcard.h"
#include "GUI/screens/screens.h"
#include <ELECHOUSE_CC1101_SRC_DRV.h>
//#include <RCSwitch.h>
#include "subGhz.h"
#include <lvgl.h>
#include "GUI/screens/sendTesla.h"
#include <stdint.h>  // For int32_t

// Initialize SD card file definitions
File root;
File recorded_signal_file;




volatile long last_RXmillis; //CC1101 Receive timer
//volatile long last_micros; // Replace 'long' with 'int32_t'
String RXbuffer; // RX buffer

// CC1101 parameters
float CC1101_MHZ_ = 433.92;
bool CC1101_TX_ = false;
int CC1101_MODULATION_ = 2;
int CC1101_DRATE = 512;
float CC1101_RX_BW = 256;
int CC1101_PKT_FORMAT_ = 3;
float CC1101_LAST_AVG_LQI = 0;
float CC1101_LAST_AVG_RSSI = 0;
int CC1101ScanEn = 0;





// int   teslaTextLine = 0;
// lv_obj_t **labels = new lv_obj_t*[teslaTextLine]; 
bool teslaRunningFlag = false;
bool teslaSucessFlag_ = false;
bool teslaMenuIsOpen;





//int NUM_LABELS = 0;




//END



#define MAX_FILE_SIZE 1024 * 1024  // 1MB
#define BUFFER_SIZE 16 * 1024            // 16KB

char buffer[BUFFER_SIZE];
int bufferIndex = 0;

bool transmitFlipperFile(String filename, bool transmit);
void handleFlipperCommandLine(String command, String value);
bool sendSamples(int samples[], int samplesLength);
//void addLineToTeslaContainer(lv_obj_t *cont, lv_obj_t **labels, int teslaTextLine, String text);
//void addLineToTeslaContainer(lv_obj_t **cont, lv_obj_t **labels, int teslaTextLine, String text);

bool setZeroName(char* current_dir, char* selected_file) {
    Serial.println("Entering setZeroName");
    Serial.print("Current Directory: ");
    Serial.println(current_dir);
    Serial.print("Selected File: ");
    Serial.println(selected_file);

    // Uncomment this return true line to execute further code
    return true;

    String fullfilename = "";
    fullfilename += pathBUffer;
    CC1101_TX_ = false;
    ELECHOUSE_cc1101.setPA(12);
    Serial.println("Attempting to transmit file");
    if (transmitFlipperFile(String(fullfilename).c_str(), true)) {
        Serial.println("Transmission successful");
        delay(20);
        return true;
    }
    Serial.println("Transmission failed");
    return false;      
}

bool initCC1101() {
    digitalWrite(5, HIGH);
     SPI.begin();
    Serial.println("Initializing CC1101");
    ELECHOUSE_cc1101.setSpiPin(CC1101_SCLK, CC1101_MISO, CC1101_MOSI, CC1101_CS);
    ELECHOUSE_cc1101.Init();
    ELECHOUSE_cc1101.setGDO(CCGDO0A, CCGDO2A);
    ELECHOUSE_cc1101.setMHZ(CC1101_MHZ_);
    ELECHOUSE_cc1101.setPA(12);
    if (CC1101_TX_) {
        ELECHOUSE_cc1101.SetTx();
        pinMode(CCGDO0A, OUTPUT);  
    } else {
        ELECHOUSE_cc1101.SetRx();
        pinMode(CCGDO2A, INPUT);  
    }
    ELECHOUSE_cc1101.setModulation(CC1101_MODULATION_);
    ELECHOUSE_cc1101.setDRate(CC1101_DRATE);
    ELECHOUSE_cc1101.setRxBW(CC1101_RX_BW);
    ELECHOUSE_cc1101.setPktFormat(CC1101_PKT_FORMAT_);

    if (!ELECHOUSE_cc1101.getCC1101()) {
        Serial.println("CC1101 Connection Error");
        return false;
    } else {
        Serial.println("CC1101 Initialized successfully");
        return true;
    }
}

//flipper

bool transmitFlipperFile(String filename, bool transmit) {
    Serial.println("Entering transmitFlipperFile");
    Serial.print("Filename: ");
    Serial.println(filename);
    Serial.print("Transmit: ");
    Serial.println(transmit);

    // Open the file
    File flipperFile = SD.open(filename);
    if (!flipperFile) {
        Serial.println("Failed to open file");
        return false;
    }

    // Check file size
    unsigned long fileSize = flipperFile.size();
    if (fileSize > MAX_FILE_SIZE) {
        Serial.println("Error: File size too large to process");
        flipperFile.close();
        return false;
    }

    // Read file content into buffer
    String command = "";
    String value = "";
    int data;
    char dataChar;
    bool appendCommand = true;
    bool breakLoop = false;
    int samples[512];
    int currentSample = 0;

    Serial.println("Buffering file data...");

    while ((data = flipperFile.read()) >= 0 && !breakLoop) {
        dataChar = data;
        buffer[bufferIndex++] = dataChar;

        if (bufferIndex >= BUFFER_SIZE) {
            Serial.println("Buffer full, stopping buffer load.");
            breakLoop = true;
        }

        switch (dataChar) {
            case ':':
                appendCommand = false;
                break;
            case '\n':
                while (value.startsWith(" ")) {
                    value = value.substring(1);
                }

                Serial.println("DUMP:");
                Serial.println(command + " | " + value);

                if (!transmit) {
                    handleFlipperCommandLine(command, value);
                } else {
                    handleFlipperCommandLine(command, value);
                    if (command == "RAW_Data" && transmit) {
                        if (sendSamples(samples, 512)) {
                            Serial.println("Samples transmitted successfully");
                        } else {
                            Serial.println("Sample transmission failed");
                        }
                    }
                }

                appendCommand = true;
                command = "";
                value = "";
                currentSample = 0;
                memset(samples, 0, sizeof(samples));
                break;
            default:
                if (appendCommand) {
                    command += String(dataChar);
                } else {
                    value += String(dataChar);

                    if (command == "RAW_Data") {
                        if (dataChar == ' ') {
                            value.replace(" ", "");
                            if (!value.isEmpty()) {
                                samples[currentSample++] = value.toInt();
                                value = "";
                            }
                        }
                    }
                }
                break;
        }
    }

    if (bufferIndex < BUFFER_SIZE) {
        Serial.println("File data buffered successfully.");
    } else {
        Serial.println("Warning: Buffer was filled before the file was completely read.");
    }

    flipperFile.close();


    disconnectSD();


    // Initialize CC1101 for transmission after buffering
    if (transmit) {
        Serial.println("Re-initializing CC1101 for transmission");
        if (!initCC1101()) {
            Serial.println("Failed to initialize CC1101");
            return false;
        }
    }

    Serial.println("Starting transmission of buffered data...");
    for (int i = 0; i < bufferIndex; i++) {

        Serial.write(buffer[i]);
    }

    Serial.println("Buffered data transmission completed.");
     // Reconnect SD card to SPI
    SPI.begin();
    if (!SD.begin(MICRO_SD_IO)) {
        Serial.println("Failed to reinitialize SD card");
        return false;
    }
    Serial.println("SD card reconnected.");
    return true;
}

bool sendSamples(int samples[], int samplesLength) {
    Serial.println("Entering sendSamples");
    if (!CC1101_TX_) {
        CC1101_TX_ = true;
        Serial.println("Switching CC1101 to TX mode");
        if (!initCC1101()) {
            Serial.println("Failed to initialize CC1101 for TX");
            return false;
        }
    }

    int delay = 0;
    unsigned long time;
    byte n = 0;

    for (int i = 0; i < samplesLength; i++) {
        n = 1;
        delay = samples[i];
        if (delay < 0) {
            delay *= -1;
            n = 0;
        }
        digitalWrite(CCGDO0A, n);
        delayMicroseconds(delay);
        Serial.print("Transmitted: ");
        Serial.print(n);
        Serial.print(" with delay: ");
        Serial.println(delay);
    }

    digitalWrite(CCGDO0A, 0);
    Serial.println("Transmission completed");
    return true;
}

void handleFlipperCommandLine(String command, String value) {
    Serial.print("Handling command: ");
    Serial.print(command);
    Serial.print(" with value: ");
    Serial.println(value);

    if (command == "Frequency") {
        float frequency = value.toFloat() / 1000000;
        String Fq = "Freq: ";
        Fq.concat(frequency);
        Serial.println(Fq);
        ELECHOUSE_cc1101.setMHZ(frequency);
        CC1101_MHZ_ = frequency;
    }
}







// /////// END CC1101 SCAN Functions ///////////////////////////////////





// // MAIN LOOP FUNCTION --------------------------------------------------------------------------- MAIN LOOP FUNCTION //
// void loop()
// {
  
//   if (OTAInProgress==0){
//     audio.loop();
//   }

//   if (OTAInProgress==1){
//   ArduinoOTA.handle();
//   server.handleClient();
//   }

//   if(ProtAnaRxEn==1){
//      ProtAnalyzerloop();
//     }

//   if(CC1101ScanEn==1){
//     CC1101Scanloop();
//   }


//   }

//  /// AUX FUNCTIONS FOR LCD ----------------------------------------------------------------------- AUX FUNCTIONS FOR LCD //

//   /*** Display callback to flush the buffer to screen ***/
//   void display_flush(lv_disp_drv_t * disp, const lv_area_t *area, lv_color_t *color_p)
//   {
//     uint32_t w = (area->x2 - area->x1 + 1);
//     uint32_t h = (area->y2 - area->y1 + 1);

//     lcd.startWrite();
//     lcd.setAddrWindow(area->x1, area->y1, w, h);
//     lcd.pushColors((uint16_t *)&color_p->full, w * h, true);
//     lcd.endWrite();

//     lv_disp_flush_ready(disp);
//   }

//   /*** Touchpad callback to read the touchpad ***/
//   void touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data)
//   {
//     uint16_t touchX, touchY;
//     bool touched = lcd.getTouch(&touchX, &touchY);

//     if (!touched)
//     {
//       data->state = LV_INDEV_STATE_REL;
//     }
//     else
//     {
//       data->state = LV_INDEV_STATE_PR;

//       /*Set the coordinates*/
//       data->point.x = touchX;
//       data->point.y = touchY;

//       // Serial.printf("Touch (x,y): (%03d,%03d)\n",touchX,touchY );
//     }
//   }

// // Get directory names from SD card and populate first dropdown
// void populateDirectoryDropdown() {
//   lv_dropdown_clear_options(ui_ddPresetsFolder);
  
//   root = SD.open("/");
//   int i = 0;
//   while (true) {
//     File entry = root.openNextFile();
//     if (!entry) {
//       break;
//     }
//     if (entry.isDirectory()) {
//       lv_dropdown_add_option(ui_ddPresetsFolder, entry.name(),LV_DROPDOWN_POS_LAST);
//       strncpy(directoryNames[i], entry.name(), sizeof(directoryNames[i]));
//       i++;
//       if (i >= 20) {
//         break;
//       }
//     }
//     entry.close();
//   }
  
//   //populateFileDropdown();
// }

// // Get file names from selected directory and populate second dropdown
// void populateFileDropdown(lv_event_t * e) {

//   lv_dropdown_clear_options(ui_ddPresetsFile);
//   int i = 0;
//   int dirIndex = lv_dropdown_get_selected(ui_ddPresetsFolder);
//   String directoryPath = "/";
//   directoryPath += directoryNames[dirIndex];
//   File dir = SD.open(directoryPath.c_str());
//   while (true) {
//     File entry = dir.openNextFile();
//     if (!entry) {
//       break;
//     }
//     if (!entry.isDirectory()) {
//       lv_dropdown_add_option(ui_ddPresetsFile, entry.name(),LV_DROPDOWN_POS_LAST);
//       strncpy(fileNames[i], entry.name(), sizeof(fileNames[i]));
//       i++;
//       if (i >= 20) {
//         break;
//       }
//     }
//     entry.close();
//   }
//   dir.close();
// }




//TESLA STUFF




