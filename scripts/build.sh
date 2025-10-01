#!/bin/bash

# Exit error
set -e

# Default flags
REBUILD=false
COVERAGE=true

# Parse arguments
for arg in "$@"; do
  case $arg in
    --rebuild)
      REBUILD=true
      ;;
    --no-coverage)
      COVERAGE=false
      ;;
    *)
      echo "Unknown option: $arg"
      exit 1
      ;;
  esac
done

BUILD_DIR="build"

# Clean build directory if --rebuild is specified
if [ "$REBUILD" = true ]; then
  echo "Performing full rebuild..."
  rm -rf "$BUILD_DIR"
fi

# Ensure build directory exists
mkdir -p "$BUILD_DIR"

# Compiler settings
CXX_COMPILER="clang++"
CMAKE_FLAGS="-DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=$CXX_COMPILER"
CMAKE_FLAGS+=" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"

if [ "$COVERAGE" = true ]; then
  COVERAGE_OPTS="--coverage -fprofile-instr-generate -fcoverage-mapping"
  CMAKE_FLAGS+=" -DCMAKE_CXX_FLAGS=\"${COVERAGE_OPTS}\""
  CMAKE_FLAGS+=" -DCMAKE_EXE_LINKER_FLAGS=--coverage"
  CMAKE_FLAGS+=" -DCMAKE_SHARED_LINKER_FLAGS=--coverage"
  CMAKE_FLAGS+=" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
fi


echo "Configuring and building project..."
eval cmake -B "$BUILD_DIR" -S . $CMAKE_FLAGS
cmake --build "$BUILD_DIR"
echo "Build complete in $BUILD_DIR/"

