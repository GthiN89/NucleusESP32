# Nucleus ESP: An Affordable Hacking Tool

Nucleus ESP is a budget-friendly yellow device for various hacking applications. It can transmit in the Sub-GHz RF range and holds additional potential, which I plan to explore further.

## Project Overview

I modified the device by desoldering the RGB diode to use it as a GPIO. Later, I disconnected the audio amplifier and photoresistor to free up additional GPIOs.  
This is my first project in C, which I began in early summer, with prior experience in PHP and some FPGA tinkering. I'm learning as I go, figuring things out by experimenting.

**Status**: Work in Progress (WIP)  
Please go easy on it; it's still evolving!  
Getting near to 0.1 nightly release!

## Features Implemented

With the help of my Software-Defined Radio (SDR), I implemented a RAW protocol for replay using a simple bit-bang method, with plans to upgrade it to more precise timing (previously, I sampled every 430 microseconds).

**TV-B-GONE**: Fully operational. Can turn TVs on and off.

**Last Stable Version**: Compiled into bin files. Occasionally, things may break in new versions, so for consistent testing, use this stable version.

### Last Commit Status
- **Updated to new LVGL**: This was challenging.  
- **Touch drivers**: Moving to the new LVGL reminded me why I use bitbang drivers - drivers are included and should compile from source.  
- **New file browser**: Unlike the previous version, this one is usable, even with bare hands.  
- **SubGhz file transmission**: When the system starts transmitting, if the `.sub` file contains more codes (e.g., brute-force files), sending completes when the counter stops increasing. You need to copy warmup `.sub` files to the root of your SD card to make it work. (This is due to the classic SPI issues; I’ll find a better solution).  
- **CYD Smart Display Library Compatibility**: The compiler may have issues with `display->sw_rotate` and `LED_RGB`. You can comment out or delete these sections; they would only cause problems anyway.  
- **RFID Module Support**: Added initialization and testing; works as expected.  
- **RF24L01 Module Support**: Successfully initialized.  
- **IR Module Support**: Can now capture and replay signals.  
- **CYD 2USB Support**: Should work, since only touch calibration is different.

### Priority List
- Fully functional/featured file explorer  
- Bug fixes  
- Finalizing RAW timing Sub-GHz functionality and releasing version 0.1 of the firmware.

### Current Functionalities
- **CC1101 RAW Replay**: Now operates similarly to the Flipper. Save files to SD card. Fully Flipper compatible.  
- **`.sub` Files Player**: Supports RAW files, tested with AM270 and AM650, should send FM too, but timing files only. Sending is stable.  
- **Tesla Charger Opener**: Operational.  
- **Large `.sub` File Support**: Handles virtually unlimited file sizes, tested with a 3MB "Came_top_brute.sub" file.  
- **TV-B-Gone**: Fully operational. Can turn TVs on and off.

### Planned Features
- Bluetooth Spam  
- Sour Apple  
- Wi-Fi Deauther  
- Bad USB  
- 2.4GHz Support  
- MouseJack Attack  
- Python Interpreter  
- Version for ESP32 S3 with 8MB RAM and 16MB ROM (JC3248W535EN)

## Timing and Transmission Quality

The timing tolerance is around 1-5 microseconds. Transmission quality from `.sub` files is as follows:

### AM270
![AM270 Transmission](https://github.com/GthiN89/NucleusESP32/blob/main/images/AM270.PNG)

### AM650
![AM650 Transmission](https://github.com/GthiN89/NucleusESP32/blob/main/images/AM650.PNG)

### The device
![Device](https://github.com/GthiN89/NucleusESP32/blob/main/images/IMG_20240924_193407_DRO.jpg)

### Partial instruction on build

[![how to Build](https://github.com/GthiN89/NucleusESP32/blob/main/images/Untitled.png)](![https://github.com/GthiN89/NucleusESP32/blob/main/video/video.mp4]

[<img src="https://github.com/GthiN89/NucleusESP32/blob/main/images/Untitled.png" width="50%">](https://github.com/GthiN89/NucleusESP32/blob/main/video/video.mp4 "How to build")

## Hardware
- Cheap Yellow Display (CYD)  
- ESP32-2432S028R  
- ESP32-2432S028Rv2  
- **CC1101 Module**  
- **RFID Module**  
- **RF24L01 Module**  
- **IR Module**

## Known Issues

- **SW Rotation Compiler Error**: If you encounter this issue, simply comment out the related code.  
- **LVGL Compiler Error**: Ensure you copy the correct LVGL configuration file to resolve this.  
- **File Browser Not Working Properly**: Make sure you are using the latest version of LVGL for full functionality.


Happy hacking!