#!/bin/bash

set -e  # Exit on any error

BUILD_DIR="build/tests"
TEST_BINARY="${BUILD_DIR}/PracticeTests"
RAW_PROFILE="${BUILD_DIR}/coverage.profraw"
MERGED_PROFILE="${BUILD_DIR}/coverage.profdata"
COVERAGE_TXT="${BUILD_DIR}/coverage.txt"

# Run the tests with profiling
echo "Running tests with coverage instrumentation..."
LLVM_PROFILE_FILE="$RAW_PROFILE" "$TEST_BINARY"

# Merge the raw profile data
echo "Merging profile data..."
llvm-profdata merge -sparse "$RAW_PROFILE" -o "$MERGED_PROFILE"

# Generate annotated coverage output
echo "Generating line-by-line coverage report..."
llvm-cov show "$TEST_BINARY" \
  -instr-profile="$MERGED_PROFILE" \
  -format=text \
  -ignore-filename-regex='.*/external|tests/.*' \
  > "$COVERAGE_TXT"

# Generate summary
echo ""
echo "Coverage Summary:"
llvm-cov report "$TEST_BINARY" -instr-profile="$MERGED_PROFILE" -ignore-filename-regex='.*/external|tests/.*'

echo ""
echo "Detailed report saved to: $COVERAGE_TXT"
