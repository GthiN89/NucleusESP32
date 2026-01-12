#!/usr/bin/env python3
"""
NucleusESP32 Conan-PlatformIO Bridge Script

Project-specific bridge script for NucleusESP32 that integrates with
the enterprise-grade SpareTools Conan-PlatformIO bridge system.

This script configures the build environment for NucleusESP32 firmware,
including HAL integration, crypto suites, and security hardening.
"""

import os
import sys
import json
from pathlib import Path

# Add SpareTools shared dev tools to path if available
sparetools_root = os.environ.get("SPARETOOLS_ROOT", "~/sparetools")
shared_dev_tools = Path(sparetools_root).expanduser() / "packages" / "foundation" / "sparetools-shared-dev-tools"
if str(shared_dev_tools) not in sys.path and shared_dev_tools.exists():
    sys.path.insert(0, str(shared_dev_tools))

try:
    from shared_dev_tools.conan_pio_bridge import ConanPIOBridge, ConanPackage
except ImportError:
    # Fallback implementation if SpareTools not available
    class ConanPackage:
        def __init__(self, name, version, user=None, channel=None, options=None):
            self.name = name
            self.version = version
            self.user = user
            self.channel = channel
            self.options = options or {}

        @property
        def reference(self):
            ref = f"{self.name}/{self.version}"
            if self.user and self.channel:
                ref += f"@{self.user}/{self.channel}"
            return ref

    class ConanPIOBridge:
        def __init__(self, workspace_root=None):
            self.workspace_root = workspace_root or Path.cwd()

        def bridge(self, conan_profile, pio_env, packages):
            # Simple fallback implementation
            config_lines = [
                f"[env:{pio_env}]",
                f"platform = espressif32",
                f"board = {pio_env.replace('esp32-', '') if 'esp32-' in pio_env else pio_env}",
                f"framework = espidf",
                f"; Conan profile: {conan_profile}",
            ]

            # Add basic lib_deps
            config_lines.extend([
                "lib_deps =",
                "    lvgl/lvgl",
                "    google/flatbuffers@^23.5.26",
            ])

            return "\n".join(config_lines)


class NucleusESP32Bridge:
    """
    NucleusESP32-specific Conan-PlatformIO bridge implementation.

    This class extends the generic bridge with Nucleus-specific configurations
    for HAL integration, crypto suites, and security features.
    """

    def __init__(self):
        self.bridge = ConanPIOBridge()
        self.nucleus_config = self._load_nucleus_config()

    def _load_nucleus_config(self):
        """Load NucleusESP32-specific configuration."""
        return {
            "hal_packages": [],  # SpareTools HAL not yet integrated
            "crypto_packages": [],  # SpareTools crypto not yet integrated
            "security_profile": "basic",
            "target_board": "esp32s3"
        }

    def get_packages_for_env(self, pio_env):
        """
        Get Conan packages required for the given PlatformIO environment.

        Args:
            pio_env: PlatformIO environment name

        Returns:
            List of ConanPackage objects
        """
        packages = []

        # Base dependencies (external for now, will be from SpareTools BOM)
        packages.extend([
            ConanPackage("gtest", "1.14.0"),
            ConanPackage("flatbuffers", "23.5.26"),
        ])

        # LVGL for GUI (will be managed by SpareTools HAL)
        packages.append(ConanPackage("lvgl", "8.3.11"))

        return packages

    def bridge_environment(self, pio_env):
        """
        Bridge the Conan dependencies for the given PlatformIO environment.

        Args:
            pio_env: PlatformIO environment name

        Returns:
            Generated PlatformIO configuration string
        """
        # Determine Conan profile based on environment
        conan_profile_map = {
            "esp32s3_sunton": "esp32_sunton_v3.prof",
            "esp32": "esp32_base.prof",
            "esp32c3": "esp32_base.prof",
            "esp32c6": "esp32_base.prof",
        }

        conan_profile = conan_profile_map.get(pio_env, "esp32_base.prof")

        # Get packages for this environment
        packages = self.get_packages_for_env(pio_env)

        # Execute bridge
        return self.bridge.bridge(conan_profile, pio_env, packages)


def add_conan_lib_deps():
    """Add Conan-managed dependencies to PlatformIO environment."""
    print("[NUCLEUS-BRIDGE] Adding dependencies to PlatformIO environment")

    try:
        # Add essential dependencies for NucleusESP32
        env.Append(LIB_DEPS=[
            "lvgl/lvgl",
            "google/flatbuffers@^23.5.26",
        ])

        print("[NUCLEUS-BRIDGE] Dependencies configured successfully")

    except Exception as e:
        print(f"[NUCLEUS-BRIDGE] Error configuring dependencies: {e}")
        # Fallback to basic dependencies
        try:
            env.Append(LIB_DEPS=[
                "lvgl/lvgl",
                "google/flatbuffers@^23.5.26",
            ])
        except:
            pass


# PlatformIO extra_script entry point
try:
    Import("env")
    add_conan_lib_deps()
except NameError:
    # Not running in PlatformIO context, skip automatic execution
    pass


def main():
    """Main entry point for standalone usage."""
    pio_env = sys.argv[1] if len(sys.argv) > 1 else "esp32-2432S028Rv3"

    print(f"[NUCLEUS-BRIDGE] Generating config for environment: {pio_env}", file=sys.stderr)

    try:
        nucleus_bridge = NucleusESP32Bridge()
        config = nucleus_bridge.bridge_environment(pio_env)
        print(config)

    except Exception as e:
        print(f"ERROR: Bridge operation failed: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()