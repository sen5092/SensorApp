#!/usr/bin/env bash
set -euo pipefail

# -------- config (Homebrew LLVM path; change if yours differs) -------------
LLVM_BIN="/opt/homebrew/opt/llvm/bin"
CC="$LLVM_BIN/clang"
CXX="$LLVM_BIN/clang++"
PROFDATA="$LLVM_BIN/llvm-profdata"
LLVMCOV="$LLVM_BIN/llvm-cov"

BUILD_DIR="build-coverage"
ABS_BUILD_DIR="$(pwd)/$BUILD_DIR"
REPORT_DIR="$ABS_BUILD_DIR/coverage-report"
SRC_DIRS="$(pwd)/src"

# -------- 1) Configure & build with coverage flags -------------------------
echo "==> Configuring (coverage ON) into $BUILD_DIR"
rm -rf "$BUILD_DIR"
cmake -S . -B "$BUILD_DIR" \
  -G Ninja \
  -DCMAKE_C_COMPILER="$CC" \
  -DCMAKE_CXX_COMPILER="$CXX" \
  -DENABLE_COVERAGE=ON
cmake --build "$BUILD_DIR" -j$( (sysctl -n hw.ncpu 2>/dev/null) || echo 4 )

# -------- 2) Run tests with absolute LLVM_PROFILE_FILE ----------------------
echo "==> Running tests (writing profiles to $ABS_BUILD_DIR)"
export LLVM_PROFILE_FILE="$ABS_BUILD_DIR/coverage_%p.profraw"
CTEST_OUTPUT_ON_FAILURE=1 ctest --test-dir "$ABS_BUILD_DIR"

# -------- 3) Merge raw profiles --------------------------------------------
echo "==> Merging raw profiles"
RAW_FILES=$(find "$ABS_BUILD_DIR" -type f \( -name 'coverage_*.profraw' -o -name 'default.profraw' \))
if [ -z "$RAW_FILES" ]; then
  echo "ERROR: No .profraw files found under $ABS_BUILD_DIR"
  exit 1
fi
OUT_PROFILE="$ABS_BUILD_DIR/coverage.profdata"
$PROFDATA merge -sparse $RAW_FILES -o "$OUT_PROFILE"

# -------- 4) Generate reports ----------------------------------------------
echo "==> Generating coverage reports"
mkdir -p "$REPORT_DIR"

# Console summary
$LLVMCOV report "$ABS_BUILD_DIR/tests/SensorTests" \
  -instr-profile="$OUT_PROFILE" \
  -ignore-filename-regex='external/.*' \
  $SRC_DIRS

# HTML report
$LLVMCOV show "$ABS_BUILD_DIR/tests/SensorTests" \
  -instr-profile="$OUT_PROFILE" \
  -format=html -output-dir="$REPORT_DIR" \
  -ignore-filename-regex='external/.*' \
  $SRC_DIRS

echo
echo "Coverage report: $REPORT_DIR/index.html"
