#!/usr/bin/env python3
import json
import os
from pathlib import Path

def generate_compile_commands():
    # Get PlatformIO's build directory
    home_dir = os.path.expanduser('~')
    platformio_dir = os.path.join(home_dir, '.platformio')
    
    # Arduino framework paths for Windows
    arduino_paths = [
        os.path.join(platformio_dir, 'packages', 'framework-arduinoespressif32', 'cores', 'esp32'),
        os.path.join(platformio_dir, 'packages', 'framework-arduinoespressif32', 'variants', 'esp32'),
        os.path.join(platformio_dir, 'packages', 'framework-arduinoespressif32', 'libraries'),
        os.path.join(platformio_dir, 'packages', 'toolchain-xtensa-esp32', 'xtensa-esp32-elf', 'include', 'c++', '8.4.0'),
        os.path.join(platformio_dir, 'packages', 'toolchain-xtensa-esp32', 'lib', 'gcc', 'xtensa-esp32-elf', '8.4.0', 'include'),
        os.path.join(platformio_dir, 'packages', 'framework-arduinoespressif32', 'tools', 'sdk', 'esp32', 'include'),
        os.path.join(platformio_dir, 'packages', 'framework-arduinoespressif32', 'tools', 'sdk', 'esp32', 'include', 'esp32'),
        os.path.join(platformio_dir, 'packages', 'framework-arduinoespressif32', 'tools', 'sdk', 'esp32', 'include', 'driver'),
        os.path.join(platformio_dir, 'packages', 'framework-arduinoespressif32', 'tools', 'sdk', 'esp32', 'include', 'freertos'),
        'src',
        'include'
    ]
    
    # Create include flags
    include_flags = ' '.join(f'-I"{p}"' for p in arduino_paths)
    
    # Base compiler flags
    base_flags = f'-DESP32 -DARDUINO=10819 -DARDUINO_ESP32_DEV -DARDUINO_ARCH_ESP32 -DARDUINO_BOARD="ESP32_DEV" -std=gnu++2a {include_flags}'
    
    compile_commands = []
    
    # Add all cpp files in src directory
    for cpp_file in Path('src').rglob('*.cpp'):
        command = {
            'directory': str(Path.cwd()),
            'command': f'clang++ {base_flags} -c "{cpp_file}"',
            'file': str(cpp_file)
        }
        compile_commands.append(command)
    
    # Write compile_commands.json
    with open('compile_commands.json', 'w') as f:
        json.dump(compile_commands, f, indent=2)

if __name__ == '__main__':
    generate_compile_commands() 