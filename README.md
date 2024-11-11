# Nucleus ESP: An Affordable Hacking Tool

Nucleus ESP is a budget-friendly yellow device for various hacking applications. It can transmit in the Sub-GHz RF range and holds additional potential, which I plan to explore further.

## Project Overview

I modified the device by desoldering the RGB diode to use it as a GPIO.  
This is my first project in C, which I began in early summer, with prior experience in PHP and some FPGA tinkering. I'm learning as I go, figuring things out by experimenting.

**Status**: Work in Progress (WIP)  
Please go easy on it; it's still evolving!
Getting near to 0.1 nightly relase!

## Features Implemented

With the help of my Software-Defined Radio (SDR), I implemented a RAW protocol for replay using a simple bit-bang method, with plans to upgrade it to more precise timing (previously, I sampled every 430 microseconds).

Currently, I’m planning to reintegrate RCswitch, as it hasn't been added to the new interface yet.

**Last Stable Version**: Compiled into bin files. Occasionally, things may break in new versions, so for consistent testing, use this stable version.

### Last Commit Status
- **Updated to new LVGL**: This was challenging.  
- **Touch drivers**: Moving to the new LVGL reminded me why I use bitbang drivers - drivers are included, and should compile from source.  
- **New file browser**: Unlike the previous version, this one is usable, even with bare hands.  
- **SubGhz file transmission**: When the system starts transmitting, if the `.sub` file contains more codes (e.g., brute-force files), sending completes when the counter stops increasing. You need to copy warmup `.sub` files to the root of your SD card to make it work. (This is due to the classic SPI issues; I’ll find a better solution).
- **CYD Smart Display Library Compatibility**: The compiler may have issues with `display->sw_rotate` and `LED_RGB`. You can comment out or delete these sections; they would only cause problems anyway.

### Priority List
- Fully functional/featured file explorer  
- Bugs
- Basically, getting everything around RAW timing Sub-GHz functionality done and releasing version 0.1 of the firmware.

### Current Functionalities
- **CC1101 RAW Replay**: Now operates similarly to the Flipper. Save files to sd card. Fully Flipper compatible.
- **`.sub` Files Player**: Supports RAW files, tested with AM270 and AM650. Sending prettz much works now.  
- **Tesla Charger Opener**: Currently deactivated; will return later, not a priority.  
- **Large `.sub` File Support**: Handles virtually unlimited file sizes, tested with a 3MB "Came_top_brute.sub" file.   
- **CYD micro USB "bad SPI" Rv2**: As you know, you never know which CYD version you’ll get.

### Planned Features
- **CYD 2USB**: This will be added soon, as I ordered some to make Nucleus for friends, and they sent me those.  
- Bluetooth Spam  
- Sour Apple  
- Wi-Fi Deauther  
- Bad USB  
- 2.4GHz Support  
- MouseJack Attack  
- Python interpreter  
- Version for ESP32 S3 with 8MB RAM and 16MB ROM (JC3248W535EN)  

## Timing and Transmission Quality

The timing tolerance is around 1-5 microseconds. Transmission quality from `.sub` files is as follows:

### AM270
![AM270 Transmission](https://github.com/GthiN89/NucleusESP32/blob/main/images/AM270.PNG)

### AM650
![AM650 Transmission](https://github.com/GthiN89/NucleusESP32/blob/main/images/AM650.PNG)

### Capture Times
![Capture Times](https://github.com/GthiN89/NucleusESP32/blob/main/images/capture%20times.bmp)

## Hardware
- Cheap Yellow Display (CYD)  
- ESP32-2432S028R  
- ESP32-2432S028Rv2  

Happy hacking!
