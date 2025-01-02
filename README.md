# Nucleus ESP: An Affordable Hacking Tool

Nucleus ESP is a budget-friendly yellow device for various hacking applications. It can transmit in the Sub-GHz RF range and holds additional potential, which I plan to explore further.

## Project Overview

I modified the device by desoldering the RGB diode to use it as a GPIO. Later, I disconnected the audio amplifier and photoresistor to free up additional GPIOs.  
This is my first project in C, which I began in early summer, with prior experience in PHP and some FPGA tinkering. I'm learning as I go, figuring things out by experimenting.

**Status**: Work in Progress (WIP)  
Please go easy on it; it's still evolving!  
Currently at **version Neon Green**.

## Features Implemented

With the help of my Software-Defined Radio (SDR), I implemented a RAW protocol for replay using a simple bit-bang method, with plans to upgrade it to more precise timing (previously, I sampled every 430 microseconds).

**TV-B-GONE**: Fully operational. Can turn TVs on and off.

**Last Stable Version**: Compiled into bin files. Occasionally, things may break in new versions, so for consistent testing, use this stable version.

### Latest Updates
- **Improved Sub-GHz Signal Reception**: Enhanced performance.
- **GUI Performance Boost**: Faster and more responsive.
- **Dark Mode for GUI**: Aesthetic improvement as part of "Neon Green" version.
- **CYD 2USB Compatibility**: Binary is included in the repo.
- **Focus on CC1101 Module for CYD Version**: Future versions will target more powerful ESP32 S3 hardware.

### Known Issues
- **Large File Transmission**: Bigger files take longer to load, but they transmit just fine.
- **Frequency Analyzer Non-Functional**: Currently broken. Fix planned.

### Priority List
- Repair frequency analyzer functionality.
- Finalize CYD version with IR, Sub-GHz, Wi-Fi, and Bluetooth features.
- Transition to ESP32 S3 hardware for enhanced capabilities.

## Current Functionalities
- **CC1101 RAW Replay**: Now operates similarly to the Flipper. Save files to SD card. Fully Flipper compatible.  
- **`.sub` Files Player**: Supports RAW files, tested with AM270 and AM650, should send FM too, but timing files only. Sending is stable.  
- **Tesla Charger Opener**: Operational.  
- **TV-B-Gone**: Fully operational.

## Planned Features
- Bluetooth Spam  
- Sour Apple  
- Wi-Fi Deauther  
- Python Interpreter  


## Timing and Transmission Quality

The timing tolerance is around 1-5 microseconds. Transmission quality from `.sub` files is as follows:

### AM270
![AM270 Transmission](https://github.com/GthiN89/NucleusESP32/blob/main/images/AM270.PNG)

### AM650
![AM650 Transmission](https://github.com/GthiN89/NucleusESP32/blob/main/images/AM650.PNG)

### The device
![Device](https://github.com/GthiN89/NucleusESP32/blob/main/images/IMG_20240924_193407_DRO.jpg)

### Partial instruction on build

[![Watch the video](https://github.com/GthiN89/NucleusESP32/raw/refs/heads/main/images/Untitled.png)](https://github.com/GthiN89/NucleusESP32/raw/refs/heads/main/video/video.mp4)

## Hardware
- Cheap Yellow Display (CYD)  
- ESP32-2432S028R  
- ESP32-2432S028Rv2  
- **CC1101 Module**  
- **IR Module**

## Repository Stats
- **Commits**: ![Commits](https://img.shields.io/github/commit-activity/m/GthiN89/NucleusESP32)
- **Issues**: ![Issues](https://img.shields.io/github/issues/GthiN89/NucleusESP32)

Happy hacking!
