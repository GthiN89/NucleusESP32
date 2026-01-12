#!/usr/bin/env python3
"""
NucleusESP32 Test Harness Demo
==============================

Demonstration of the NucleusTestHarness capabilities for hardware simulation
and testing following SpareTools/ngapy patterns.

Run this script to see the test harness in action:
    python test_harness/demo.py
"""

import time
from pathlib import Path
from nucleus_test_harness import NucleusTestHarness


def demo_basic_verification():
    """Demonstrate basic verification capabilities."""
    print("🔍 Basic Verification Demo")
    print("-" * 40)

    harness = NucleusTestHarness()

    # Test NFC communication
    card_data = b"12345678"
    nfc_result = harness.verify_nfc_communication(
        expected_data=card_data,
        actual_data=card_data,
        description="NFC card detection"
    )
    print(f"NFC test: {'PASSED' if nfc_result else 'FAILED'}")

    # Test RF transmission
    rf_result = harness.verify_rf_transmission(
        frequency=433920000,
        data=b"HELLO",
        description="RF signal transmission"
    )
    print(f"RF test: {'PASSED' if rf_result else 'FAILED'}")

    # Test IR transmission
    ir_result = harness.verify_ir_code(
        code="0x20DF10EF",
        protocol="NEC",
        description="IR power button code"
    )
    print(f"IR test: {'PASSED' if ir_result else 'FAILED'}")

    print()


def demo_hardware_state_tracking():
    """Demonstrate hardware state tracking."""
    print("🔧 Hardware State Tracking Demo")
    print("-" * 40)

    harness = NucleusTestHarness()

    print("Initial hardware states:")
    for module, state in harness.hardware_state.items():
        print(f"  {module}: initialized={state.get('initialized', False)}")

    # Simulate operations
    harness.verify_nfc_communication(b"card", b"card", "Card detection")
    harness.verify_rf_transmission(433920000, b"data", "RF transmission")

    print("\nAfter operations:")
    for module, state in harness.hardware_state.items():
        status = "active" if state.get('communication_active') or state.get('transmission_active') else "idle"
        print(f"  {module}: {status}")

    print()


def demo_performance_benchmarking():
    """Demonstrate performance benchmarking."""
    print("⚡ Performance Benchmarking Demo")
    print("-" * 40)

    harness = NucleusTestHarness()

    def slow_hardware_operation():
        """Simulate a slow hardware operation."""
        time.sleep(0.1)  # 100ms delay
        return "operation_complete"

    def fast_hardware_operation():
        """Simulate a fast hardware operation."""
        time.sleep(0.01)  # 10ms delay
        return "quick_result"

    # Benchmark operations
    slow_result = harness.benchmark_operation(
        "slow_hardware_operation",
        slow_hardware_operation
    )

    fast_result = harness.benchmark_operation(
        "fast_hardware_operation",
        fast_hardware_operation
    )

    print(f"Slow operation: {slow_result['duration']:.3f}s")
    print(f"Fast operation: {fast_result['duration']:.3f}s")
    print()


def demo_failure_simulation():
    """Demonstrate hardware failure simulation."""
    print("💥 Hardware Failure Simulation Demo")
    print("-" * 40)

    harness = NucleusTestHarness()

    # Normal operation
    normal_result = harness.verify_nfc_communication(b"test", b"test", "Normal operation")
    print(f"Normal operation: {'PASSED' if normal_result else 'FAILED'}")

    # Simulate hardware failure
    harness.simulate_hardware_failure("nfc_module", "communication_error")

    # Test after failure
    failure_result = harness.verify_hardware_state(
        "nfc_module",
        {"communication_active": False, "error": "Communication timeout"}
    )
    print(f"Failure detection: {'PASSED' if failure_result else 'FAILED'}")

    # Reset and test recovery
    harness.reset_hardware_simulation()
    reset_result = harness.verify_hardware_state(
        "nfc_module",
        {"communication_active": False, "error": None}
    )
    print(f"Reset verification: {'PASSED' if reset_result else 'FAILED'}")

    print()


def demo_comprehensive_reporting():
    """Demonstrate comprehensive report generation."""
    print("📊 Comprehensive Reporting Demo")
    print("-" * 40)

    results_dir = Path("demo-results")
    harness = NucleusTestHarness(results_dir=results_dir)

    # Run various tests
    harness.verify_nfc_communication(b"card1", b"card1", "NFC test 1")
    harness.verify_rf_transmission(433920000, b"rf_data", "RF test")
    harness.verify_ir_code("0x12345678", "NEC", "IR test")

    # Benchmark an operation
    harness.benchmark_operation("demo_operation", lambda: time.sleep(0.05))

    # Generate comprehensive report
    report_file = results_dir / "demo_comprehensive_report.json"
    success = harness.generate_comprehensive_report(report_file)

    if success:
        print(f"✅ Report generated: {report_file}")
        print(f"📁 Results directory: {results_dir}")

        # Show summary
        print("\n📈 Test Summary:")
        print(f"   Total tests: {len(harness.test_results)}")
        passed = len([r for r in harness.test_results if r.get('passed')])
        print(f"   Passed: {passed}")
        print(f"   Success rate: {passed/len(harness.test_results)*100:.1f}%")
        print(f"   Total duration: {harness.test_start_time:.3f}s")
    else:
        print("❌ Report generation failed")

    print()


def main():
    """Run all demonstrations."""
    print("🚀 NucleusESP32 Test Harness Demo")
    print("=" * 50)
    print("Demonstrating SpareTools/ngapy-compatible testing capabilities")
    print()

    demo_basic_verification()
    demo_hardware_state_tracking()
    demo_performance_benchmarking()
    demo_failure_simulation()
    demo_comprehensive_reporting()

    print("🎉 Demo completed!")
    print("\nTo use in your tests:")
    print("  from test_harness import NucleusTestHarness")
    print("  harness = NucleusTestHarness()")
    print("  result = harness.verify_nfc_communication(expected, actual)")


if __name__ == "__main__":
    main()