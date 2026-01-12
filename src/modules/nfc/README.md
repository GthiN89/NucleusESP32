# NFC Module

A robust, SOLID-compliant NFC/RFID module for ESP32 and similar microcontrollers. This module provides a clean interface for NFC card detection and reading, with support for multiple hardware implementations.

## Features

- Interface-based design for hardware abstraction
- MFRC522 RFID reader support
- Comprehensive error handling
- Power management
- Event-based card detection
- Thread-safe operations
- Extensive test coverage

## Installation

1. Include the module in your project:
```cpp
#include "modules/nfc/nfc.h"
#include "modules/nfc/mfrc522_reader.h"
```

2. Create an instance with MFRC522 hardware:
```cpp
auto reader = std::make_unique<MFRC522Reader>(SS_PIN, RST_PIN);
NFC nfc(std::move(reader));
```

## Usage

### Basic Usage

```cpp
// Initialize the module
nfc.begin();

// Set up card detection callback
nfc.setCardDetectedCallback([](const String& uid) {
    Serial.print("Card detected: ");
    Serial.println(uid);
});

// In your main loop
void loop() {
    nfc.update();
    // ... other tasks
}
```

### Power Management

```cpp
// Put the module in low-power mode
nfc.shutdown();

// Resume operations
nfc.begin();
```

## Development

### Prerequisites

- CMake 3.10 or higher
- C++17 compatible compiler
- Docker (for development container)

### Development Container

This project includes a devcontainer configuration for VSCode. To use it:

1. Install the "Remote - Containers" extension in VSCode
2. Open the project in VSCode
3. Click "Reopen in Container" when prompted

### Building

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON

# Build
cmake --build build

# Run tests
cd build && ctest --output-on-failure
```

### Running Tests in Docker

```bash
# Build and run tests
docker-compose run test

# Run static analysis
docker-compose run analysis
```

## Testing

The module includes comprehensive tests covering:

- Basic functionality
- Error handling
- Edge cases
- State transitions
- Power management
- Stress testing

### Running Tests

```bash
# All tests
ctest --output-on-failure

# Specific test
./build/test/nfc_tests --gtest_filter=NFCTest.CardDetectionTest
```

## Code Quality

The codebase is maintained with strict quality standards:

- SOLID principles compliance
- Static analysis with clang-tidy
- Format checking with clang-format
- Additional checks with cppcheck and cpplint

### Running Quality Checks

```bash
# Run all checks
./scripts/run-analysis.sh

# Format code
clang-format -i src/modules/nfc/*.{cpp,h}
```

## Architecture

The module follows SOLID principles:

- **Single Responsibility**: Each class has a single, well-defined purpose
- **Open/Closed**: New reader implementations can be added without modifying existing code
- **Liskov Substitution**: All reader implementations are interchangeable
- **Interface Segregation**: Clean, minimal interfaces
- **Dependency Inversion**: High-level module depends on abstractions

### Class Diagram
```mermaid
%%{init: {'theme': 'forest'}}%%
classDiagram
    class INFCReader {
        <<interface>>
        +initialize()*
        +isNewCardPresent()* bool
        +readCardSerial()* bool
        +getUID()* UID
        +halt()*
        +powerDown()*
    }
    
    class NFC {
        -reader: unique_ptr~INFCReader~
        -callback: CardDetectedCallback
        -state: NFCState
        +NFC(readerImpl: unique_ptr~INFCReader~)
        +begin()
        +update()
        +shutdown()
        +getState() NFCState
        +setCardDetectedCallback(callback)
        -formatUID(uidBytes, uidSize) String
    }
    
    class MFRC522Reader {
        -mfrc: MFRC522
        -initialized: bool
        +MFRC522Reader(ssPin, rstPin)
        +initialize()
        +isNewCardPresent() bool
        +readCardSerial() bool
        +getUID() UID
        +halt()
        +powerDown()
    }
    
    class UID {
        +data: array~byte,10~
        +size: byte
    }
    
    class NFCState {
        <<enumeration>>
        Idle
        Reading
    }
    
    INFCReader <|.. MFRC522Reader : implements
    NFC o-- INFCReader : owns
    NFC o-- NFCState : uses
    MFRC522Reader ..> UID : creates
    INFCReader ..> UID : uses
```

### State Diagram
```mermaid
%%{init: {'theme': 'forest'}}%%
stateDiagram-v2
    [*] --> Idle: begin()
    Idle --> Reading: isNewCardPresent()
    Reading --> Idle: halt()
    Reading --> Idle: error
    Idle --> [*]: shutdown()
    
    state Reading {
        [*] --> DetectingCard
        DetectingCard --> ReadingSerial: detected
        ReadingSerial --> ProcessingUID: success
        ProcessingUID --> InvokingCallback: valid UID
        InvokingCallback --> [*]: complete
    }
```

### Sequence Diagram: Card Detection
```mermaid
sequenceDiagram
    participant App
    participant NFC
    participant MFRC522Reader
    participant Card
    participant Callback
    
    App->>NFC: update()
    activate NFC
    NFC->>MFRC522Reader: isNewCardPresent()
    activate MFRC522Reader
    MFRC522Reader->>Card: RF field interaction
    Card-->>MFRC522Reader: presence detected
    MFRC522Reader-->>NFC: true
    NFC->>MFRC522Reader: readCardSerial()
    MFRC522Reader->>Card: request UID
    Card-->>MFRC522Reader: send UID
    MFRC522Reader-->>NFC: true
    deactivate MFRC522Reader
    NFC->>NFC: formatUID()
    NFC->>Callback: invoke(uid)
    activate Callback
    Callback-->>NFC: return
    deactivate Callback
    NFC->>MFRC522Reader: halt()
    NFC-->>App: return
    deactivate NFC
```

### Component Integration
```mermaid
%%{init: {'theme': 'forest'}}%%
graph TD
    subgraph NFC Module
        A[NFC Class] --> B[INFCReader]
        B --> C[MFRC522Reader]
        A --> D[State Management]
        A --> E[Event Handling]
    end
    
    subgraph Hardware Layer
        C --> F[SPI Bus]
        F --> G[MFRC522 Chip]
    end
    
    subgraph Application Layer
        H[User Code] --> A
        A --> I[Callbacks]
    end
    
    style NFC Module fill:#f9f,stroke:#333,stroke-width:2px
    style Hardware Layer fill:#bbf,stroke:#333,stroke-width:2px
    style Application Layer fill:#bfb,stroke:#333,stroke-width:2px
```

### Protocol State Machine
```mermaid
%%{init: {'theme': 'forest'}}%%
stateDiagram-v2
    [*] --> Ready: initialize()
    Ready --> WaitingForCard: field active
    WaitingForCard --> CardDetected: card present
    CardDetected --> Authenticating: valid card
    Authenticating --> Reading: auth success
    Reading --> Ready: halt()
    
    state Authenticating {
        [*] --> RequestA
        RequestA --> SelectA: success
        SelectA --> AuthKey: card selected
        AuthKey --> [*]: authenticated
    }
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Run tests and quality checks
5. Submit a pull request

## License

MIT License - see LICENSE file for details
