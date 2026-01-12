#!/usr/bin/env python3
"""
Run NucleusESP32 Unit Tests
===========================

Script to run unit tests using SpareTools Python environment.
Following ngapy-dev pattern: python test environment provided by SpareTools.
"""

import os
import sys
import subprocess
from pathlib import Path

def main():
    """Run unit tests using SpareTools Python environment."""
    
    # Get SpareTools Python executable
    # This would be set by Conan build environment
    python_exe = os.environ.get('PYTHON_EXECUTABLE', sys.executable)
    
    # Test harness directory
    test_harness_dir = Path(__file__).parent
    
    # Run integration tests
    print("Running NucleusESP32 integration tests...")
    sys.path.insert(0, str(test_harness_dir))
    
    try:
        from test_harness import run_integration_tests, run_hardware_tests
        
        results_dir = Path("test-results")
        results_dir.mkdir(exist_ok=True)
        
        # Run integration tests
        integration_result = run_integration_tests(results_dir)
        print(f"Integration tests: {integration_result}")
        
        # Run hardware tests
        hardware_result = run_hardware_tests(results_dir)
        print(f"Hardware tests: {hardware_result}")
        
        # Overall result
        if integration_result == "pass" and hardware_result == "pass":
            print("All tests passed!")
            return 0
        else:
            print("Some tests failed!")
            return 1
            
    except ImportError as e:
        print(f"Error importing test harness: {e}")
        print("Ensure SpareTools packages are installed via Conan.")
        return 1
    except Exception as e:
        print(f"Test execution error: {e}")
        import traceback
        traceback.print_exc()
        return 1


if __name__ == "__main__":
    sys.exit(main())
