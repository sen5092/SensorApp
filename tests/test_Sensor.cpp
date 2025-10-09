#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <nlohmann/json.hpp>
#include <atomic>
#include <thread>
#include <memory>
#include <string>
#include <unordered_map>

#include "Sensor.hpp"
#include "HardwareDataSource.hpp"
#include "MockCamera.hpp"
#include "ITransport.hpp"
#include "ConfigTypes.hpp"
#include "Logger.hpp"

using json = nlohmann::json;
using Catch::Matchers::WithinAbs;

//
// ─── DUMMY TRANSPORT IMPLEMENTATION ─────────────────────────────────────────────
//
class DummyTransport : public ITransport {
public:
    bool connected = false;
    std::string lastSent;

    void connect() override { connected = true; }
    void close() noexcept override { connected = false; }

    // ✅ Return type now matches real ITransport
    std::size_t sendString(const std::string& data) override {
        lastSent = data;
        return data.size();
    }

    bool isConnected() const override { return connected; }
};


//
// ─── SENSOR TESTS ───────────────────────────────────────────────────────────────
//
TEST_CASE("Sensor constructor validates configuration", "[Sensor]") {
    SensorConfig cfg{};
    cfg.sensorId = "";
    cfg.intervalSeconds = 1;
    cfg.metadata = {};  // map, not string

    std::shared_ptr<ICamera> camera = std::make_shared<MockCamera>();
    auto ds = std::make_unique<HardwareDataSource>(camera);
    auto tx = std::make_unique<DummyTransport>();

    REQUIRE_THROWS_AS(Sensor(cfg, std::move(ds), std::move(tx)), std::invalid_argument);

    cfg.sensorId = "unit_sensor";
    cfg.intervalSeconds = 0;
    cfg.metadata = {};
    camera = std::make_shared<MockCamera>();
    ds = std::make_unique<HardwareDataSource>(camera);
    tx = std::make_unique<DummyTransport>();

    REQUIRE_THROWS_AS(Sensor(cfg, std::move(ds), std::move(tx)), std::invalid_argument);
}

TEST_CASE("Sensor runOnce builds a valid JSON payload", "[Sensor]") {
    SensorConfig cfg;
    cfg.sensorId = "mock_sensor";
    cfg.intervalSeconds = 1;
    cfg.metadata = {{"environment", "unit-test"}};
    cfg.units = {
        {"frame_width", "px"},
        {"brightness", "intensity"}
    };

    std::shared_ptr<ICamera> camera = std::make_shared<MockCamera>();
    camera->open(0);  // Ensure camera is "opened" for the test
    auto ds = std::make_unique<HardwareDataSource>(camera);
    auto tx = std::make_unique<DummyTransport>();
    DummyTransport* txPtr = tx.get();

    Sensor sensor(cfg, std::move(ds), std::move(tx));
    sensor.runOnce();

    REQUIRE_FALSE(txPtr->lastSent.empty());

    json payload = json::parse(txPtr->lastSent);
    REQUIRE(payload["sensor_id"] == "mock_sensor");
    REQUIRE(payload["metadata"]["environment"] == "unit-test");
    REQUIRE(payload.contains("timestamp_ms"));
    REQUIRE(payload.contains("readings"));

    REQUIRE(payload["readings"].contains("frame_width"));
    REQUIRE(payload["readings"]["frame_width"]["unit"] == "px");
    REQUIRE(payload["readings"]["brightness"]["unit"] == "intensity");

    // Numeric values roughly valid
    REQUIRE_THAT(payload["readings"]["frame_status"]["value"].get<double>(), WithinAbs(1.0, 0.0));
    REQUIRE_THAT(payload["readings"]["frame_width"]["value"].get<double>(), WithinAbs(640.0, 1.0));
    REQUIRE_THAT(payload["readings"]["frame_height"]["value"].get<double>(), WithinAbs(480.0, 1.0));
}

TEST_CASE("Sensor connect and close update transport state", "[Sensor]") {
    SensorConfig cfg;
    cfg.sensorId = "sensor_test";
    cfg.intervalSeconds = 1;
    cfg.metadata = {};
    cfg.units = {};

    std::shared_ptr<ICamera> camera = std::make_shared<MockCamera>();
    auto ds = std::make_unique<HardwareDataSource>(camera);
    auto tx = std::make_unique<DummyTransport>();
    DummyTransport* txPtr = tx.get();

    Sensor sensor(cfg, std::move(ds), std::move(tx));

    sensor.connect();
    REQUIRE(txPtr->connected);
    REQUIRE(txPtr->isConnected());

    sensor.close();
    REQUIRE_FALSE(txPtr->connected);
    REQUIRE_FALSE(txPtr->isConnected());
}

TEST_CASE("Sensor run loop executes and stops cleanly", "[Sensor]") {
    SensorConfig cfg;
    cfg.sensorId = "loop_sensor";
    cfg.intervalSeconds = 1;
    cfg.metadata = {};
    cfg.units = {};

    std::shared_ptr<ICamera> camera = std::make_shared<MockCamera>();
    auto ds = std::make_unique<HardwareDataSource>(camera);
    auto tx = std::make_unique<DummyTransport>();
    DummyTransport* txPtr = tx.get();

    Sensor sensor(cfg, std::move(ds), std::move(tx));

    std::atomic<bool> running{true};
    std::thread worker([&] { sensor.run(running); });

    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    running = false;
    worker.join();

    REQUIRE_FALSE(txPtr->lastSent.empty());
}
