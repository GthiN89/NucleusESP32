"""
NucleusESP32 Test Harness
=========================

Integration test harness using SpareTools test framework (ngapy-compatible API).
Python test environment provided by SpareTools (sparetools-cpython).
"""

try:
    from sparetools_test_harness import SpareToolsTestHarness
    from sparetools_test_harness.pytest_plugin import PytestTestHarness
except ImportError:
    # Fallback if SpareTools packages not available
    SpareToolsTestHarness = None
    PytestTestHarness = None

from .nucleus_test_harness import NucleusTestHarness
from .test_integration import IntegrationTestHarness, run_integration_tests
from .test_hardware import HardwareTestHarness, run_hardware_tests
from .pytest_plugin import nucleus_harness, hardware_harness, integration_harness

__version__ = '0.1.0'

__all__ = [
    'SpareToolsTestHarness',
    'PytestTestHarness',
    'NucleusTestHarness',
    'IntegrationTestHarness',
    'HardwareTestHarness',
    'nucleus_harness',
    'hardware_harness',
    'integration_harness',
    'run_integration_tests',
    'run_hardware_tests',
]
