file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS *.cpp)

add_executable(SensorTests ${TEST_FILES})

target_link_libraries(SensorTests
    PRIVATE
        SensorLib
        Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(SensorTests)
