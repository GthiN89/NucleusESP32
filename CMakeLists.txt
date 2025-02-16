cmake_minimum_required(VERSION 3.10)
project(NucleusESP32 CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -Wno-unused-parameter
    )
endif()

# Enable testing
enable_testing()

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
            --error-exitcode=1
            --inline-suppr
        )
    else()
        message(WARNING "cppcheck requested but executable not found")
    endif()
endif()

# Download and configure GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/release-1.12.1.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add NFC module source files
set(NFC_SOURCES
    src/modules/nfc/nfc.cpp
    src/modules/nfc/mfrc522_reader.cpp
)

# Create NFC library
add_library(nfc_lib ${NFC_SOURCES})
target_include_directories(nfc_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/modules/nfc
)

# Add test executable
add_executable(nfc_tests
    test/test_nfc.cpp
)

# Link test executable with Google Test and NFC library
target_link_libraries(nfc_tests
    PRIVATE
    gtest_main
    nfc_lib
)

target_include_directories(nfc_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/test
    ${CMAKE_SOURCE_DIR}/src
)

# Define UNIT_TEST for test builds
target_compile_definitions(nfc_tests PRIVATE UNIT_TEST)

# Add coverage target
if(ENABLE_COVERAGE)
    add_custom_target(coverage
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
        COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}
            ${CMAKE_CTEST_COMMAND} --output-on-failure
        COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}
            gcovr -r ${CMAKE_SOURCE_DIR} --html --html-details
            -o coverage/index.html
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating coverage report"
    )
endif()

# Discover tests
include(GoogleTest)
gtest_discover_tests(nfc_tests) 