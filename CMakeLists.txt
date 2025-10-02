cmake_minimum_required(VERSION 3.16)

# Project metadata
project(SensorApp
    VERSION 1.0
    DESCRIPTION "Sensor project for processing and transmitting laptop webcam frame data."
    LANGUAGES CXX
)

# Default C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default Debug build
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")

# Export compile_commands.json (for clangd, clang-tidy, etc.)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -----------------
# Options (User Toggles)
# -----------------
option(USE_OPENCV "Enable OpenCV features" ON)

set(CMAKE_CXX_CLANG_TIDY
    clang-tidy;
    -header-filter=^${CMAKE_SOURCE_DIR}/include;
    -header-filter=^${CMAKE_SOURCE_DIR}/src;
)

# -----------------
# Helper Functions
# -----------------

# A helper function to enable strict warnings for a target
function(enable_strict_warnings target_name)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
        target_compile_options(${target_name} PRIVATE
            -Wall               # Most common warnings
            -Wextra             # More warnings
            -Wpedantic          # Standards compliance
            -Wshadow            # Warn if a variable hides another
            -Wconversion        # Warn on type conversions
            -Wsign-conversion   # Signed/unsigned mismatches
        )
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        target_compile_options(${target_name} PRIVATE
            /W4   # high warnings level on MSVC
        )
    endif()
endfunction()

# Sanitizers (Debug builds only)
function(enable_sanitizers target_name)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug"
        AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")

        target_compile_options(${target_name} PRIVATE
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
        )

        target_link_options(${target_name} PRIVATE
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
        )
    endif()
endfunction()


# -----------------
# Dependencies
# -----------------
include(FetchContent)

set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/external")
set(FETCHCONTENT_QUIET OFF)

# Catch2
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.6.0
)
FetchContent_MakeAvailable(Catch2)

# nlohmann/json
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)


# -----------------
# Subdirectories
# -----------------
add_subdirectory(src)

enable_testing()
add_subdirectory(tests)
