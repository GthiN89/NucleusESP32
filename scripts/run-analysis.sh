#!/bin/bash

# Exit on error and undefined variables
set -euo pipefail
IFS=$'\n\t'

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Error handler
trap 'echo -e "${RED}Error occurred in script at line $LINENO${NC}"' ERR

# Check for required tools
required_tools=("cmake" "clang-format" "clang-tidy" "cppcheck" "cpplint" "gcovr")
missing_tools=()

for tool in "${required_tools[@]}"; do
    if ! command -v "$tool" &> /dev/null; then
        missing_tools+=("$tool")
    fi
done

if [ ${#missing_tools[@]} -ne 0 ]; then
    echo -e "${RED}Error: Missing required tools: ${missing_tools[*]}${NC}"
    exit 1
fi

echo -e "${YELLOW}Running code analysis tools...${NC}"

# Directory containing source files
SRC_DIR="src/modules/nfc"
TEST_DIR="test"

# Verify source directories exist
if [ ! -d "$SRC_DIR" ] || [ ! -d "$TEST_DIR" ]; then
    echo -e "${RED}Error: Source or test directory not found${NC}"
    exit 1
fi

# Create build directory if it doesn't exist
mkdir -p build
cd build || exit 1

# Configure CMake with all analysis options enabled
echo -e "\n${YELLOW}Configuring CMake...${NC}"
cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DENABLE_CLANG_TIDY=ON \
        -DENABLE_CPPCHECK=ON \
        -DENABLE_COVERAGE=ON \
        -DCMAKE_BUILD_TYPE=Debug

# Function to run tool with error handling
run_tool() {
    local tool_name=$1
    local cmd=$2
    local is_fatal=${3:-true}  # Third parameter determines if errors are fatal
    echo -e "\n${YELLOW}Running ${tool_name}...${NC}"
    if ! eval "$cmd"; then
        echo -e "${RED}${tool_name} found issues${NC}"
        if [ "$is_fatal" = true ]; then
            return 1
        fi
    else
        echo -e "${GREEN}${tool_name} passed${NC}"
    fi
    return 0
}

# Run clang-format (non-fatal)
run_tool "clang-format" "find ../$SRC_DIR ../$TEST_DIR -name '*.cpp' -o -name '*.h' | while read -r file; do
    clang-format --dry-run -Werror \"\$file\" || {
        echo -e \"${YELLOW}Formatting issues found in \$file${NC}\"
        clang-format --dry-run \"\$file\" 2>&1
    }
done" false

# Run clang-tidy
if [ -f compile_commands.json ]; then
    run_tool "clang-tidy" "find ../$SRC_DIR ../$TEST_DIR -name '*.cpp' | while read -r file; do
        echo \"Checking \$file...\"
        if ! clang-tidy \"\$file\" -p . --quiet; then
            echo -e \"${RED}Clang-tidy issues found in \$file${NC}\"
            exit 1
        fi
    done"
else
    echo -e "${RED}Error: compile_commands.json not found${NC}"
    exit 1
fi

# Run cppcheck with more detailed output
run_tool "cppcheck" "cppcheck --enable=all \
                              --suppress=missingIncludeSystem \
                              --error-exitcode=1 \
                              --inline-suppr \
                              --template='{file}:{line}: {severity}: {message}' \
                              --std=c++17 \
                              --language=c++ \
                              --verbose \
                              ../$SRC_DIR ../$TEST_DIR"

# Run cpplint with custom filters (non-fatal)
run_tool "cpplint" "find ../$SRC_DIR ../$TEST_DIR -name '*.cpp' -o -name '*.h' | \
    xargs cpplint --filter=-legal/copyright,-build/include_subdir,+build/include_order" false

# Build and run tests with coverage
echo -e "\n${YELLOW}Building and running tests with coverage...${NC}"
if ! cmake --build . --target coverage; then
    echo -e "${RED}Failed to build or run tests${NC}"
    exit 1
fi

# Generate and check coverage report
echo -e "\n${YELLOW}Generating coverage report...${NC}"
if ! gcovr -r .. --html --html-details -o coverage/index.html; then
    echo -e "${RED}Failed to generate coverage report${NC}"
    exit 1
fi

# Check coverage percentage
coverage_percentage=$(gcovr -r .. | grep TOTAL | awk '{print $4}' | sed 's/%//')
if (( $(echo "$coverage_percentage < 80" | bc -l) )); then
    echo -e "${RED}Coverage below 80%: $coverage_percentage%${NC}"
    exit 1
fi

echo -e "\n${GREEN}All critical checks passed successfully!${NC}"
echo -e "${GREEN}Coverage: $coverage_percentage%${NC}" 