# Clang tidy should only run on the library and main app, not tests, so we place it in this scope.
set(CMAKE_CXX_CLANG_TIDY
    clang-tidy;
    -quiet
)

# Build shared library with reusable code
set(APP_SOURCES
    ConfigLoader.cpp
    HardwareDataSource.cpp
    Sensor.cpp
    TcpSocket.cpp
    TransportFactory.cpp
    UdpSocket.cpp
)

set(CONFIG_FILES
    sensor_config.json
    transport_config.json
)

# If CMake can’t find OpenCV automatically, pass -DOpenCV_DIR at configure time:
#   -DOpenCV_DIR=/opt/homebrew/opt/opencv/lib/cmake/opencv4
find_package(OpenCV REQUIRED)
message(STATUS "Found OpenCV ${OpenCV_VERSION}")


# ------------------------------------------------------------------------------
# Camera selection
# ------------------------------------------------------------------------------
if(ENABLE_MOCK_CAMERA)
    message(STATUS "Building with MockCamera")
    add_compile_definitions(USE_MOCK_CAMERA)
else()
    message(STATUS "Building with HardwareCamera")
endif()



# ------------------------------------------------------------------------------
# Library target
# ------------------------------------------------------------------------------

# Set up the SensorLib library target
add_library(SensorLib ${APP_SOURCES})
target_include_directories(SensorLib PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Link OpenCV into SensorLib (so code inside your lib can use it)
target_include_directories(SensorLib SYSTEM PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(SensorLib PRIVATE ${OpenCV_LIBS})
target_link_libraries(SensorLib PRIVATE nlohmann_json::nlohmann_json)
enable_strict_warnings(SensorLib)
enable_sanitizers(SensorLib)
enable_coverage(SensorLib)


# ------------------------------------------------------------------------------
# Main executable
# ------------------------------------------------------------------------------
add_executable(Sensor main.cpp)
target_link_libraries(Sensor PRIVATE SensorLib)
enable_strict_warnings(Sensor)
enable_sanitizers(Sensor)
enable_coverage(Sensor)

# Put only the relevant configs next to the built executable
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/config)
foreach(name ${CONFIG_FILES})
  configure_file(
    ${CMAKE_SOURCE_DIR}/config/${name}
    ${CMAKE_CURRENT_BINARY_DIR}/config/${name}
    COPYONLY
  )
endforeach()
