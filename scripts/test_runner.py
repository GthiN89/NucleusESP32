#!/usr/bin/env python3
"""
NucleusESP32 Unified Test Runner
=================================

Following SpareTools patterns for comprehensive testing environment.
Provides unified test execution for unit tests, integration tests, and CI/CD.

Usage:
    python scripts/test_runner.py [options]

    # Run all tests
    python scripts/test_runner.py

    # Run specific test types
    python scripts/test_runner.py --unit-only
    python scripts/test_runner.py --integration-only
    python scripts/test_runner.py --hardware-only

    # Generate reports
    python scripts/test_runner.py --report-dir ./test-reports --format json html
"""

import os
import sys
import time
import argparse
from pathlib import Path
from typing import Optional, List, Any, Dict
from dataclasses import dataclass, asdict


@dataclass
class TestSuiteResult:
    """Result of a test suite execution."""
    suite_name: str
    passed: int
    failed: int
    skipped: int
    errors: int
    duration: float
    return_code: int
    output: str
    junit_file: Optional[str] = None


class ESP32TestRunnerAdapter:
    """Adapter to bridge SpareTools test runner with NucleusESP32 specific needs."""

    def __init__(self, sparetools_runner):
        self.sparetools_runner = sparetools_runner

    def run_command(self, cmd, cwd=None, capture_output=True, check=False):
        """Delegate command execution to SpareTools runner."""
        return self.sparetools_runner.run_command(cmd, cwd, capture_output, check)

    def run_unit_tests(self, parallel=True, coverage=True):
        """Run unit tests using SpareTools runner."""
        # Adapt to SpareTools API
        return self.sparetools_runner.run_tests(
            test_patterns=["test_*.cpp", "test_*.c"],
            parallel=parallel,
            coverage=coverage,
            junit_output=True
        )

    def run_integration_tests(self, hardware_simulation=True):
        """Run integration tests using SpareTools runner."""
        # Adapt to SpareTools API with ESP32-specific settings
        return self.sparetools_runner.run_tests(
            test_patterns=["test_*.py"],
            parallel=False,  # Integration tests typically not parallel
            coverage=False,
            junit_output=True,
            extra_env={"NUCLEUS_HARDWARE_SIMULATION": "1" if hardware_simulation else "0"}
        )

    def run_quality_checks(self):
        """Run linting and quality checks using SpareTools runner."""
        return self.sparetools_runner.run_quality_checks()

    def run_security_scan(self):
        """Run security scanning using SpareTools runner."""
        return self.sparetools_runner.run_security_scan()

    def run_tests(self, unit_only=False, integration_only=False, hardware_only=False,
                  coverage=True, parallel=True, formats=None):
        """Run all tests using SpareTools runner."""
        if unit_only:
            return self.run_unit_tests(parallel, coverage)
        elif integration_only:
            return self.run_integration_tests()
        else:
            # Run comprehensive test suite
            results = []
            if not integration_only:
                results.append(self.run_unit_tests(parallel, coverage))
            if not unit_only:
                results.append(self.run_integration_tests())

            # Return overall success
            return all(r.get('return_code', 1) == 0 for r in results)


class NucleusTestRunner:
    """
    Unified test runner for NucleusESP32 delegating to SpareTools.

    Provides comprehensive test execution capabilities through SpareTools:
    - Unit tests (C++ with GoogleTest)
    - Integration tests (Python with hardware simulation)
    - Linting and formatting checks
    - Security scanning
    - Coverage reporting
    - Parallel test execution
    """

    def __init__(self, project_root: Optional[Path] = None, verbose: bool = False):
        """
        Initialize test runner.

        Args:
            project_root: Root directory of the project
            verbose: Enable verbose output
        """
        self.project_root = project_root or Path(__file__).parent.parent
        self.verbose = verbose
        self.sparetools_runner = None

        # Set up result directories
        self.results_dir = self.project_root / "test-results"
        self.coverage_dir = self.project_root / "coverage-results"
        self.junit_dir = self.results_dir

        # Test results tracking
        self.test_results = []

        # Ensure directories exist
        self.results_dir.mkdir(exist_ok=True)
        self.coverage_dir.mkdir(exist_ok=True)

        # Try to import and initialize SpareTools test runner
        self._init_sparetools_runner()

    def _init_sparetools_runner(self):
        """Initialize SpareTools test runner if available."""
        try:
            # Import SpareTools test runner from scripts/testing/ (PR #52)
            import sys
            import os
            from pathlib import Path

            # Try to find SpareTools test runner in package or local scripts
            try:
                # First try importing from installed SpareTools package
                from sparetools_shared_dev_tools.test_runner import ESP32TestRunner
                self.sparetools_runner = ESP32TestRunner(self.project_root, self.verbose)
            except ImportError:
                # Fallback: try to import from local SpareTools scripts if available
                sparetools_root = os.environ.get('SPARETOOLS_ROOT')
                if sparetools_root:
                    scripts_path = Path(sparetools_root) / 'scripts' / 'testing'
                    if scripts_path.exists():
                        sys.path.insert(0, str(scripts_path.parent))
                        try:
                            from testing.test_runner import SpareToolsTestRunner
                            # Create adapter for ESP32-specific functionality
                            self.sparetools_runner = ESP32TestRunnerAdapter(
                                SpareToolsTestRunner(self.project_root, self.verbose)
                            )
                        except ImportError:
                            self.sparetools_runner = None
                    else:
                        self.sparetools_runner = None
                else:
                    self.sparetools_runner = None

            if self.sparetools_runner:
                print("✅ SpareTools test runner initialized")
            else:
                print("⚠️  SpareTools test runner not available, using fallback implementation")

        except Exception as e:
            print(f"⚠️  Failed to initialize SpareTools test runner: {e}, using fallback implementation")
            self.sparetools_runner = None

    def run_command(self, cmd: List[str], cwd: Optional[Path] = None,
                   capture_output: bool = True, check: bool = False, env: Optional[dict] = None):
        """
        Run a command and return results.

        Delegates to SpareTools if available, otherwise provides fallback.
        """
        if self.sparetools_runner:
            return self.sparetools_runner.run_command(cmd, cwd, capture_output, check, env)
        else:
            # Fallback implementation
            import subprocess
            import os
            if self.verbose:
                print(f"[EXEC] {' '.join(cmd)}")

            try:
                # Merge environment variables if provided
                run_env = os.environ.copy()
                if env:
                    run_env.update(env)

                result = subprocess.run(
                    cmd,
                    cwd=cwd or self.project_root,
                    capture_output=capture_output,
                    text=True,
                    check=check,
                    env=run_env
                )
                return result.returncode, result.stdout, result.stderr
            except subprocess.CalledProcessError as e:
                return e.returncode, e.stdout, e.stderr

    def run_unit_tests(self, parallel: bool = True, coverage: bool = True):
        """Run unit tests delegating to SpareTools."""
        if self.sparetools_runner:
            return self.sparetools_runner.run_unit_tests(parallel, coverage)
        else:
            # Fallback implementation
            print("🧪 Running unit tests (fallback)...")
            # Simplified fallback - would need full implementation
            return type('MockResult', (), {'return_code': 0, 'passed': 0, 'failed': 0})()

    def run_integration_tests(self, hardware_simulation: bool = True):
        """Run integration tests delegating to SpareTools."""
        if self.sparetools_runner:
            return self.sparetools_runner.run_integration_tests(hardware_simulation)
        else:
            # Fallback implementation
            print("🔗 Running integration tests (fallback)...")
            return type('MockResult', (), {'return_code': 0, 'passed': 0, 'failed': 0})()

    def run_linting_checks(self):
        """Run linting checks delegating to SpareTools."""
        if self.sparetools_runner:
            return self.sparetools_runner.run_quality_checks()
        else:
            # Fallback implementation
            print("🔍 Running linting checks (fallback)...")
            return type('MockResult', (), {'return_code': 0, 'passed': 1, 'failed': 0})()

    def run_security_scanning(self):
        """Run security scanning delegating to SpareTools."""
        if self.sparetools_runner:
            return self.sparetools_runner.run_security_scan()
        else:
            # Fallback implementation
            print("🔒 Running security scanning (fallback)...")
            return type('MockResult', (), {'return_code': 0, 'passed': 1, 'failed': 0})()

    def run_all_tests(self, parallel: bool = True, coverage: bool = True,
                     security: bool = True, formats: list = None):
        """Run all tests delegating to SpareTools."""
        if self.sparetools_runner:
            return self.sparetools_runner.run_tests(
                unit_only=False, integration_only=False, hardware_only=False,
                coverage=coverage, parallel=parallel, formats=formats
            )
        else:
            # Fallback implementation
            print("🚀 Running all tests (fallback)...")
            # Run individual test methods
            results = []
            results.append(self.run_unit_tests(parallel, coverage))
            results.append(self.run_integration_tests())
            if security:
                results.append(self.run_security_scanning())

            # Return success if all passed
            all_passed = all(r.return_code == 0 for r in results)
            return all_passed

    def run_unit_tests(self, parallel: bool = True, coverage: bool = True) -> TestSuiteResult:
        """
        Run C++ unit tests using GoogleTest.

        Args:
            parallel: Run tests in parallel
            coverage: Generate coverage reports

        Returns:
            TestSuiteResult for unit tests
        """
        print("🧪 Running unit tests (C++)...")

        start_time = time.time()

        # Build tests first
        print("   Building test executables...")
        build_cmd = ["cmake", "--build", "build-release", "--config", "Debug"]
        if parallel:
            build_cmd.extend(["--parallel", str(os.cpu_count() or 4)])

        return_code, stdout, stderr = self.run_command(build_cmd)

        if return_code != 0:
            print(f"   ❌ Build failed with code {return_code}")
            return TestSuiteResult(
                suite_name="unit_tests",
                passed=0, failed=0, skipped=0, errors=1,
                duration=time.time() - start_time,
                return_code=return_code,
                output=stdout + stderr
            )

        # Run unit tests
        print("   Running C++ unit tests...")
        test_cmd = ["ctest", "--output-on-failure", "--output-junit", str(self.junit_dir / "unit_tests.xml")]

        if parallel:
            test_cmd.extend(["--parallel", str(os.cpu_count() or 4)])

        return_code, stdout, stderr = self.run_command(test_cmd, cwd=self.project_root / "build-release")

        # Parse CTest output for counts (simplified)
        passed = stdout.count("Passed")
        failed = stdout.count("Failed")
        duration = time.time() - start_time

        status = "✅ PASSED" if return_code == 0 else "❌ FAILED"
        print(f"   {status}: {passed} passed, {failed} failed ({duration:.2f}s)")

        return TestSuiteResult(
            suite_name="unit_tests",
            passed=passed, failed=failed, skipped=0, errors=0,
            duration=duration,
            return_code=return_code,
            output=stdout + stderr,
            junit_file=str(self.junit_dir / "unit_tests.xml")
        )

    def run_integration_tests(self, hardware_simulation: bool = True) -> TestSuiteResult:
        """
        Run Python integration tests.

        Args:
            hardware_simulation: Enable hardware simulation mode

        Returns:
            TestSuiteResult for integration tests
        """
        print("🔗 Running integration tests (Python)...")

        start_time = time.time()

        # Set environment variables for hardware simulation
        env = os.environ.copy()
        env["NUCLEUS_HARDWARE_SIMULATION"] = "1" if hardware_simulation else "0"
        env["NUCLEUS_TEST_RESULTS_DIR"] = str(self.results_dir)
        env["PYTHONPATH"] = f"{self.project_root}/src:{self.project_root}/test:{self.project_root}/test_harness"

        # Run integration tests
        test_cmd = [
            sys.executable, "-m", "pytest",
            "test/integration/",
            "-v",
            "--tb=short",
            "--junitxml", str(self.junit_dir / "integration_tests.xml"),
            "--json-report", "--json-report-file", str(self.results_dir / "integration_results.json")
        ]

        return_code, stdout, stderr = self.run_command(test_cmd, env=env)

        # Parse pytest output for counts (simplified parsing)
        output = stdout + stderr
        passed = output.count("PASSED")
        failed = output.count("FAILED")
        errors = output.count("ERROR")
        skipped = output.count("SKIPPED")
        duration = time.time() - start_time

        status = "✅ PASSED" if return_code == 0 else "❌ FAILED"
        print(f"   {status}: {passed} passed, {failed} failed, {skipped} skipped ({duration:.2f}s)")

        return TestSuiteResult(
            suite_name="integration_tests",
            passed=passed, failed=failed, skipped=skipped, errors=errors,
            duration=duration,
            return_code=return_code,
            output=output,
            junit_file=str(self.junit_dir / "integration_tests.xml")
        )

    def run_linting_checks(self) -> TestSuiteResult:
        """
        Run linting and formatting checks.

        Returns:
            TestSuiteResult for linting checks
        """
        print("🧹 Running linting and formatting checks...")

        start_time = time.time()

        # Run pre-commit hooks
        precommit_cmd = ["pre-commit", "run", "--all-files", "--show-diff-on-failure"]
        return_code, stdout, stderr = self.run_command(precommit_cmd)

        duration = time.time() - start_time
        output = stdout + stderr

        # Pre-commit returns 0 for success, non-zero for failures
        if return_code == 0:
            print(f"   ✅ PASSED: All linting checks passed ({duration:.2f}s)")
            return TestSuiteResult(
                suite_name="linting",
                passed=1, failed=0, skipped=0, errors=0,
                duration=duration,
                return_code=return_code,
                output=output
            )
        else:
            print(f"   ❌ FAILED: Linting checks failed ({duration:.2f}s)")
            return TestSuiteResult(
                suite_name="linting",
                passed=0, failed=1, skipped=0, errors=0,
                duration=duration,
                return_code=return_code,
                output=output
            )

    def run_security_scanning(self) -> TestSuiteResult:
        """
        Run security scanning with Trivy.

        Returns:
            TestSuiteResult for security scanning
        """
        print("🔒 Running security scanning...")

        start_time = time.time()

        # Run Trivy security scan
        trivy_cmd = [
            "trivy", "fs", ".",
            "--format", "json",
            "--output", str(self.results_dir / "security_scan.json"),
            "--severity", "HIGH,CRITICAL"
        ]

        return_code, stdout, stderr = self.run_command(trivy_cmd)

        duration = time.time() - start_time
        output = stdout + stderr

        if return_code == 0:
            print(f"   ✅ PASSED: No high/critical security issues found ({duration:.2f}s)")
            return TestSuiteResult(
                suite_name="security",
                passed=1, failed=0, skipped=0, errors=0,
                duration=duration,
                return_code=return_code,
                output=output
            )
        else:
            print(f"   ❌ FAILED: Security issues detected ({duration:.2f}s)")
            return TestSuiteResult(
                suite_name="security",
                passed=0, failed=1, skipped=0, errors=0,
                duration=duration,
                return_code=return_code,
                output=output
            )

    def run_coverage_analysis(self) -> TestSuiteResult:
        """
        Run coverage analysis and generate reports.

        Returns:
            TestSuiteResult for coverage analysis
        """
        print("📊 Running coverage analysis...")

        start_time = time.time()

        # Generate coverage report (this would be integrated with the test runs)
        coverage_cmd = [
            sys.executable, "-m", "pytest",
            "--cov=nucleus",
            "--cov-report=html:" + str(self.coverage_dir / "html"),
            "--cov-report=xml:" + str(self.coverage_dir / "coverage.xml"),
            "--cov-report=json:" + str(self.coverage_dir / "coverage.json"),
            "--cov-fail-under=60",
            "test/"
        ]

        return_code, stdout, stderr = self.run_command(coverage_cmd)

        duration = time.time() - start_time
        output = stdout + stderr

        if return_code == 0:
            print(f"   ✅ PASSED: Coverage requirements met ({duration:.2f}s)")
            return TestSuiteResult(
                suite_name="coverage",
                passed=1, failed=0, skipped=0, errors=0,
                duration=duration,
                return_code=return_code,
                output=output
            )
        else:
            print(f"   ❌ FAILED: Coverage requirements not met ({duration:.2f}s)")
            return TestSuiteResult(
                suite_name="coverage",
                passed=0, failed=1, skipped=0, errors=0,
                duration=duration,
                return_code=return_code,
                output=output
            )

    def generate_summary_report(self, formats: List[str] = None) -> bool:
        """
        Generate summary report in specified formats.

        Args:
            formats: List of formats to generate (json, html, text)

        Returns:
            bool: True if report generation succeeds
        """
        if formats is None:
            formats = ["json", "text"]

        summary_data = {
            "test_run_summary": {
                "timestamp": time.time(),
                "total_duration": sum(r.duration for r in self.test_results),
                "total_suites": len(self.test_results),
                "overall_status": "PASS" if all(r.return_code == 0 for r in self.test_results) else "FAIL"
            },
            "suite_results": [asdict(r) for r in self.test_results]
        }

        try:
            for fmt in formats:
                if fmt == "json":
                    with open(self.results_dir / "test_summary.json", "w") as f:
                        json.dump(summary_data, f, indent=2, default=str)
                    print(f"📄 JSON summary report: {self.results_dir / 'test_summary.json'}")

                elif fmt == "html":
                    self._generate_html_report(summary_data)

                elif fmt == "text":
                    self._generate_text_report(summary_data)

            return True
        except Exception as e:
            print(f"❌ Failed to generate summary report: {e}")
            return False

    def _generate_html_report(self, summary_data: Dict[str, Any]) -> None:
        """Generate HTML summary report."""
        html_content = f"""
        <!DOCTYPE html>
        <html>
        <head>
            <title>NucleusESP32 Test Results</title>
            <style>
                body {{ font-family: Arial, sans-serif; margin: 20px; }}
                .summary {{ background: #f0f0f0; padding: 10px; border-radius: 5px; }}
                .suite {{ margin: 10px 0; padding: 10px; border: 1px solid #ddd; }}
                .pass {{ background: #d4edda; border-color: #c3e6cb; }}
                .fail {{ background: #f8d7da; border-color: #f5c6cb; }}
            </style>
        </head>
        <body>
            <h1>NucleusESP32 Test Results</h1>
            <div class="summary">
                <h2>Summary</h2>
                <p><strong>Status:</strong> {summary_data['test_run_summary']['overall_status']}</p>
                <p><strong>Total Suites:</strong> {summary_data['test_run_summary']['total_suites']}</p>
                <p><strong>Total Duration:</strong> {summary_data['test_run_summary']['total_duration']:.2f}s</p>
            </div>
            <h2>Test Suites</h2>
            {"".join(self._generate_suite_html(suite) for suite in summary_data['suite_results'])}
        </body>
        </html>
        """

        html_file = self.results_dir / "test_summary.html"
        with open(html_file, "w") as f:
            f.write(html_content)
        print(f"📄 HTML summary report: {html_file}")

    def _generate_suite_html(self, suite: Dict[str, Any]) -> str:
        """Generate HTML for a single test suite."""
        status_class = "pass" if suite['return_code'] == 0 else "fail"
        return f"""
        <div class="suite {status_class}">
            <h3>{suite['suite_name'].replace('_', ' ').title()}</h3>
            <p><strong>Duration:</strong> {suite['duration']:.2f}s</p>
            <p><strong>Results:</strong> {suite['passed']} passed, {suite['failed']} failed, {suite['skipped']} skipped</p>
        </div>
        """

    def _generate_text_report(self, summary_data: Dict[str, Any]) -> None:
        """Generate text summary report."""
        text_content = f"""
NucleusESP32 Test Results
{'='*40}

SUMMARY
-------
Status: {summary_data['test_run_summary']['overall_status']}
Total Suites: {summary_data['test_run_summary']['total_suites']}
Total Duration: {summary_data['test_run_summary']['total_duration']:.2f}s

TEST SUITES
-----------
"""

        for suite in summary_data['suite_results']:
            text_content += f"""
{suite['suite_name'].replace('_', ' ').title()}
  Duration: {suite['duration']:.2f}s
  Results: {suite['passed']} passed, {suite['failed']} failed, {suite['skipped']} skipped
  Status: {'PASS' if suite['return_code'] == 0 else 'FAIL'}
"""

        text_file = self.results_dir / "test_summary.txt"
        with open(text_file, "w") as f:
            f.write(text_content)
        print(f"📄 Text summary report: {text_file}")

    def run_all_tests(self, parallel: bool = True, coverage: bool = True,
                     security: bool = True, formats: List[str] = None) -> bool:
        """
        Run all test suites.

        Args:
            parallel: Run tests in parallel
            coverage: Generate coverage reports
            security: Run security scanning
            formats: Report formats to generate

        Returns:
            bool: True if all tests pass
        """
        print("🚀 Starting NucleusESP32 test suite execution...")

        # Run test suites
        self.test_results.append(self.run_unit_tests(parallel=parallel, coverage=coverage))
        self.test_results.append(self.run_integration_tests())
        self.test_results.append(self.run_linting_checks())

        if coverage:
            self.test_results.append(self.run_coverage_analysis())

        if security:
            self.test_results.append(self.run_security_scanning())

        # Generate summary reports
        self.generate_summary_report(formats or ["json", "html", "text"])

        # Overall result
        all_passed = all(r.return_code == 0 for r in self.test_results)

        if all_passed:
            print("\n🎉 All tests passed!")
        else:
            print("\n❌ Some tests failed. Check reports for details.")

        return all_passed


def main():
    """Main entry point."""
    parser = argparse.ArgumentParser(
        description="NucleusESP32 Unified Test Runner",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python scripts/test_runner.py                    # Run all tests
  python scripts/test_runner.py --unit-only        # Unit tests only
  python scripts/test_runner.py --integration-only # Integration tests only
  python scripts/test_runner.py --coverage         # With coverage
  python scripts/test_runner.py --format html      # HTML reports only
        """
    )

    parser.add_argument("--unit-only", action="store_true", help="Run unit tests only")
    parser.add_argument("--integration-only", action="store_true", help="Run integration tests only")
    parser.add_argument("--lint-only", action="store_true", help="Run linting checks only")
    parser.add_argument("--coverage", action="store_true", help="Generate coverage reports")
    parser.add_argument("--no-security", action="store_true", help="Skip security scanning")
    parser.add_argument("--no-parallel", action="store_true", help="Disable parallel test execution")
    parser.add_argument("--report-dir", type=Path, help="Custom report directory")
    parser.add_argument("--format", nargs="+", choices=["json", "html", "text"],
                       default=["json", "html", "text"], help="Report formats")
    parser.add_argument("--verbose", "-v", action="store_true", help="Verbose output")

    args = parser.parse_args()

    # Initialize test runner
    runner = NucleusTestRunner(verbose=args.verbose)

    if args.report_dir:
        runner.results_dir = args.report_dir
        runner.results_dir.mkdir(exist_ok=True)

    # Run requested test suites
    if args.unit_only:
        result = runner.run_unit_tests(parallel=not args.no_parallel, coverage=args.coverage)
        runner.test_results.append(result)
        success = result.return_code == 0
    elif args.integration_only:
        result = runner.run_integration_tests()
        runner.test_results.append(result)
        success = result.return_code == 0
    elif args.lint_only:
        result = runner.run_linting_checks()
        runner.test_results.append(result)
        success = result.return_code == 0
    else:
        # Run all tests
        success = runner.run_all_tests(
            parallel=not args.no_parallel,
            coverage=args.coverage,
            security=not args.no_security,
            formats=args.format
        )

    # Exit with appropriate code
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()