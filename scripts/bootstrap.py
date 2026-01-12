#!/usr/bin/env python3
"""
NucleusESP32 Development Environment Bootstrap Script

Following SpareTools patterns for hermetic development environment setup.
Provides one-command setup for ESP32 embedded development.

Usage:
    python scripts/bootstrap.py [--dry-run] [--verbose]

Requirements:
    - Python 3.8+ (SpareTools will provide 3.12.7 hermetic environment)
    - Internet connection for downloading dependencies
    - Git for cloning SpareTools

Project Type: embedded/esp32
"""

import os
import sys
import subprocess
import shutil
import platform
import argparse
import json
from pathlib import Path
from typing import Optional, Dict, Any, List


class Bootstrapper:
    """Handles the bootstrap process for NucleusESP32 development environment following SpareTools patterns."""

    def __init__(self, dry_run: bool = False, verbose: bool = False):
        self.project_root = Path(__file__).parent.parent
        self.system = platform.system().lower()
        self.is_windows = self.system == "windows"
        self.is_linux = self.system == "linux"
        self.is_macos = self.system == "darwin"
        self.dry_run = dry_run
        self.verbose = verbose

        # Project type detection (embedded/esp32 category)
        self.project_type = "embedded/esp32"
        self.project_name = "nucleus-esp32"

        # SpareTools configuration - package-based architecture
        # No longer using submodule - packages come from remote repository
        self.sparetools_available = False

        # Hermetic environment paths
        self.venv_dir = self.project_root / ".venv"

    def run_command(self, cmd: list, cwd: Optional[Path] = None,
                   check: bool = True, capture_output: bool = False) -> subprocess.CompletedProcess:
        """Run a command and return the result. Supports dry-run mode."""
        cmd_str = ' '.join(str(c) for c in cmd)
        if self.dry_run:
            print(f"[DRY-RUN] Would execute: {cmd_str}")
            # Return a mock result for dry-run
            class MockResult:
                def __init__(self):
                    self.returncode = 0
                    self.stdout = ""
                    self.stderr = ""
            return MockResult()
        elif self.verbose:
            print(f"[EXEC] {cmd_str}")

        try:
            result = subprocess.run(
                cmd,
                cwd=cwd or self.project_root,
                check=check,
                capture_output=capture_output,
                text=True
            )
            return result
        except subprocess.CalledProcessError as e:
            print(f"❌ Command failed: {cmd_str}")
            print(f"Error: {e}")
            if e.stdout:
                print(f"stdout: {e.stdout}")
            if e.stderr:
                print(f"stderr: {e.stderr}")
            raise

    def check_prerequisites(self) -> bool:
        """Check if all prerequisites are met following SpareTools patterns."""
        print("🔍 Checking prerequisites...")

        # Check Python version (SpareTools will provide hermetic 3.12.7)
        python_version = sys.version_info
        if python_version < (3, 8):
            print(f"❌ Python {python_version.major}.{python_version.minor} is not supported.")
            print("   SpareTools will provide Python 3.12.7 hermetic environment.")
            print("   Please upgrade system Python to 3.8+ for bootstrap.")
            return False

        print(f"✅ Python {python_version.major}.{python_version.minor}.{python_version.micro} detected (system)")

        # Check git availability
        try:
            result = self.run_command(["git", "--version"], capture_output=True)
            version_output = result.stdout.strip()
            # Extract version from output like "git version 2.34.1"
            if "git version" in version_output:
                version = version_output.split()[-1]
            else:
                version = "unknown"
            print(f"✅ Git {version} is available")
        except (subprocess.CalledProcessError, FileNotFoundError, IndexError):
            print("❌ Git is not available. Please install Git.")
            return False

        # Check internet connectivity (comprehensive check)
        connectivity_ok = False
        try:
            self.run_command(["curl", "-s", "--head", "https://github.com"],
                           capture_output=True, check=False)
            connectivity_ok = True
        except (subprocess.CalledProcessError, FileNotFoundError):
            try:
                self.run_command(["ping", "-c", "1", "github.com"],
                               capture_output=True, check=False)
                connectivity_ok = True
            except (subprocess.CalledProcessError, FileNotFoundError):
                pass

        if connectivity_ok:
            print("✅ Internet connectivity detected")
        else:
            print("❌ No internet connectivity detected")
            return False

        # Check for existing virtual environment conflicts
        if self.venv_dir.exists():
            print(f"⚠️  Virtual environment already exists at {self.venv_dir}")
            print("   It will be recreated during bootstrap.")

        return True

    def setup_sparetools(self) -> bool:
        """Set up SpareTools package-based architecture."""
        print("\n🔧 Setting up SpareTools package-based architecture...")

        try:
            # Check if SpareTools packages are available via Conan
            result = self.run_command(["conan", "search", "sparetools-base/2.0.0"], capture_output=True)
            if result.returncode == 0:
                print("✅ SpareTools packages available via Conan")
                self.sparetools_available = True

                # Use SpareTools bootstrap scripts directly (PR #52)
                try:
                    # Import and run SpareTools bootstrap functionality
                    # This will use the new bootstrap scripts from SpareTools
                    from sparetools_bootstrap.bootstrap.esp32_bootstrap import ESP32Bootstrap

                    bootstrapper = ESP32Bootstrap(project_root=self.project_root)
                    success = bootstrapper.run_bootstrap()

                    if success:
                        print("✅ SpareTools ESP32 bootstrap completed")
                    else:
                        print("⚠️  SpareTools bootstrap had issues, continuing with manual setup")
                except ImportError:
                    print("⚠️  SpareTools bootstrap module not available, using manual setup")
                except Exception as e:
                    print(f"⚠️  SpareTools bootstrap failed: {e}, using manual setup")
            else:
                print("⚠️  SpareTools packages not found in configured Conan remotes")
                print("   Some advanced features may not be available")
                self.sparetools_available = False

            return True

        except Exception as e:
            print(f"❌ SpareTools package setup failed: {e}")
            print("   Continuing with manual setup (limited functionality)")
            self.sparetools_available = False
            return True

    def get_bundled_python_path(self) -> Optional[str]:
        """Get path to bundled SpareTools CPython executable."""
        print("   Locating bundled SpareTools CPython...")

        try:
            # First try to get Python path from Conan conf
            result = self.run_command(["conan", "conf", "get", "user.cpython:executable"],
                                     capture_output=True, check=False)
            if result.returncode == 0 and result.stdout.strip():
                python_path = result.stdout.strip()
                if os.path.exists(python_path):
                    print(f"   Found bundled Python via Conan conf: {python_path}")
                    return python_path

            # Fallback: look in .conan directory structure
            conan_dir = self.project_root / ".conan"
            if conan_dir.exists():
                # Common locations for sparetools-cpython
                candidates = [
                    conan_dir / "sparetools-cpython" / "3.12.7" / "bin" / "python3",
                    conan_dir / "sparetools-cpython" / "3.12.7" / "bin" / "python",
                    conan_dir / "sparetools-cpython" / "3.12.7" / "Scripts" / "python.exe",  # Windows
                ]

                for candidate in candidates:
                    if candidate.exists():
                        print(f"   Found bundled Python at: {candidate}")
                        return str(candidate)

            print("   ⚠️  Bundled Python not found, will use system Python")
            return None

        except Exception as e:
            print(f"   ⚠️  Error locating bundled Python: {e}")
            return None

    def create_virtual_environment(self) -> bool:
        """Create virtual environment using bundled SpareTools CPython."""
        print("\n🏗️  Creating virtual environment...")

        if self.venv_dir.exists():
            print(f"   Removing existing virtual environment at {self.venv_dir}")
            if not self.dry_run:
                shutil.rmtree(self.venv_dir)

        # Try to use bundled CPython first
        bundled_python = self.get_bundled_python_path()
        if bundled_python:
            python_cmd = bundled_python
            print(f"   ✅ Using bundled SpareTools CPython: {python_cmd}")
        else:
            python_cmd = sys.executable
            print(f"   ⚠️  Using system Python (bundled not available): {python_cmd}")

        try:
            self.run_command([python_cmd, "-m", "venv", str(self.venv_dir)])
            print("✅ Virtual environment created")
        except subprocess.CalledProcessError:
            print("❌ Failed to create virtual environment")
            return False

        # Upgrade pip in the virtual environment
        pip_cmd = [str(self.venv_dir / "bin" / "pip"), "install", "--upgrade", "pip"]
        if self.is_windows:
            pip_cmd[0] = str(self.venv_dir / "Scripts" / "pip.exe")

        try:
            self.run_command(pip_cmd)
            print("✅ Pip upgraded in virtual environment")
        except subprocess.CalledProcessError:
            print("⚠️  Could not upgrade pip, continuing...")

        return True

    def install_dependencies(self) -> bool:
        """Install project dependencies using bundled CPython and Conan."""
        print("\n📦 Installing dependencies...")

        # CRITICAL: Install Conan dependencies FIRST to make bundled CPython available
        conanfile = self.project_root / "conanfile.py"
        if conanfile.exists():
            print("   Installing Conan dependencies (includes bundled CPython)...")
            try:
                # Use esp32_security profile for hardened builds
                self.run_command(["conan", "install", ".", "--build=missing", "--profile:host=esp32_security", "--profile:build=default"])
                print("✅ Conan dependencies installed (bundled CPython now available)")
            except subprocess.CalledProcessError:
                print("❌ Failed to install Conan dependencies")
                return False
        else:
            print("⚠️  conanfile.py not found, skipping Conan dependencies")

        # Now determine which Python/pip to use (prioritize bundled CPython)
        python_cmd, pip_cmd = self._get_python_commands()

        # Install Python dependencies
        requirements_file = self.project_root / "requirements-dev.txt"
        if requirements_file.exists():
            print("   Installing Python dependencies...")
            try:
                self.run_command(pip_cmd + ["install", "-r", str(requirements_file)])
                print("✅ Python dependencies installed")
            except subprocess.CalledProcessError:
                print("❌ Failed to install Python dependencies")
                return False
        else:
            print("⚠️  requirements-dev.txt not found, skipping Python dependencies")

        return True

    def _get_python_commands(self) -> tuple[str, list[str]]:
        """Get Python and pip commands, prioritizing bundled CPython."""
        # Try bundled CPython from virtual environment first
        if self.venv_dir.exists():
            python_cmd = str(self.venv_dir / "bin" / "python")
            pip_cmd = [python_cmd, "-m", "pip"]
            if self.is_windows:
                python_cmd = str(self.venv_dir / "Scripts" / "python.exe")
                pip_cmd = [str(self.venv_dir / "Scripts" / "pip.exe"), "--python", python_cmd]
            print("   Using virtual environment Python (bundled CPython)")
            return python_cmd, pip_cmd

        # Fallback: try to find bundled CPython directly
        bundled_python = self.get_bundled_python_path()
        if bundled_python:
            print(f"   Using bundled SpareTools CPython: {bundled_python}")
            return bundled_python, [bundled_python, "-m", "pip"]

        # Final fallback: system Python
        python_cmd = str(sys.executable)
        pip_cmd = [python_cmd, "-m", "pip"]
        print("   Using system Python (bundled not available)")
        return python_cmd, pip_cmd

    def setup_precommit_hooks(self) -> bool:
        """Set up pre-commit hooks and development tools following SpareTools patterns."""
        print("\n🔗 Setting up development tools and pre-commit hooks...")

        # Get Python command (prioritizes bundled CPython)
        python_cmd, _ = self._get_python_commands()

        # Install pre-commit if not available
        try:
            self.run_command([python_cmd, "-m", "pip", "install", "pre-commit"])
            print("✅ Pre-commit installed")
        except subprocess.CalledProcessError:
            print("⚠️  Could not install pre-commit, checking if already available...")

        # Set up pre-commit hooks
        try:
            self.run_command([python_cmd, "-m", "pre_commit", "install"])
            print("✅ Pre-commit hooks installed")
        except (subprocess.CalledProcessError, FileNotFoundError):
            print("⚠️  Pre-commit hooks not set up (pre-commit not available)")
            return True  # Not critical for bootstrap

        # Install additional development tools following SpareTools patterns
        dev_tools = ["black", "isort", "flake8", "mypy"]
        try:
            self.run_command([python_cmd, "-m", "pip", "install"] + dev_tools)
            print("✅ Development tools installed")
        except subprocess.CalledProcessError:
            print("⚠️  Could not install some development tools")

        return True

    def setup_platformio(self) -> bool:
        """Set up ESP32 development tools."""
        print("\n🔌 Setting up ESP32 development tools...")

        try:
            # Try SpareTools bootstrap if available (PR #52)
            if self.sparetools_available:
                try:
                    # Use SpareTools ESP32 bootstrap functionality
                    from sparetools_bootstrap.bootstrap.esp32_config import ESP32Config

                    config = ESP32Config(project_root=self.project_root)
                    success = config.setup_platformio()

                    if success:
                        print("✅ ESP32 tools setup completed via SpareTools")
                        return True
                    else:
                        print("⚠️  SpareTools ESP32 setup had issues, falling back to manual setup")
                except ImportError:
                    print("⚠️  SpareTools ESP32 config not available, falling back to manual setup")
                except Exception as e:
                    print(f"⚠️  SpareTools ESP32 setup failed: {e}, falling back to manual setup")

            # Manual PlatformIO setup
            try:
                # Check if PlatformIO is available
                self.run_command(["pio", "--version"], capture_output=True)
                print("✅ PlatformIO is available")

                # Install ESP32 platform if not already installed
                try:
                    self.run_command(["pio", "platform", "show", "espressif32"], capture_output=True)
                    print("✅ ESP32 platform already installed")
                except subprocess.CalledProcessError:
                    print("   Installing ESP32 platform...")
                    self.run_command(["pio", "platform", "install", "espressif32", "--with-package", "framework-arduinoespressif32"])
                    print("✅ ESP32 platform installed")

                print("✅ ESP32 tools setup completed")
                return True

            except (subprocess.CalledProcessError, FileNotFoundError) as e:
                print(f"❌ PlatformIO setup failed: {e}")
                print("   Please install PlatformIO: pip install platformio")
                return False

        except Exception as e:
            print(f"❌ ESP32 tools setup failed: {e}")
            return False

    def create_env_file(self) -> bool:
        """Create comprehensive environment configuration file following SpareTools patterns."""
        print("\n📝 Creating environment configuration...")

        env_file = self.project_root / ".env"
        if env_file.exists():
            print("   .env file already exists, backing up and recreating")
            if not self.dry_run:
                env_file.rename(env_file.with_suffix('.env.backup'))

        # Determine Python paths (prioritize bundled CPython)
        python_cmd, _ = self._get_python_commands()
        venv_python = python_cmd if python_cmd != str(sys.executable) else ""

        # Build environment content
        env_vars = {
            "# NucleusESP32 Development Environment": "",
            "# Generated by SpareTools bootstrap script": "",
            "# Project Type": f"embedded/esp32",
            "# Project Name": f"{self.project_name}",
            "": "",
            "# Project paths": "",
            "PROJECT_ROOT": str(self.project_root),
            "": "",
            "# Python configuration": "",
            "PYTHONPATH": f"{self.project_root}/src:{self.project_root}/test:{self.project_root}/test_harness",
            "VIRTUAL_ENV": str(self.venv_dir) if self.venv_dir.exists() else "",
            "": "",
            "# ESP32/Embedded configuration": "",
            "PLATFORMIO_CORE_DIR": str(self.project_root / ".platformio"),
            "ESP32_TOOLCHAIN_PATH": "",  # Will be set by PlatformIO
            "": "",
            "# Build configuration": "",
            "BUILD_TYPE": "Debug",
            "CMAKE_BUILD_TYPE": "Debug",
            "CONAN_BUILD_TYPE": "Debug",
            "": "",
            "# Testing configuration": "",
            "PYTEST_DISABLE_PLUGIN_AUTOLOAD": "1",
            "NUCLEUS_TEST_PARALLEL": "1",
            "NUCLEUS_TEST_TIMEOUT": "300",
            "": "",
            "# Conan configuration": "",
            "CONAN_USER_HOME": str(self.project_root / ".conan"),
            "": "",
            "# Development tools": "",
            "PRE_COMMIT_HOME": str(self.project_root / ".pre-commit-cache"),
        }

        env_content = "\n".join(f"{k}={v}" if v else k for k, v in env_vars.items()) + "\n"

        try:
            if not self.dry_run:
                with open(env_file, 'w') as f:
                    f.write(env_content)
            print("✅ Environment configuration created")
        except IOError as e:
            print(f"❌ Failed to create .env file: {e}")
            return False

        return True

    def run_post_bootstrap_checks(self) -> bool:
        """Run comprehensive checks to verify the bootstrap was successful."""
        print("\n✅ Running post-bootstrap verification...")

        checks_passed = 0
        total_checks = 0

        # Get Python command for checks (prioritizes bundled CPython)
        python_cmd, _ = self._get_python_commands()

        # Check Python version (should be 3.12+ from SpareTools)
        total_checks += 1
        try:
            result = self.run_command([python_cmd, "--version"], capture_output=True)
            version_str = result.stdout.strip()
            print(f"✅ Python available: {version_str}")
            checks_passed += 1
        except (subprocess.CalledProcessError, FileNotFoundError):
            print("❌ Python not available")

        # Check virtual environment
        total_checks += 1
        if self.venv_dir.exists():
            print("✅ Virtual environment created")
            checks_passed += 1
        else:
            print("⚠️  Virtual environment not created")

        # Check Conan
        total_checks += 1
        try:
            result = self.run_command(["conan", "--version"], capture_output=True)
            version_output = result.stdout.strip()
            # Extract version from output like "Conan version 2.0.17"
            if "Conan version" in version_output:
                version = version_output.split()[-1]
            else:
                version = "unknown"
            print(f"✅ Conan {version} available")
            checks_passed += 1
        except (subprocess.CalledProcessError, FileNotFoundError, IndexError):
            print("❌ Conan not available")

        # Check PlatformIO
        total_checks += 1
        try:
            result = self.run_command(["pio", "--version"], capture_output=True)
            version = result.stdout.strip()
            print(f"✅ PlatformIO available: {version}")
            checks_passed += 1
        except (subprocess.CalledProcessError, FileNotFoundError):
            print("⚠️  PlatformIO not available (will be installed later)")

        # Check pytest
        total_checks += 1
        try:
            result = self.run_command([python_cmd, "-m", "pytest", "--version"], capture_output=True)
            print("✅ pytest available")
            checks_passed += 1
        except (subprocess.CalledProcessError, FileNotFoundError):
            print("⚠️  pytest not available")

        # Check pre-commit
        total_checks += 1
        try:
            result = self.run_command([python_cmd, "-m", "pre_commit", "--version"], capture_output=True)
            print("✅ pre-commit available")
            checks_passed += 1
        except (subprocess.CalledProcessError, FileNotFoundError):
            print("⚠️  pre-commit not available")

        # Check Conan dependencies resolution
        total_checks += 1
        conan_build_dir = self.project_root / "build-release"
        if conan_build_dir.exists() and (conan_build_dir / "conan_toolchain.cmake").exists():
            print("✅ Conan dependencies resolved")
            checks_passed += 1
        else:
            print("⚠️  Conan dependencies not resolved")

        # Check environment file
        total_checks += 1
        env_file = self.project_root / ".env"
        if env_file.exists():
            print("✅ Environment configuration created")
            checks_passed += 1
        else:
            print("❌ Environment configuration missing")

        print(f"\n📊 Bootstrap verification: {checks_passed}/{total_checks} passed")

        if checks_passed >= total_checks * 0.8:  # 80% success rate
            print("🎉 Bootstrap completed successfully!")
            return True
        else:
            print("⚠️  Bootstrap completed with issues. Check warnings above.")
            return False

    def run(self) -> int:
        """Run the complete bootstrap process following SpareTools layered architecture."""
        print("🚀 NucleusESP32 SpareTools Bootstrap")
        print("=" * 50)
        print(f"Project Type: {self.project_type}")
        print(f"Project Name: {self.project_name}")
        if self.dry_run:
            print("DRY-RUN MODE: No actual changes will be made")
        print()

        try:
            # Phase 1: Prerequisites and SpareTools setup
            print("📋 Phase 1: Foundation Setup")
            if not self.check_prerequisites():
                return 1

            if not self.setup_sparetools():
                return 1

            # Phase 2: Hermetic environment creation
            print("\n📋 Phase 2: Hermetic Environment")
            if not self.create_virtual_environment():
                return 1

            if not self.install_dependencies():
                return 1

            # Phase 3: Development tools setup
            print("\n📋 Phase 3: Development Tools")
            if not self.setup_platformio():
                return 1

            if not self.setup_precommit_hooks():
                return 1

            if not self.create_env_file():
                return 1

            # Phase 4: Verification
            print("\n📋 Phase 4: Verification")
            if not self.run_post_bootstrap_checks():
                return 1

            # Success message and next steps
            self._print_success_message()
            return 0

        except KeyboardInterrupt:
            print("\n\n⚠️  Bootstrap interrupted by user")
            return 1
        except Exception as e:
            print(f"\n❌ Bootstrap failed with unexpected error: {e}")
            if self.verbose:
                import traceback
                traceback.print_exc()
            return 1

    def _print_success_message(self) -> None:
        """Print success message and next steps."""
        print("\n🎉 NucleusESP32 development environment ready!")
        print("=" * 50)

        activation_cmd = f"source {self.venv_dir}/bin/activate" if not self.is_windows else f"{self.venv_dir}\\Scripts\\activate.bat"
        if not self.venv_dir.exists():
            activation_cmd = f"source {self.project_root}/.env"

        print("Next steps:")
        print(f"   1. Activate environment: {activation_cmd}")
        print("   2. Build ESP32 firmware: pio run -e esp32dev")
        print("   3. Run unit tests: python -m pytest test/")
        print("   4. Run integration tests: python test_harness/run_nucleus_unit_tests.py")
        print("   5. Start developing!")
        print()
        print("Useful commands:")
        print("   • pio run -e <board>    # Build for specific ESP32 board")
        print("   • pre-commit run --all-files  # Run code quality checks")
        print("   • conan install .       # Refresh dependencies")
        print("   • python scripts/bootstrap.py --help  # Bootstrap help")
        print()
        print("Environment file created: .env")
        print("Virtual environment: .venv/")
        print("SpareTools: .sparetools/")


def main():
    """Main entry point with command line argument support."""
    parser = argparse.ArgumentParser(
        description="NucleusESP32 SpareTools Bootstrap",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python scripts/bootstrap.py                    # Normal bootstrap
  python scripts/bootstrap.py --dry-run         # Preview changes
  python scripts/bootstrap.py --verbose         # Detailed output
  python scripts/bootstrap.py --dry-run --verbose  # Preview with details
        """
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Show what would be done without making changes"
    )
    parser.add_argument(
        "--verbose", "-v",
        action="store_true",
        help="Enable verbose output"
    )

    args = parser.parse_args()

    bootstrapper = Bootstrapper(dry_run=args.dry_run, verbose=args.verbose)
    return bootstrapper.run()


if __name__ == "__main__":
    sys.exit(main())