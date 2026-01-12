cmake_minimum_required(VERSION 3.20)
project(NucleusESP32
    VERSION 0.1.0
    DESCRIPTION "ESP32-based multi-tool device firmware"
    LANGUAGES CXX C
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -Wno-unused-parameter
        -Wno-missing-field-initializers
    )
endif()

# Enable testing
enable_testing()

# Configure Conan integration
if(EXISTS ${CMAKE_BINARY_DIR}/conan/conan_toolchain.cmake)
    include(${CMAKE_BINARY_DIR}/conan/conan_toolchain.cmake)
elseif(EXISTS ${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
    include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
endif()

# Configure code coverage
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
if(ENABLE_COVERAGE)
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
endif()

# Configure clang-tidy
option(ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy)
    if(CLANG_TIDY_EXE)
        set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_EXE})
    else()
        message(WARNING "clang-tidy requested but executable not found")
    endif()
endif()

# Configure cppcheck
option(ENABLE_CPPCHECK "Enable cppcheck" OFF)
if(ENABLE_CPPCHECK)
    find_program(CPPCHECK_EXE NAMES cppcheck)
    if(CPPCHECK_EXE)
        set(CMAKE_CXX_CPPCHECK
            ${CPPCHECK_EXE}
            --enable=all
            --suppress=missingIncludeSystem
            --suppress=unusedFunction
            --error-exitcode=1
            --inline-suppr
        )
    else()
        message(WARNING "cppcheck requested but executable not found")
    endif()
endif()

# Configure address sanitizer
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
if(ENABLE_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

# Configure undefined behavior sanitizer
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
if(ENABLE_UBSAN)
    add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=undefined)
endif()

# Find Conan dependencies
find_package(GTest REQUIRED)

# Define source files for each module
set(CORE_SOURCES
    src/main.cpp
)

set(NFC_SOURCES
    src/modules/nfc/nfc.cpp
    src/modules/nfc/mfrc522_reader.cpp
)

set(RF_SOURCES
    src/modules/RF/CC1101.cpp
    src/modules/RF/brute.cpp
)

set(IR_SOURCES
    src/modules/IR/ir.cpp
)

set(GUI_SOURCES
    src/GUI/ScreenManager.cpp
    src/GUI/events.cpp
)

set(UTILITY_SOURCES
    src/modules/ETC/SDcard.cpp
    src/modules/dataProcessing/SubGHzParser.cpp
)

# Create module libraries
add_library(core_lib ${CORE_SOURCES})
target_include_directories(core_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)

add_library(nfc_lib ${NFC_SOURCES})
target_include_directories(nfc_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/modules/nfc
)
target_link_libraries(nfc_lib PUBLIC core_lib)

add_library(rf_lib ${RF_SOURCES})
target_include_directories(rf_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/modules/RF
)
target_link_libraries(rf_lib PUBLIC core_lib)

add_library(ir_lib ${IR_SOURCES})
target_include_directories(ir_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/modules/IR
)
target_link_libraries(ir_lib PUBLIC core_lib)

add_library(gui_lib ${GUI_SOURCES})
target_include_directories(gui_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/GUI
)
target_link_libraries(gui_lib PUBLIC core_lib)

add_library(utility_lib ${UTILITY_SOURCES})
target_include_directories(utility_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/modules/ETC
    ${CMAKE_SOURCE_DIR}/src/modules/dataProcessing
)
target_link_libraries(utility_lib PUBLIC core_lib)

# Create main firmware executable (for ESP32 this would be handled by PlatformIO)
if(NOT ESP32_BUILD)
    add_executable(nucleus_firmware ${CORE_SOURCES})
    target_link_libraries(nucleus_firmware
        PRIVATE
        nfc_lib
        rf_lib
        ir_lib
        gui_lib
        utility_lib
    )
endif()

# Common mocks library (following oms-dev pattern)
add_library(common_mocks
    src/Tests/CommonMocks/CommonMocks.cpp
)
target_include_directories(common_mocks PUBLIC
    ${CMAKE_SOURCE_DIR}/src/Tests/CommonMocks
    ${CMAKE_SOURCE_DIR}/src/Tests
    ${CMAKE_SOURCE_DIR}/test
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(common_mocks PUBLIC GTest::gmock)

# Unit tests - Following oms-dev pattern: src/Tests/NucleusUnitTests/
set(UNIT_TEST_SOURCES
    src/Tests/NucleusUnitTests/main.cpp
    src/Tests/NucleusUnitTests/pch.cpp
    src/Tests/NucleusUnitTests/NfcModuleTests.cpp
    src/Tests/NucleusUnitTests/RfModuleTests.cpp
    src/Tests/NucleusUnitTests/IrModuleTests.cpp
    src/Tests/NucleusUnitTests/DataProcessingTests.cpp
    # Legacy test file (keep for compatibility)
    test/test_nfc.cpp
)

add_executable(unit_tests ${UNIT_TEST_SOURCES})

target_link_libraries(unit_tests
    PRIVATE
    GTest::gtest_main
    GTest::gmock
    common_mocks
    nfc_lib
    rf_lib
    ir_lib
    gui_lib
    utility_lib
)

target_include_directories(unit_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/Tests/NucleusUnitTests
    ${CMAKE_SOURCE_DIR}/src/Tests
    ${CMAKE_SOURCE_DIR}/test
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)

# Define UNIT_TEST for test builds
target_compile_definitions(unit_tests PRIVATE UNIT_TEST)

# Enable precompiled headers for unit tests (following oms-dev pattern)
target_precompile_headers(unit_tests PRIVATE src/Tests/NucleusUnitTests/pch.h)

# Note: Integration tests are primarily Python-based and run via pytest
# Mock headers are included via target_include_directories for testing

# Test targets
include(GoogleTest)
gtest_discover_tests(unit_tests)

# Add test labels for better organization
set_tests_properties(${gtest_discover_tests}
    PROPERTIES
    LABELS "unit"
    TIMEOUT 60
)

# Performance tests
add_custom_target(performance_tests
    COMMAND ${CMAKE_COMMAND} -E echo "Running performance tests..."
    COMMAND unit_tests --gtest_filter="*Performance*"
    COMMENT "Running performance tests"
)

# Memory leak tests (with valgrind)
find_program(VALGRIND_EXE NAMES valgrind)
if(VALGRIND_EXE)
    add_custom_target(memcheck
        COMMAND ${VALGRIND_EXE}
            --leak-check=full
            --show-leak-kinds=all
            --track-origins=yes
            --verbose
            --log-file=valgrind.log
            $<TARGET_FILE:unit_tests>
        COMMENT "Running memory leak checks with Valgrind"
    )
endif()

# Coverage target
if(ENABLE_COVERAGE)
    find_program(GCOVR_EXE NAMES gcovr)
    if(GCOVR_EXE)
        add_custom_target(coverage
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
            COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}
                ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND ${GCOVR_EXE} -r ${CMAKE_SOURCE_DIR}
                --html --html-details
                --exclude="test/*"
                --exclude="build/*"
                --exclude=".*"
                -o coverage/index.html
                --print-summary
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating coverage report"
        )

        add_custom_target(coverage_xml
            COMMAND ${GCOVR_EXE} -r ${CMAKE_SOURCE_DIR}
                --xml -o coverage/coverage.xml
                --exclude="test/*"
                --exclude="build/*"
                --exclude=".*"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating XML coverage report"
        )
    endif()
endif()

# Static analysis targets
add_custom_target(static_analysis
    COMMAND ${CMAKE_COMMAND} -E echo "Running static analysis..."
    COMMENT "Running static analysis tools"
)

if(ENABLE_CLANG_TIDY)
    add_custom_target(clang_tidy_check
        COMMAND ${CMAKE_COMMAND} -E echo "Running clang-tidy..."
        COMMAND ${CLANG_TIDY_EXE} ${NFC_SOURCES} ${RF_SOURCES} ${IR_SOURCES} ${GUI_SOURCES} ${UTILITY_SOURCES}
            --header-filter=${CMAKE_SOURCE_DIR}/src/*
            --warnings-as-errors=*
            -p ${CMAKE_BINARY_DIR}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-tidy static analysis"
    )
    add_dependencies(static_analysis clang_tidy_check)
endif()

if(ENABLE_CPPCHECK)
    add_custom_target(cppcheck
        COMMAND ${CPPCHECK_EXE}
            --enable=all
            --suppress=missingIncludeSystem
            --suppress=unusedFunction
            --error-exitcode=1
            --inline-suppr
            --xml
            --xml-version=2
            ${CMAKE_SOURCE_DIR}/src
        COMMENT "Running cppcheck static analysis"
    )
    add_dependencies(static_analysis cppcheck)
endif()

# Documentation generation
find_program(DOXYGEN_EXE NAMES doxygen)
if(DOXYGEN_EXE)
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXE} ${CMAKE_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating documentation with Doxygen"
    )
endif()

# Clean all build artifacts
add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/*
    COMMENT "Cleaning all build artifacts"
)

# Install target (for packaging)
install(TARGETS nucleus_firmware unit_tests
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# Package configuration
include(CPack)
set(CPACK_PACKAGE_NAME "nucleus-esp32")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "ESP32-based multi-tool device firmware")
set(CPACK_PACKAGE_VENDOR "NucleusESP32 Team")
set(CPACK_PACKAGE_CONTACT "team@nucleus-esp32.local")

# Print build configuration summary
message(STATUS "========================================")
message(STATUS "NucleusESP32 Build Configuration")
message(STATUS "========================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "C Standard: ${CMAKE_C_STANDARD}")
message(STATUS "Coverage: ${ENABLE_COVERAGE}")
message(STATUS "Clang-Tidy: ${ENABLE_CLANG_TIDY}")
message(STATUS "Cppcheck: ${ENABLE_CPPCHECK}")
message(STATUS "ASan: ${ENABLE_ASAN}")
message(STATUS "UBSan: ${ENABLE_UBSAN}")
message(STATUS "Testing: Enabled")
message(STATUS "========================================") 