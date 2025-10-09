#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <opencv2/core.hpp>
#include "HardwareDataSource.hpp"
#include "MockCamera.hpp"
#include "Logger.hpp"

TEST_CASE("HardwareDataSource happy path using MockCamera", "[HardwareDataSource]") {
    std::shared_ptr<ICamera> camera = std::make_shared<MockCamera>();
    camera->open(0);

    HardwareDataSource ds(camera);

    auto values = ds.readAll();

    REQUIRE_THAT(values["frame_status"], Catch::Matchers::WithinAbs(1.0, 0));
    REQUIRE_THAT(values["frame_width"], Catch::Matchers::WithinAbs(640.0, 0));
    REQUIRE_THAT(values["frame_height"], Catch::Matchers::WithinAbs(480.0, 0));
    REQUIRE_THAT(values["channels"], Catch::Matchers::WithinAbs(3.0, 0));
    REQUIRE_THAT(values["brightness"], Catch::Matchers::WithinAbs(20.0, 0));
}

TEST_CASE("HardwareDataSource handles camera not opened", "[HardwareDataSource]") {
    std::shared_ptr<ICamera> camera = std::make_shared<MockCamera>(); // not opened
    HardwareDataSource ds(camera);

    auto values = ds.readAll();
    REQUIRE_THAT(values["frame_status"], Catch::Matchers::WithinAbs(0.0, 0));
}