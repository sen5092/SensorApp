file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS *.cpp)

add_executable(SensorTests ${TEST_FILES})

target_link_libraries(SensorTests
    PRIVATE
        SensorLib
        Catch2::Catch2WithMain
        nlohmann_json::nlohmann_json
)

target_compile_definitions(SensorTests PRIVATE ENABLE_MOCK_CAMERA=1)
enable_coverage(SensorTests)

include(Catch)
catch_discover_tests(SensorTests)
