#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include "ConfigLoader.hpp"
#include <fstream>
#include <cstdio>

// RAII wrapper for a temporary JSON file
struct TempJsonFile {
    std::string path;
    TempJsonFile(const std::string& name, const std::string& contents) : path(name) {
        std::ofstream out(path);
        out << contents;
    }
    ~TempJsonFile() {
        std::remove(path.c_str()); // cleanup on destruction
    }
};

// ---------------- SensorConfig tests ----------------

TEST_CASE("SensorConfig loads valid minimal config", "[ConfigLoader]") {
    TempJsonFile tmp("sensor_valid.json", R"({ "sensor_id": "sensor123" })");

    auto cfg = ConfigLoader::loadSensorConfig(tmp.path);
    REQUIRE(cfg.sensorId == "sensor123");
    REQUIRE(cfg.intervalSeconds == 1); // default
    REQUIRE(cfg.units.empty());
    REQUIRE(cfg.metadata.empty());
}

TEST_CASE("SensorConfig with custom interval and maps", "[ConfigLoader]") {
    TempJsonFile tmp("sensor_with_maps.json", R"({
        "sensor_id": "abc",
        "interval_seconds": 5,
        "units": { "temp": "C" },
        "metadata": { "loc": "lab" }
    })");

    auto cfg = ConfigLoader::loadSensorConfig(tmp.path);
    REQUIRE(cfg.sensorId == "abc");
    REQUIRE(cfg.intervalSeconds == 5);
    REQUIRE(cfg.units.at("temp") == "C");
    REQUIRE(cfg.metadata.at("loc") == "lab");
}

TEST_CASE("SensorConfig missing sensor_id throws", "[ConfigLoader]") {
    TempJsonFile tmp("sensor_missing.json", R"({ })");
    REQUIRE_THROWS_AS(ConfigLoader::loadSensorConfig(tmp.path), std::runtime_error);
}

TEST_CASE("SensorConfig invalid interval type throws", "[ConfigLoader]") {
    TempJsonFile tmp("sensor_invalid_interval.json", R"({
        "sensor_id": "id1",
        "interval_seconds": "oops"
    })");
    REQUIRE_THROWS_AS(ConfigLoader::loadSensorConfig(tmp.path), std::runtime_error);
}

TEST_CASE("SensorConfig invalid map value throws", "[ConfigLoader]") {
    TempJsonFile tmp("sensor_invalid_map.json", R"({
        "sensor_id": "id2",
        "units": { "temp": 123 }
    })");
    REQUIRE_THROWS_AS(ConfigLoader::loadSensorConfig(tmp.path), std::runtime_error);
}

TEST_CASE("SensorConfig interval is zero throws", "[ConfigLoader]") {
    TempJsonFile tmp("sensor_zero_interval.json", R"({
        "sensor_id": "id0",
        "interval_seconds": 0
    })");
    REQUIRE_THROWS_AS(ConfigLoader::loadSensorConfig(tmp.path), std::runtime_error);
}

TEST_CASE("SensorConfig interval is negative throws", "[ConfigLoader]") {
    TempJsonFile tmp("sensor_negative_interval.json", R"({
        "sensor_id": "idNeg",
        "interval_seconds": -5
    })");
    REQUIRE_THROWS_AS(ConfigLoader::loadSensorConfig(tmp.path), std::runtime_error);
}

TEST_CASE("SensorConfig units present but not object throws", "[ConfigLoader]") {
    TempJsonFile tmp("sensor_units_not_obj.json", R"({
        "sensor_id": "idUnits",
        "units": "should_be_object"
    })");
    REQUIRE_THROWS_AS(ConfigLoader::loadSensorConfig(tmp.path), std::runtime_error);
}

TEST_CASE("SensorConfig metadata present but not object throws", "[ConfigLoader]") {
    TempJsonFile tmp("sensor_metadata_not_obj.json", R"({
        "sensor_id": "idMeta",
        "metadata": "oops"
    })");
    REQUIRE_THROWS_AS(ConfigLoader::loadSensorConfig(tmp.path), std::runtime_error);
}

// ---------------- TransportConfig tests ----------------

TEST_CASE("TransportConfig loads valid TCP config", "[ConfigLoader]") {
    TempJsonFile tmp("tcp_valid.json", R"({
        "kind": "tcp",
        "tcp": { "host": "localhost", "port": 8080 }
    })");

    auto cfg = ConfigLoader::loadTransportConfig(tmp.path);
    REQUIRE(cfg.kind == "tcp");
    REQUIRE(cfg.host == "localhost");
    REQUIRE(cfg.port == 8080);
}

TEST_CASE("TransportConfig loads valid UDP config", "[ConfigLoader]") {
    TempJsonFile tmp("udp_valid.json", R"({
        "kind": "udp",
        "udp": { "host": "127.0.0.1", "port": 5000 }
    })");

    auto cfg = ConfigLoader::loadTransportConfig(tmp.path);
    REQUIRE(cfg.kind == "udp");
    REQUIRE(cfg.host == "127.0.0.1");
    REQUIRE(cfg.port == 5000);
}

TEST_CASE("TransportConfig missing kind throws", "[ConfigLoader]") {
    TempJsonFile tmp("missing_kind.json", R"({ })");
    REQUIRE_THROWS_AS(ConfigLoader::loadTransportConfig(tmp.path), std::runtime_error);
}

TEST_CASE("TransportConfig unsupported kind throws", "[ConfigLoader]") {
    TempJsonFile tmp("bad_kind.json", R"({ "kind": "serial" })");
    REQUIRE_THROWS_AS(ConfigLoader::loadTransportConfig(tmp.path), std::runtime_error);
}

TEST_CASE("TransportConfig tcp missing host throws", "[ConfigLoader]") {
    TempJsonFile tmp("tcp_no_host.json", R"({
        "kind": "tcp",
        "tcp": { "port": 1234 }
    })");
    REQUIRE_THROWS_AS(ConfigLoader::loadTransportConfig(tmp.path), std::runtime_error);
}

TEST_CASE("TransportConfig tcp missing port throws", "[ConfigLoader]") {
    TempJsonFile tmp("tcp_no_port.json", R"({
        "kind": "tcp",
        "tcp": { "host": "localhost" }
    })");
    REQUIRE_THROWS_AS(ConfigLoader::loadTransportConfig(tmp.path), std::runtime_error);
}

TEST_CASE("TransportConfig tcp invalid port type throws", "[ConfigLoader]") {
    TempJsonFile tmp("tcp_bad_port.json", R"({
        "kind": "tcp",
        "tcp": { "host": "localhost", "port": "not_a_number" }
    })");
    REQUIRE_THROWS_AS(ConfigLoader::loadTransportConfig(tmp.path), std::runtime_error);
}

TEST_CASE("TransportConfig TCP port out of range throws", "[ConfigLoader]") {
    TempJsonFile tmp("tcp_port_oob.json", R"({
        "kind": "tcp",
        "tcp": { "host": "localhost", "port": 70000 }
    })");
    REQUIRE_THROWS_AS(ConfigLoader::loadTransportConfig(tmp.path), std::runtime_error);
}

TEST_CASE("TransportConfig udp host missing throws", "[ConfigLoader]") {
    TempJsonFile tmp("udp_no_host.json", R"({
        "kind": "udp",
        "udp": { "port": 5000 }
    })");
    REQUIRE_THROWS_AS(ConfigLoader::loadTransportConfig(tmp.path), std::runtime_error);
}

TEST_CASE("TransportConfig udp host not object throws", "[ConfigLoader]") {
    TempJsonFile tmp("udp_not_object.json", R"({
        "kind": "udp",
        "udp": 123
    })");
    REQUIRE_THROWS_AS(ConfigLoader::loadTransportConfig(tmp.path), std::runtime_error);
}

TEST_CASE("TransportConfig udp missing port throws", "[ConfigLoader]") {
    TempJsonFile tmp("udp_no_port.json", R"({
        "kind": "udp",
        "udp": { "host": "127.0.0.1" }
    })");
    REQUIRE_THROWS_AS(ConfigLoader::loadTransportConfig(tmp.path), std::runtime_error);
}

TEST_CASE("TransportConfig udp invalid port type throws", "[ConfigLoader]") {
    TempJsonFile tmp("udp_bad_port_type.json", R"({
        "kind": "udp",
        "udp": { "host": "127.0.0.1", "port": "not_a_number" }
    })");
    REQUIRE_THROWS_AS(ConfigLoader::loadTransportConfig(tmp.path), std::runtime_error);
}

TEST_CASE("TransportConfig udp port out of range throws", "[ConfigLoader]") {
    TempJsonFile tmp("udp_bad_port.json", R"({
        "kind": "udp",
        "udp": { "host": "127.0.0.1", "port": 70000 }
    })");
    REQUIRE_THROWS_AS(ConfigLoader::loadTransportConfig(tmp.path), std::runtime_error);
}

// ---------------- File open error ----------------

TEST_CASE("ConfigLoader load non-existent file throws", "[ConfigLoader]") {
    REQUIRE_THROWS_AS(ConfigLoader::loadSensorConfig("no_such_file.json"), std::runtime_error);
    REQUIRE_THROWS_AS(ConfigLoader::loadTransportConfig("no_such_file.json"), std::runtime_error);
}
