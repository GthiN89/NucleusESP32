"""
Pytest Plugin for NucleusESP32 Test Harness
===========================================

Pytest plugin providing fixtures and utilities for NucleusESP32 testing
following SpareTools patterns.
"""

import pytest
from pathlib import Path
from .nucleus_test_harness import NucleusTestHarness


@pytest.fixture(scope="session")
def nucleus_harness(request):
    """
    Session-scoped test harness fixture.

    Provides a NucleusTestHarness instance for the entire test session.
    Results are stored in the session results directory.
    """
    # Get results directory from pytest config or use default
    results_dir = getattr(request.config, "_nucleus_results_dir", None)
    if results_dir is None:
        results_dir = Path("test-results") / "pytest-session"

    harness = NucleusTestHarness(results_dir=results_dir, hardware_simulation=True)
    yield harness

    # Generate session report at the end
    report_file = results_dir / "pytest_session_report.json"
    harness.generate_comprehensive_report(report_file)


@pytest.fixture(scope="function")
def hardware_harness(request, tmp_path):
    """
    Function-scoped hardware test harness fixture.

    Provides a fresh NucleusTestHarness instance for each test function.
    Hardware simulation is enabled by default.
    """
    results_dir = tmp_path / "hardware_test_results"
    harness = NucleusTestHarness(results_dir=results_dir, hardware_simulation=True)

    yield harness

    # Reset hardware simulation between tests
    harness.reset_hardware_simulation()


@pytest.fixture(scope="function")
def integration_harness(request, tmp_path):
    """
    Function-scoped integration test harness fixture.

    Similar to hardware_harness but configured for integration testing.
    """
    results_dir = tmp_path / "integration_test_results"
    harness = NucleusTestHarness(results_dir=results_dir, hardware_simulation=True)

    yield harness

    # Generate integration-specific report
    report_file = results_dir / "integration_report.json"
    harness.generate_comprehensive_report(report_file)


@pytest.fixture(autouse=True)
def setup_test_environment(request):
    """
    Auto-used fixture to set up test environment variables.

    Sets environment variables needed for NucleusESP32 testing.
    """
    import os

    # Set test environment variables
    original_env = {}
    test_env_vars = {
        "NUCLEUS_TEST_MODE": "1",
        "NUCLEUS_HARDWARE_SIMULATION": "1",
        "PYTEST_DISABLE_PLUGIN_AUTOLOAD": "1",
    }

    for key, value in test_env_vars.items():
        original_env[key] = os.environ.get(key)
        os.environ[key] = value

    yield

    # Restore original environment
    for key, value in original_env.items():
        if value is None:
            os.environ.pop(key, None)
        else:
            os.environ[key] = value


def pytest_configure(config):
    """Configure pytest with NucleusESP32 test harness settings."""
    # Add custom markers
    config.addinivalue_line(
        "markers", "hardware: marks tests as hardware integration tests"
    )
    config.addinivalue_line(
        "markers", "integration: marks tests as integration tests"
    )
    config.addinivalue_line(
        "markers", "simulation: marks tests as hardware simulation tests"
    )

    # Store results directory for fixtures
    results_base = Path(config.getoption("--basetemp", Path("test-results")))
    config._nucleus_results_dir = results_base / "nucleus"


def pytest_addoption(parser):
    """Add command line options for NucleusESP32 testing."""
    group = parser.getgroup("nucleus", "NucleusESP32 test harness options")

    group.addoption(
        "--hardware-simulation",
        action="store_true",
        default=True,
        help="Enable hardware simulation mode (default: enabled)"
    )

    group.addoption(
        "--disable-hardware-simulation",
        action="store_false",
        dest="hardware_simulation",
        help="Disable hardware simulation mode"
    )

    group.addoption(
        "--nucleus-results-dir",
        type=Path,
        help="Directory to store NucleusESP32 test results"
    )


def pytest_collection_modifyitems(config, items):
    """
    Modify test collection to add markers and skip tests based on configuration.
    """
    hardware_simulation = config.getoption("--hardware-simulation", True)

    for item in items:
        # Add simulation marker if hardware simulation is enabled
        if hardware_simulation and "hardware" in item.keywords:
            item.add_marker(pytest.mark.simulation)

        # Skip hardware tests if simulation is disabled and no real hardware available
        if not hardware_simulation and "hardware" in item.keywords:
            # Check if real hardware is available (would be implemented)
            hardware_available = False  # Placeholder
            if not hardware_available:
                item.add_marker(pytest.mark.skip(reason="Real hardware not available"))


def pytest_terminal_summary(terminalreporter, exitstatus, config):
    """
    Add NucleusESP32 test summary to pytest terminal output.
    """
    if hasattr(config, "_nucleus_results_dir"):
        results_dir = config._nucleus_results_dir
        if results_dir.exists():
            terminalreporter.write_sep("=", "NucleusESP32 Test Results")
            terminalreporter.write_line(f"Results directory: {results_dir}")

            # Count result files
            json_files = list(results_dir.glob("**/*.json"))
            if json_files:
                terminalreporter.write_line(f"Generated {len(json_files)} result files")

            terminalreporter.write_line(f"Open reports: file://{results_dir}/pytest_session_report.json")


# Export fixtures for direct import
__all__ = [
    "nucleus_harness",
    "hardware_harness",
    "integration_harness",
    "setup_test_environment",
]