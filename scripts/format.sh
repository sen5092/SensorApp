#!/bin/bash

# Directories to search
DIRS="src include"

# File extensions to format (properly escaped for bash)
EXTENSIONS=( -name "*.cpp" -o -name "*.hpp" )

# Dry-run mode?
if [[ "$1" == "--check" ]]; then
    echo "Checking formatting (dry-run)..."
    find $DIRS -type f \( "${EXTENSIONS[@]}" \) | while read -r file; do
        if ! clang-format --style=file "$file" | diff -u --color=always "$file" - > /dev/null; then
            echo "Formatting issue in: $file"
            clang-format --style=file "$file" | diff -u --color=always "$file" -
        fi
    done
    echo "Done checking formatting!"

else
    echo "Formatting files in-place..."
    find $DIRS -type f \( "${EXTENSIONS[@]}" \) | while read -r file; do
        echo "Formatting $file"
        clang-format -i --style=file "$file"
    done
    echo "Done formatting!"
fi
