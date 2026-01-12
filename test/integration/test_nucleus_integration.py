"""
NucleusESP32 Integration Tests
==============================

Integration tests using the NucleusTestHarness for hardware simulation
and cross-component interaction testing.
"""

import pytest
from pathlib import Path
from test_harness import NucleusTestHarness


class TestNucleusIntegration:
    """Integration tests for NucleusESP32 hardware modules."""

    @pytest.fixture
    def harness(self, tmp_path):
        """Create test harness for each test."""
        results_dir = tmp_path / "test_results"
        return NucleusTestHarness(results_dir=results_dir, hardware_simulation=True)

    def test_nfc_rf_coexistence(self, harness):
        """Test that NFC and RF modules can operate simultaneously."""
        # Simulate NFC card detection
        card_data = b"12345678"
        nfc_result = harness.verify_nfc_communication(
            expected_data=card_data,
            actual_data=card_data,
            description="NFC card detection"
        )

        # Simulate RF transmission on different frequency
        rf_data = b"RF_TEST_DATA"
        rf_result = harness.verify_rf_transmission(
            frequency=433920000,
            data=rf_data,
            description="RF transmission during NFC operation"
        )

        assert nfc_result is True
        assert rf_result is True

        # Verify both modules are in expected states
        nfc_state = harness.verify_hardware_state(
            "nfc_module",
            {"communication_active": True}
        )
        rf_state = harness.verify_hardware_state(
            "rf_module",
            {"transmission_active": True}
        )

        assert nfc_state is True
        assert rf_state is True

    def test_hardware_initialization_sequence(self, harness):
        """Test proper hardware initialization order."""
        # ESP32 core should be initialized first
        esp32_state = harness.verify_hardware_state(
            "esp32_core",
            {"initialized": True}
        )
        assert esp32_state is True

        # Other modules should start uninitialized
        modules_to_check = ["nfc_module", "rf_module", "ir_module", "sd_module"]
        for module in modules_to_check:
            state = harness.verify_hardware_state(
                module,
                {"initialized": False}
            )
            assert state is True, f"{module} should start uninitialized"

    def test_performance_benchmarking(self, harness):
        """Test performance benchmarking capabilities."""
        def sample_operation():
            """Simulate a hardware operation."""
            import time
            time.sleep(0.01)  # 10ms operation
            return "operation_result"

        # Benchmark the operation
        result = harness.benchmark_operation(
            "sample_hardware_operation",
            sample_operation
        )

        assert result["success"] is True
        assert result["operation"] == "sample_hardware_operation"
        assert result["duration"] > 0
        assert result["result"] == "operation_result"

    def test_comprehensive_report_generation(self, harness, tmp_path):
        """Test comprehensive report generation."""
        # Run some test operations
        harness.verify_nfc_communication(b"test", b"test", "Test NFC")
        harness.verify_rf_transmission(433920000, b"rf_data", "Test RF")

        # Generate comprehensive report
        report_file = tmp_path / "comprehensive_report.json"
        success = harness.generate_comprehensive_report(report_file)

        assert success is True
        assert report_file.exists()

        # Verify report contents
        import json
        with open(report_file) as f:
            report_data = json.load(f)

        assert "test_summary" in report_data
        assert "hardware_state" in report_data
        assert "performance_metrics" in report_data
        assert "test_results" in report_data
        assert len(report_data["test_results"]) >= 2

    def test_hardware_failure_simulation(self, harness):
        """Test hardware failure simulation."""
        # Simulate communication failure
        failure_simulated = harness.simulate_hardware_failure(
            "nfc_module",
            "communication_error"
        )
        assert failure_simulated is True

        # Verify failure state
        failure_state = harness.verify_hardware_state(
            "nfc_module",
            {"communication_active": False, "error": "Communication timeout"}
        )
        assert failure_state is True

    def test_hardware_state_reset(self, harness):
        """Test hardware simulation state reset."""
        # Modify some state
        harness.hardware_state["nfc_module"]["communication_active"] = True
        harness.hardware_state["rf_module"]["transmission_active"] = True

        # Reset simulation
        reset_success = harness.reset_hardware_simulation()
        assert reset_success is True

        # Verify reset state
        nfc_reset = harness.verify_hardware_state(
            "nfc_module",
            {"communication_active": False}
        )
        rf_reset = harness.verify_hardware_state(
            "rf_module",
            {"transmission_active": False}
        )

        assert nfc_reset is True
        assert rf_reset is True