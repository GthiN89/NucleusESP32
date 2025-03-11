# Nucleus ESP: An Affordable Hacking Tool

Nucleus ESP is a budget-friendly yellow device for various hacking applications. It can transmit in the Sub-GHz RF range and holds additional potential, which I plan to explore further.

### The device
![Device](https://github.com/GthiN89/NucleusESP32/blob/main/images/brute.jpg)
![Device](https://github.com/GthiN89/NucleusESP32/blob/main/images/decoder.jpg)
![Device](https://github.com/GthiN89/NucleusESP32/blob/main/images/encoder.jpg)

## Project Overview

I modified the device by desoldering the RGB diode to use it as a GPIO. Later, I disconnected the audio amplifier and photoresistor to free up additional GPIOs.

This is my first project in C, which I began in early summer, with prior experience in PHP and some FPGA tinkering. I'm learning as I go, figuring things out by experimenting.

I learned a lot. This project implements decoding of the filtered and reconstructed subGhz radio signal, acting as a codegrabber, and its being tested on various real HW im able to get cheap. But it must be dirt. Dirty cheap...
*clones - low secure aftermarket car and gate modules.

After lets say finalizing SubGhz part, i started working on NFC/RFID part witn PN532 connected on shared SPI with CC1101

**Status**: Work in Progress (WIP)  
Please go easy on it; it's still evolving!  
Some push work, some not, in some all functiuons work, and in other one is push much far, but some not work at all.... After finishing ancoders/decders, i will create "stable" relase, but so far jus development without protocol....

## Features Implemented

With the help of my Software-Defined Radio (SDR), I implemented a RAW protocol for replay using a simple bit-bang pooler method, and later, started to use interrupt as i should to more precise timing (previously, I sampled every 430 microseconds). Then i read read watch some videos, visit some death sites on internet archive, and become able to reallz work with the signal....

**TV-B-GONE**: Fully operational. Can turn TVs on and off, on HW side use 2 IR diods, one normal, one 1W  160 degrees, both driver at the edge, bezond what is considered save

**Last Stable Version**: Compiled into bin files. Occasionally, things may break in new versions, so for consistent testing, use this stable version.

**Encoders and Decoders**: Implemented CAME, NICE, Ansonic and Holtec 12bit encoders and decoders + Horman 44bit decoder

**BruteForce Subghz attack**: Implemented CAME, NICE, Ansonic and Holtec 12bit.

### Latest Updates
- **Implemented Sub-GHz Signal filtering and reconstruction**: Can process filter out noise, detect wrong logic state and switch them, calculate proable "original" pulse lenght, and provide reconstructed signal with tolerance of 1-5 microseconds
- **GUI Performance Boost and variables//buffers fusing**: Faster and more responsive, lowering memory comsuption over time.
- **Dark Mode for GUI**: Aesthetic improvement as part of "Neon Green" version, bc neon green and black are it.
- **CYD 2USB**: Despite originallz starting on classic CYD, now for 2USB, bc those are on market now.
- **Focus on CC1101 Module for CYD Version**: Future versions will target more powerful ESP32 S3 hardware.
- **Integration of Decoders and Encoders for Ansonic, Came, Nice, Hormann, and SMC5326**.
- **RC Switch as second Decoder(User-Selectable)**.
- **Brute Force Attack using decoders** 

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
- **TV-B-Gone**: Operational.

## Planned Features
- Bluetooth Spam  
- Sour Apple  
- Wi-Fi Deauther  
- Python Interpreter  

## Timing and Transmission Quality

The timing tolerance is around 1-5 microseconds both ways.


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
