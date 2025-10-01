#!/usr/bin/env bash

set -Eeuo pipefail

# Default options
RUN_TIDY=false
RUN_CSA=false
BUILD_DIR="build"
REPORT_DIR="build/reports"

# Create output directory
mkdir -p "$REPORT_DIR"

# Parse arguments
for arg in "$@"; do
  case $arg in
    --tidy)
      RUN_TIDY=true
      shift
      ;;
    --csa)
      RUN_CSA=true
      shift
      ;;
    -h|--help)
      echo "Usage: $0 [--tidy] [--csa]"
      exit 0
      ;;
    *)
      echo "Unknown option: $arg"
      exit 1
      ;;
  esac
done

# Run clang-tidy
if [ "$RUN_TIDY" = true ]; then
  echo "Running clang-tidy..."

  # Only .cpp files; headers will still be checked via includes
  FILES=$(find src \( -name '*.cpp' \))

  if [ -n "$FILES" ]; then
    echo "Found $(echo "$FILES" | wc -l | tr -d ' ') files to analyze..."

    if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
      echo "Warning: $BUILD_DIR/compile_commands.json not found; results may be incomplete." >&2
    fi

    # Overwrite the report; capture both stdout and stderr
    clang-tidy $FILES -p "$BUILD_DIR" -extra-arg=-fno-color-diagnostics \
      > "$REPORT_DIR/clang-tidy.txt" 2>&1

    echo "Clang-tidy complete. Report saved to $REPORT_DIR/clang-tidy.txt"
  else
    echo "No .cpp files found in src/"
  fi
fi

# Run analyze-build (Clang Static Analyzer)
if [ "$RUN_CSA" = true ]; then
  echo "Running Clang Static Analyzer..."

  if ! command -v analyze-build >/dev/null 2>&1; then
    echo "Error: analyze-build not found in PATH." >&2
    exit 1
  fi
  if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
    echo "Error: $BUILD_DIR/compile_commands.json not found. Configure/build first." >&2
    exit 1
  fi

  analyze-build \
    --cdb "$BUILD_DIR/compile_commands.json" \
    --use-analyzer "$(which clang)" \
    --output "$REPORT_DIR"

  echo "CSA complete. Report saved to $REPORT_DIR/scan-build-<datetime>/index.html"
fi

if [ "$RUN_TIDY" = false ] && [ "$RUN_CSA" = false ]; then
  echo "No analysis type specified. Use --tidy and/or --csa"
  exit 1
fi
