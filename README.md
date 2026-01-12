# NucleusESP32: Multi-Tool ESP32 Firmware

[![CI/CD Pipeline](https://github.com/sparesparrow/NucleusESP32/actions/workflows/ci.yml/badge.svg)](https://github.com/sparesparrow/NucleusESP32/actions/workflows/ci.yml)
[![Code Coverage](https://codecov.io/gh/sparesparrow/NucleusESP32/branch/main/graph/badge.svg)](https://codecov.io/gh/sparesparrow/NucleusESP32)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-6.1+-blue.svg)](https://platformio.org/)

NucleusESP32 is a comprehensive multi-tool firmware for ESP32-based devices, providing Sub-GHz RF communication, NFC/RFID capabilities, IR remote control, and more. Built with modern development practices using SpareTools ecosystem.

## 🔧 SpareTools Integration

NucleusESP32 is a **SpareTools ESP32 Consumer** - a reference implementation demonstrating best practices for ESP32 firmware development within the SpareTools ecosystem.

### What is SpareTools?

[SpareTools](https://github.com/sparesparrow/sparetools) is a comprehensive development ecosystem providing:

- **Hermetic Development Environments**: Consistent Python 3.12.7 environments across all projects
- **Unified Build System**: CMake + Conan + PlatformIO integration
- **Reusable CI/CD Workflows**: GitHub Actions workflows for quality, testing, building, and security
- **Comprehensive Testing**: Hardware simulation, unit tests, and integration testing
- **Package Management**: Conan-based dependency management with shared tooling via Cloudsmith

### Integration Benefits

- **Consistent Tooling**: Same development experience across all SpareTools projects
- **Automated Setup**: One-command bootstrap creates complete development environment
- **Shared Components**: Reusable test harnesses, build tools, and CI/CD workflows via packages
- **Quality Assurance**: Automated code quality, security scanning, and testing
- **Ecosystem Support**: Access to shared ESP32 tooling and best practices through package management

### Project Structure

```
NucleusESP32/
├── src/                   # ESP32 firmware source code
├── include/               # C/C++ header files
├── test/                  # Unit and integration tests
├── test_harness/          # Hardware simulation components
├── scripts/               # Development and build scripts
├── .github/workflows/     # CI/CD workflows
├── platformio.ini         # PlatformIO configuration
├── CMakeLists.txt         # CMake build system
├── conanfile.py           # Conan package definition (consumes SpareTools packages)
└── pyproject.toml         # Python project configuration
```

### Package-Based Architecture

NucleusESP32 consumes SpareTools components through Conan packages hosted on Cloudsmith:

- `sparetools-base/2.0.0` - Foundation utilities and configuration
- `sparetools-cpython/3.12.7` - Hermetic Python environment
- `sparetools-bootstrap/2.0.0` - ESP32 bootstrap and setup tools
- `sparetools-test-harness/2.0.0` - Hardware simulation and testing framework
- `sparetools-shared-dev-tools/2.0.0` - Development tools and CLI

## 🎯 Features

### Core Functionality
- **Sub-GHz RF Communication**: CC1101-based transceiver with RAW protocol support
- **NFC/RFID**: MFRC522 reader with comprehensive card reading capabilities
- **IR Remote Control**: Full TV-B-GONE implementation with custom IR codes
- **WiFi & Bluetooth**: Network connectivity and wireless features
- **SD Card Storage**: File system support for data persistence

### Advanced Features
- **Signal Processing**: Advanced filtering and reconstruction algorithms
- **Multiple Encoders/Decoders**: CAME, NICE, Ansonic, Holtek, Hormann protocols
- **Brute Force Attacks**: Systematic protocol testing capabilities
- **Frequency Analysis**: Signal strength and frequency detection
- **Cross-Platform Testing**: Host simulation with ESP32 target validation

### Development Features
- **Modern Build System**: CMake + Conan + PlatformIO integration
- **Comprehensive Testing**: Unit tests, integration tests, and hardware simulation
- **CI/CD Pipeline**: Automated building, testing, and deployment
- **Code Quality**: Static analysis, formatting, and security scanning
- **Documentation**: Auto-generated API docs and development guides

## 🚀 Quick Start

### Prerequisites
- Python 3.8+ (automatically managed by SpareTools)
- Git
- Internet connection

### Development Environment Setup

1. **Clone the repository**
   ```bash
   git clone https://github.com/sparesparrow/NucleusESP32.git
   cd NucleusESP32
   ```

2. **Bootstrap development environment**
   ```bash
   python scripts/bootstrap.py
   ```

   This will:
   - Install SpareTools and dependencies
   - Set up Python virtual environment
   - Configure Conan package management
   - Install PlatformIO and ESP32 toolchains
   - Set up pre-commit hooks

3. **Build and flash firmware**
   ```bash
   # Build for default board (ESP32-2432S028Rv3)
   pio run

   # Or build for specific board
   pio run -e esp32-8048S070C

   # Flash to device
   pio run -t upload
   ```

4. **Run tests**
   ```bash
   # Unit tests
   pytest test/

   # Integration tests
   pytest test/integration/

   # Hardware simulation tests
   python test/micropython.py
   ```

## 🛠️ Development Workflow

### Project Structure
```
NucleusESP32/
├── src/                    # Source code
│   ├── main.cpp           # Main firmware entry point
│   ├── modules/           # Hardware modules
│   │   ├── nfc/          # NFC/RFID functionality
│   │   ├── RF/           # Sub-GHz RF communication
│   │   ├── IR/           # Infrared remote control
│   │   └── ETC/          # Utilities and storage
│   └── GUI/               # User interface
├── test/                  # Test suite
│   ├── integration/       # Hardware integration tests
│   └── conftest.py       # Test configuration
├── boards/                # Board-specific configurations
├── scripts/              # Development scripts
├── .github/workflows/    # CI/CD pipelines
├── conanfile.py          # Conan package definition
├── platformio.ini        # PlatformIO configuration
└── CMakeLists.txt        # CMake build system
```

### Supported Boards

The firmware supports numerous ESP32-based boards:

| Board | Display | Touch | Status |
|-------|---------|-------|--------|
| ESP32-2432S028Rv3 | 2.8" ILI9341 | XPT2046 | ✅ Default |
| ESP32-8048S070C | 7.0" IPS | XPT2046 | ✅ Stable |
| ESP32-4827S043C | 4.8" IPS | XPT2046 | ✅ Stable |
| ESP32-1732S019C | 1.73" Round | None | ✅ Minimal |
| ESP32-S3-TouchLCD7 | 1.14" ST7789 | Built-in | 🚧 Experimental |

### Building for Different Boards

```bash
# List all available environments
pio run --list-targets

# Build for specific board
pio run -e esp32-8048S070C

# Clean and rebuild
pio run -t clean && pio run
```

### Testing

```bash
# Run all tests
pytest

# Run with coverage
pytest --cov=nucleus --cov-report=html

# Run specific test categories
pytest -m "unit"           # Unit tests only
pytest -m "integration"    # Integration tests only
pytest -m "not slow"       # Skip slow tests

# Hardware simulation
python test/micropython.py
```

### Code Quality

```bash
# Format code
black src/ test/
isort src/ test/

# Lint code
flake8 src/ test/
mypy src/

# Static analysis
python scripts/run-analysis.sh

# Pre-commit checks
pre-commit run --all-files
```

## 📋 Current Status

### ✅ Implemented Features
- **Sub-GHz RF**: Full CC1101 support with RAW protocol replay
- **NFC/RFID**: MFRC522 reader with card detection and data dumping
- **IR Control**: TV-B-GONE with dual IR LEDs
- **GUI**: LVGL-based interface with touch support
- **File System**: LittleFS support for data persistence
- **Multiple Protocols**: CAME, NICE, Ansonic, Holtek encoders/decoders

### 🚧 In Development
- **ESP32-S3 Support**: Enhanced hardware capabilities
- **Bluetooth Features**: BLE spam and Sour Apple attacks
- **WiFi Deauther**: Network testing tools
- **Python Interpreter**: Embedded scripting support

### 🐛 Known Issues
- Frequency analyzer occasionally non-functional
- Large file transmission may be slow to load
- Some board configurations need refinement

## 🔧 Hardware Requirements

### Core Components
- **ESP32 Module**: Any supported ESP32 board
- **CC1101 Module**: Sub-GHz transceiver
- **MFRC522**: NFC/RFID reader (optional)
- **IR LEDs**: For remote control (optional)
- **SD Card**: For file storage (optional)

### Pin Configuration
Default pin assignments (configurable per board):
```
CC1101_CS    = 27
CC1101_SCK   = 22
CC1101_MISO  = 35
CC1101_MOSI  = 21
CC1101_GDO0  = 33
CC1101_GDO2  = 32

MFRC522_SS   = 5
MFRC522_RST  = 17

IR_TX        = 12
IR_RX        = 13

TOUCH_IRQ    = 36
TOUCH_MOSI   = 32
TOUCH_MISO   = 39
TOUCH_CLK    = 25
TOUCH_CS     = 33
```

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/amazing-feature`
3. Bootstrap development environment: `python scripts/bootstrap.py`
4. Make your changes and ensure tests pass
5. Commit your changes: `git commit -m 'Add amazing feature'`
6. Push to the branch: `git push origin feature/amazing-feature`
7. Open a Pull Request

### Development Guidelines
- Follow SOLID principles and modern C++ practices
- Write comprehensive tests for new features
- Update documentation for API changes
- Ensure code passes all quality checks
- Test on multiple board configurations

## 📚 Documentation

- [API Documentation](docs/api/) - Auto-generated from source code
- [Hardware Guide](docs/hardware/) - Board configurations and pinouts
- [Protocol Reference](docs/protocols/) - Supported RF protocols
- [Testing Guide](docs/testing/) - Development and testing procedures

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Original NucleusESP32 project by [GthiN89](https://github.com/GthiN89/NucleusESP32)
- SpareTools ecosystem for modern development workflow
- ESP32 and Arduino communities for hardware support
- Contributors and testers

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/sparesparrow/NucleusESP32/issues)
- **Discussions**: [GitHub Discussions](https://github.com/sparesparrow/NucleusESP32/discussions)
- **Documentation**: [Project Wiki](https://github.com/sparesparrow/NucleusESP32/wiki)

---

**Happy Hacking! 🔧⚡**
