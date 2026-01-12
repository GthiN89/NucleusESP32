from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, CMake
from conan.tools.layout import basic_layout
from conan.tools.files import copy
import os


class NucleusESP32Conan(ConanFile):
    """NucleusESP32 Conan package following SpareTools layered architecture."""

    name = "nucleus-esp32"
    version = "0.1.0"
    package_type = "application"

# Foundation layer - SpareTools base provides common utilities and configuration
python_requires = "sparesparrow/sparetools-base/2.0.4"

# Optional metadata
license = "MIT"
author = "NucleusESP32 Team"
url = "https://github.com/sparesparrow/NucleusESP32"
description = "ESP32-based multi-tool device firmware"
topics = ("esp32", "embedded", "rf", "nfc", "iot")

# Binary configuration - focus on host testing, not ESP32 cross-compilation
settings = "os", "compiler", "build_type", "arch"

# Sources are located in the same place as this recipe
exports_sources = "CMakeLists.txt", "src/*", "include/*", "test/*", "test_harness/*"

def layout(self):
    basic_layout(self, src_folder=".")

    def build_requirements(self):
        # Use SpareTools BOM for version management
        versions = self.python_requires["sparetools-base"].module.SpareToolsVersions.versions

        # Tools layer - SpareTools CPython provides hermetic Python environment
        # Enable SpareTools CPython for bundled Python environment
        self.tool_requires(f"sparetools-cpython/{versions['cpython']}")

        # Test harness for Python integration tests (ngapy-style)
        # Enable test harness when available from SpareTools
        self.tool_requires(f"sparetools-test-harness/{versions['test-harness']}")

        # Shared development tools - enable for development environment
        self.tool_requires(f"sparetools-shared-dev-tools/{versions['shared-dev-tools']}")

        # ESP32-specific bootstrap tools
        self.tool_requires(f"sparetools-bootstrap/{versions['bootstrap']}")

    def requirements(self):
        # Use SpareTools BOM for version management
        versions = self.python_requires["sparetools-base"].module.SpareToolsVersions.versions

        # Testing framework for host-based C++ unit tests
        self.requires(f"gtest/{versions['gtest']}")

        # FlatBuffers for message protocol serialization (external dependency for now)
        self.requires("flatbuffers/23.5.26")

        # SpareTools HAL for Sunton ESP32 boards
        self.requires(f"sparetools-hal-sunton/{versions['hal-sunton']}")

        # SpareTools crypto suite for security features
        self.requires(f"sparetools-crypto-suite/{versions['crypto-suite']}")

    def generate(self):
        # Generate CMake toolchain and dependencies for host testing
        tc = CMakeToolchain(self)
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

        # Copy test harness Python modules (following ngapy-dev pattern)
        if os.path.exists(os.path.join(self.source_folder, "test_harness")):
            copy(self, "test_harness/**/*.py",
                 src=self.source_folder,
                 dst=os.path.join(self.package_folder, "test_harness"))

    def package_info(self):
        # Define components for different modules (when built)
        # Note: These will be populated when actual libraries are built
        # self.cpp_info.components["rf_module"].libs = ["rf"]
        # self.cpp_info.components["nfc_module"].libs = ["nfc"]
        # self.cpp_info.components["ir_module"].libs = ["ir"]

        # Python test environment setup following SpareTools patterns
        # Use buildenv_info and runenv_info for Conan 2.x compatibility
        self.buildenv_info.define("NUCLEUS_PACKAGE_DIR", self.package_folder)
        self.buildenv_info.append_path("PYTHONPATH", self.package_folder)
        if os.path.exists(os.path.join(self.package_folder, "test_harness")):
            self.buildenv_info.append_path("PYTHONPATH", os.path.join(self.package_folder, "test_harness"))

        self.runenv_info.define("NUCLEUS_PACKAGE_DIR", self.package_folder)
        self.runenv_info.append_path("PYTHONPATH", self.package_folder)
        if os.path.exists(os.path.join(self.package_folder, "test_harness")):
            self.runenv_info.append_path("PYTHONPATH", os.path.join(self.package_folder, "test_harness"))

        # Mark as development package following SpareTools conventions
        self.cpp_info.libs = []  # No libraries provided at package level