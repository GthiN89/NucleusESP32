# NucleusESP32 Test Harness

Python integration test harness using SpareTools test framework.

## Overview

Following the `ngapy-dev` pattern, this test harness uses:
- **SpareTools Python Environment**: Provided by `sparetools-cpython/3.12.7`
- **SpareTools Test Harness**: `sparetools-test-harness/2.0.0` with ngapy-compatible API
- **Integration Tests**: Hardware and module interaction tests

## Structure

```
test_harness/
├── __init__.py                    # Package initialization
├── test_integration.py            # Integration test harness
├── test_hardware.py               # Hardware test harness
└── run_nucleus_unit_tests.py      # Test runner script
```

## Usage

### Running Integration Tests

```bash
# Using the test runner
python test_harness/run_nucleus_unit_tests.py

# Or directly
python -m test_harness.test_integration
```

### Using SpareTools Test Harness

The test harness provides ngapy-compatible API:

```python
from test_harness import IntegrationTestHarness

th = IntegrationTestHarness()

# Use ngapy-style verification
th.verification.verify(actual, expected, "Test description", test_num=1)
th.verification.verify_tol(actual, expected, tolerance, "Tolerance test", test_num=2)
th.verification.verify_range(value, min_val, max_val, "Range test", test_num=3)
```

## Test Environment Setup

The Python test environment is automatically provided by SpareTools when using Conan:

```bash
# Install dependencies (includes SpareTools Python)
conan install . --build=missing

# Python environment is now available
python test_harness/run_nucleus_unit_tests.py
```

## Test Results

Test results are stored in `test-results/` directory:
- Text logs: `test-results/*.txt`
- JUnit XML: `test-results/*.xml`

## Integration with CI/CD

The test harness generates JUnit XML reports compatible with CI/CD systems:
- GitHub Actions
- Jenkins
- GitLab CI

## References

- Ngapy Test Harness: `/home/sparrow/Desktop/oms/ngapy-dev/test_harness/`
- SpareTools Test Harness: `~/sparetools/packages/foundation/sparetools-test-harness/`
