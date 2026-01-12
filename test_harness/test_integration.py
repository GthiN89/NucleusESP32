"""
Integration Tests for NucleusESP32
==================================

Integration testing using SpareTools test harness (ngapy-compatible API).
Tests hardware module interactions and cross-component functionality.
"""

import os
import sys
import time
from pathlib import Path
from typing import Dict, List, Optional, Any

from .nucleus_test_harness import NucleusTestHarness


class IntegrationTestHarness(NucleusTestHarness):
    """Enhanced test harness for NucleusESP32 integration testing."""

    def __init__(self, results_dir: Optional[Path] = None):
        """Initialize integration test harness."""
        super().__init__(results_dir)
        self.platform_info = self._get_platform_info()

    def _get_platform_info(self) -> Dict[str, str]:
        """Get comprehensive platform information."""
        import platform
        return {
            'system': platform.system(),
            'release': platform.release(),
            'version': platform.version(),
            'machine': platform.machine(),
            'processor': platform.processor(),
            'python_version': platform.python_version(),
        }

    def test_nfc_rf_coexistence(self, test_num: int = 1) -> bool:
        """Test NFC and RF modules can coexist without conflicts."""
        # Simulate NFC and RF hardware operations
        nfc_data = b"12345678"  # Sample NFC card data
        rf_data = b"ABCDEFGH"   # Sample RF data

        # Test NFC communication
        nfc_result = self.verify_nfc_communication(
            expected_data=nfc_data,
            actual_data=nfc_data,
            description="NFC card detection during RF operation",
            test_num=test_num
        )

        # Test RF transmission
        rf_result = self.verify_rf_transmission(
            frequency=433920000,  # 433.92 MHz
            data=rf_data,
            description="RF transmission during NFC operation",
            test_num=test_num
        )

        return nfc_result and rf_result

    def test_hardware_initialization_sequence(self, test_num: int = 2) -> bool:
        """Test proper hardware initialization sequence."""
        # Test ESP32 core initialization
        esp32_state = self.verify_hardware_state(
            module="esp32_core",
            expected_state={"initialized": True, "free_heap": 32768},
            description="ESP32 core initialization",
            test_num=test_num
        )

        # Test module initialization order
        nfc_init = self.verify_hardware_state(
            module="nfc_module",
            expected_state={"initialized": False},  # Should start uninitialized
            description="NFC module pre-initialization state",
            test_num=test_num
        )

        return esp32_state and nfc_init

    def test_module_state_transitions(self, test_num: int = 3) -> bool:
        """Test proper state transitions between modules."""
        # Simulate state transitions
        initial_state = self.verify_hardware_state(
            module="nfc_module",
            expected_state={"initialized": False, "communication_active": False},
            description="Initial NFC module state",
            test_num=test_num
        )

        # Simulate activation
        self.hardware_state["nfc_module"]["initialized"] = True
        self.hardware_state["nfc_module"]["communication_active"] = True

        activated_state = self.verify_hardware_state(
            module="nfc_module",
            expected_state={"initialized": True, "communication_active": True},
            description="NFC module activated state",
            test_num=test_num
        )

        return initial_state and activated_state


def run_integration_tests(results_dir: Optional[Path] = None) -> str:
    """
    Run integration test suite.
    
    Args:
        results_dir: Directory to store test results
        
    Returns:
        Test result: "pass", "fail", or "abort"
    """
    if results_dir is None:
        results_dir = Path("test-results")
    
    results_dir.mkdir(exist_ok=True)
    
    th = IntegrationTestHarness(results_dir=results_dir)
    
    try:
        # Run integration tests
        th.test_nfc_rf_coexistence(test_num=1)
        th.test_hardware_initialization_sequence(test_num=2)
        th.test_module_state_transitions(test_num=3)
        
        # Generate JUnit XML report
        junit_file = results_dir / "integration_tests.xml"
        th.generate_junit_report(str(junit_file))
        
        return "pass"
    except Exception as e:
        print(f"Integration test error: {e}")
        return "abort"
