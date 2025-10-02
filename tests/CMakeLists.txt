file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS *.cpp)

add_executable(SensorTests ${TEST_FILES})

target_link_libraries(SensorTests
    PRIVATE
        SensorLib
        Catch2::Catch2WithMain
)

# Disable warnings for test files (often use things that trigger warnings)
# We want to focus on warnings in the library code, not tests.
# Note: You might want to adjust this if you want stricter checks in tests too.
set_target_properties(SensorTests PROPERTIES
    COMPILE_OPTIONS ""
    LINK_OPTIONS ""
)


include(Catch)
catch_discover_tests(SensorTests)
