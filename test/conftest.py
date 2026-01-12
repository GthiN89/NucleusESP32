"""
Pytest configuration for NucleusESP32 testing.

This file provides fixtures and configuration for testing the ESP32 firmware
components using mocks and simulation.
"""

import pytest
import sys
from pathlib import Path
from unittest.mock import Mock, MagicMock

# Add source directories to Python path
project_root = Path(__file__).parent.parent
src_dir = project_root / "src"
sys.path.insert(0, str(src_dir))
sys.path.insert(0, str(project_root))

# ESP32 hardware mocks
@pytest.fixture
def mock_esp32_hardware():
    """Mock ESP32 hardware interfaces."""
    hardware = Mock()

    # GPIO mocks
    hardware.gpio = Mock()
    hardware.gpio.set_direction = Mock()
    hardware.gpio.set_level = Mock()
    hardware.gpio.get_level = Mock(return_value=0)

    # SPI mocks
    hardware.spi = Mock()
    hardware.spi.begin = Mock()
    hardware.spi.endTransaction = Mock()
    hardware.spi.beginTransaction = Mock()
    hardware.spi.transfer = Mock(return_value=0)

    # I2C mocks
    hardware.i2c = Mock()
    hardware.i2c.begin = Mock()
    hardware.i2c.writeByte = Mock()
    hardware.i2c.readByte = Mock(return_value=0)

    # Timer mocks
    hardware.timer = Mock()
    hardware.timer.get_time = Mock(return_value=1000000)  # 1 second in microseconds

    return hardware

@pytest.fixture
def mock_arduino_api():
    """Mock Arduino API functions."""
    arduino = Mock()

    # Pin functions
    arduino.pinMode = Mock()
    arduino.digitalWrite = Mock()
    arduino.digitalRead = Mock(return_value=0)
    arduino.analogRead = Mock(return_value=512)

    # Serial
    arduino.Serial = Mock()
    arduino.Serial.begin = Mock()
    arduino.Serial.println = Mock()
    arduino.Serial.print = Mock()

    # SPI
    arduino.SPI = Mock()
    arduino.SPI.begin = Mock()
    arduino.SPI.end = Mock()
    arduino.SPI.beginTransaction = Mock()
    arduino.SPI.endTransaction = Mock()
    arduino.SPI.transfer = Mock(return_value=0)

    # Wire (I2C)
    arduino.Wire = Mock()
    arduino.Wire.begin = Mock()
    arduino.Wire.beginTransmission = Mock()
    arduino.Wire.endTransmission = Mock(return_value=0)
    arduino.Wire.write = Mock()
    arduino.Wire.read = Mock(return_value=0)
    arduino.Wire.available = Mock(return_value=1)

    # Time functions
    arduino.millis = Mock(return_value=1000)
    arduino.micros = Mock(return_value=1000000)
    arduino.delay = Mock()
    arduino.delayMicroseconds = Mock()

    return arduino

@pytest.fixture
def mock_nfc_hardware():
    """Mock NFC hardware (MFRC522)."""
    nfc = Mock()

    # MFRC522 specific methods
    nfc.PCD_Init = Mock()
    nfc.PCD_PerformSelfTest = Mock(return_value=True)
    nfc.PCD_GetAntennaGain = Mock(return_value=0x04)
    nfc.PCD_SetAntennaGain = Mock()

    # Card detection
    nfc.PICC_IsNewCardPresent = Mock(return_value=True)
    nfc.PICC_ReadCardSerial = Mock(return_value=True)
    nfc.PICC_HaltA = Mock()

    # Authentication
    nfc.PCD_Authenticate = Mock(return_value=True)
    nfc.PCD_StopCrypto1 = Mock()

    # Data operations
    nfc.MIFARE_Read = Mock(return_value=True)
    nfc.MIFARE_Write = Mock(return_value=True)
    nfc.MIFARE_Decrement = Mock(return_value=True)
    nfc.MIFARE_Increment = Mock(return_value=True)
    nfc.MIFARE_Restore = Mock(return_value=True)
    nfc.MIFARE_Transfer = Mock(return_value=True)

    # Status and error handling
    nfc.GetStatusCodeName = Mock(return_value="SUCCESS")

    return nfc

@pytest.fixture
def mock_cc1101():
    """Mock CC1101 RF transceiver."""
    cc1101 = Mock()

    # Initialization
    cc1101.init = Mock(return_value=True)
    cc1101.reset = Mock()

    # Configuration
    cc1101.setFrequency = Mock()
    cc1101.setModulation = Mock()
    cc1101.setDeviation = Mock()
    cc1101.setDataRate = Mock()

    # Transmission
    cc1101.sendData = Mock(return_value=True)
    cc1101.transmit = Mock(return_value=True)

    # Reception
    cc1101.receiveData = Mock(return_value=True)
    cc1101.CheckReceived = Mock(return_value=False)
    cc1101.handleSignal = Mock()
    cc1101.decode = Mock()

    # State management
    cc1101.disableReceiver = Mock()
    cc1101.disableTransmit = Mock()
    cc1101.emptyReceive = Mock()

    return cc1101

@pytest.fixture
def mock_ir_transceiver():
    """Mock IR transceiver hardware."""
    ir = Mock()

    # IRrecv methods
    ir.begin = Mock()
    ir.decode = Mock(return_value=True)
    ir.resume = Mock()

    # IRsend methods
    ir.sendNEC = Mock()
    ir.sendSony = Mock()
    ir.sendRC5 = Mock()
    ir.sendRC6 = Mock()
    ir.sendRaw = Mock()

    # Results
    ir.results = Mock()
    ir.results.decode_type = 1  # NEC
    ir.results.value = 0x00FF00FF
    ir.results.bits = 32
    ir.results.rawbuf = [100, 200, 100, 200] * 32  # Mock raw data

    return ir

@pytest.fixture
def mock_sd_card():
    """Mock SD card interface."""
    sd = Mock()

    # Initialization
    sd.initializeSD = Mock(return_value=True)
    sd.begin = Mock(return_value=True)

    # File operations
    sd.open = Mock()
    sd.exists = Mock(return_value=True)
    sd.remove = Mock(return_value=True)
    sd.mkdir = Mock(return_value=True)
    sd.rmdir = Mock(return_value=True)

    # File system info
    sd.totalBytes = Mock(return_value=16*1024*1024)  # 16MB
    sd.usedBytes = Mock(return_value=1024*1024)     # 1MB used

    return sd

@pytest.fixture
def mock_touchscreen():
    """Mock touchscreen interface."""
    touch = Mock()

    # Touch detection
    touch.getTouch = Mock(return_value=Mock(x=160, y=120, z=100))
    touch.touched = Mock(return_value=True)

    # Calibration
    touch.setCalibration = Mock()
    touch.calibrate = Mock()

    # Configuration
    touch.begin = Mock()

    return touch

@pytest.fixture
def mock_screen_manager():
    """Mock screen manager for GUI testing."""
    screen_mgr = Mock()

    # Screen operations
    screen_mgr.draw_image = Mock()
    screen_mgr.createmainMenu = Mock()
    screen_mgr.updateScreen = Mock()

    # Widget access
    screen_mgr.getTextArea = Mock(return_value="Mock Text Area")
    screen_mgr.getTextAreaRCSwitchMethod = Mock(return_value="Mock RCSwitch")
    screen_mgr.text_area_IR = Mock()
    screen_mgr.detectLabel = Mock()

    return screen_mgr

@pytest.fixture
def temp_test_dir(tmp_path):
    """Create a temporary directory for tests."""
    test_dir = tmp_path / "nucleus_test"
    test_dir.mkdir()
    return test_dir

@pytest.fixture
def sample_rf_signal():
    """Sample RF signal data for testing."""
    return {
        'frequency': 433.92,
        'modulation': 'OOK',
        'data': [1, 0, 1, 0, 1, 1, 0, 0] * 10,
        'timing': [350, 350, 700, 350] * 20
    }

@pytest.fixture
def sample_nfc_card():
    """Sample NFC card data for testing."""
    return {
        'uid': [0x12, 0x34, 0x56, 0x78],
        'type': 'MIFARE_CLASSIC_1K',
        'capacity': 1024,
        'readable_sectors': [0, 1, 2, 3],
        'default_keys': [0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]
    }

@pytest.fixture
def sample_ir_code():
    """Sample IR remote code for testing."""
    return {
        'protocol': 'NEC',
        'address': 0x00,
        'command': 0x45,
        'raw_data': [9000, 4500, 560, 560, 560, 1690] * 16,  # Simplified
        'timing': [560, 560, 560, 1690, 560, 560, 560, 1690]
    }

# Pytest configuration
def pytest_configure(config):
    """Configure pytest with custom markers."""
    config.addinivalue_line(
        "markers", "slow: marks tests as slow (deselect with '-m \"not slow\"')"
    )
    config.addinivalue_line(
        "markers", "hardware: marks tests that require hardware (deselect with '-m \"not hardware\"')"
    )
    config.addinivalue_line(
        "markers", "integration: marks tests as integration tests"
    )

def pytest_collection_modifyitems(config, items):
    """Modify test collection to add markers based on test names."""
    for item in items:
        # Mark slow tests
        if "slow" in item.nodeid or "performance" in item.nodeid:
            item.add_marker(pytest.mark.slow)

        # Mark hardware tests
        if "hardware" in item.nodeid or "_hw_" in item.nodeid:
            item.add_marker(pytest.mark.hardware)

        # Mark integration tests
        if "integration" in item.nodeid or "test/integration" in str(item.fspath):
            item.add_marker(pytest.mark.integration)

# Test data fixtures
@pytest.fixture
def test_config():
    """Test configuration data."""
    return {
        'esp32_board': 'esp32dev',
        'serial_baud': 115200,
        'spi_frequency': 1000000,
        'i2c_frequency': 100000,
        'rf_frequency': 433.92,
        'ir_pin': 15,
        'rf_tx_pin': 12,
        'rf_rx_pin': 13,
        'nfc_ss_pin': 5,
        'nfc_rst_pin': 17
    }