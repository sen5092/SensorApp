# Build shared library with reusable code
set(APP_SOURCES
    TcpSocket.cpp
    ConfigLoader.cpp
    Sensor.cpp
    TransportFactory.cpp
    UdpSocket.cpp
)

set(CONFIG_FILES
    sensor_config.json
    transport_config.json
)

if(SIMULATION)
    message(STATUS "Building with SimulationDataSource")
    add_compile_definitions(SIMULATION)
    list(APPEND APP_SOURCES
        SimulationDataSource.cpp
    )

    list(APPEND CONFIG_FILES
        simulation_datasource_config.json
    )
else()
    message(STATUS "Building with HardwareDataSource")
    list(APPEND APP_SOURCES
        HardwareDataSource.cpp
    )
endif()


if (USE_OPENCV)
    message(STATUS "Building with OpenCV support")
    # If CMake can’t find OpenCV automatically, pass -DOpenCV_DIR at configure time:
    #   -DOpenCV_DIR=/opt/homebrew/opt/opencv/lib/cmake/opencv4
    find_package(OpenCV REQUIRED)

    message(STATUS "Found OpenCV ${OpenCV_VERSION}")

    list(APPEND APP_SOURCES
        HardwareDataSource.cpp
    )

    add_compile_definitions(USE_OPENCV)

else()
    message(STATUS "Building without OpenCV")
    list(APPEND APP_SOURCES
        SimulationDataSource.cpp
    )

    list(APPEND CONFIG_FILES
        simulation_datasource_config.json
    )

endif()


add_library(SensorLib ${APP_SOURCES})

target_include_directories(SensorLib PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(SensorLib PUBLIC nlohmann_json::nlohmann_json)

if (USE_OPENCV)
  # Link OpenCV into SensorLib (so code inside your lib can use it)
  target_include_directories(SensorLib PUBLIC ${OpenCV_INCLUDE_DIRS})
  target_link_libraries(SensorLib PRIVATE ${OpenCV_LIBS})
endif()



# Main executable (depends on PracticeLib)
add_executable(Sensor main.cpp)
target_link_libraries(Sensor PRIVATE SensorLib)
target_include_directories(Sensor PRIVATE ${PROJECT_SOURCE_DIR}/include)



# Put only the relevant configs next to the built executable
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/config)
foreach(name ${CONFIG_FILES})
  configure_file(
    ${CMAKE_SOURCE_DIR}/config/${name}
    ${CMAKE_CURRENT_BINARY_DIR}/config/${name}
    COPYONLY
  )
endforeach()
