// ConfigLoader.cpp
#include "ConfigLoader.hpp"

#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <iostream>

using json = nlohmann::json;

// --- small helper: read a JSON file into a json object ---
namespace {
    json readJsonFile(const std::string& path) {
        std::ifstream in(path);
        if (!in.is_open()) {
            throw std::runtime_error("ConfigLoader: cannot open file: " + path);
        }
        json j;
        in >> j;
        return j;
    }

    // helper to read a string->string map if the field exists
    static void readStringMapIfPresent(const json& j, const char* key,
                                       std::unordered_map<std::string, std::string>& out) {
        if (j.contains(key)) {
            const auto& obj = j.at(key);
            if (!obj.is_object()) {
                throw std::runtime_error(std::string("ConfigLoader: '") + key + "' must be an object");
            }
            out.clear();
            for (const auto& [k, v] : obj.items()) {
                if (!v.is_string()) {
                    throw std::runtime_error(std::string("ConfigLoader: '") + key + "." + k + "' must be string");
                }
                out.emplace(k, v.get<std::string>());
            }
        }
    }
}

// ---------------- Sensor ----------------

SensorConfig ConfigLoader::loadSensorConfig(const std::string& path) const {
    const json j = readJsonFile(path);

    SensorConfig cfg;

    // sensor_id (required, string)
    if (!j.contains("sensor_id") || !j["sensor_id"].is_string()) {
        throw std::runtime_error("SensorConfig: missing or invalid 'sensor_id' in " + path);
    }
    cfg.sensorId = j["sensor_id"].get<std::string>();

    // interval_seconds (optional int, default 1)
    if (j.contains("interval_seconds")) {
        if (!j["interval_seconds"].is_number_integer()) {
            throw std::runtime_error("SensorConfig: 'interval_seconds' must be an integer in " + path);
        }
        cfg.intervalSeconds = j["interval_seconds"].get<int32_t>();
    } else {
        cfg.intervalSeconds = 1;
    }

    // Optional maps
    readStringMapIfPresent(j, "units", cfg.units);
    readStringMapIfPresent(j, "metadata", cfg.metadata);

    return cfg;
}

// ---------------- Transport ----------------

TransportConfig ConfigLoader::loadTransportConfig(const std::string& path) const {
    const json j = readJsonFile(path);

    if (!j.contains("kind") || !j["kind"].is_string()) {
        throw std::runtime_error("TransportConfig: missing or invalid 'kind' in " + path);
    }

    TransportConfig cfg;
    cfg.kind = j["kind"].get<std::string>();

    if (cfg.kind == "tcp") {
        if (!j.contains("tcp") || !j["tcp"].is_object()) {
            throw std::runtime_error("TransportConfig: missing 'tcp' object for kind='tcp' in " + path);
        }
        const auto& tcp = j["tcp"];

        if (!tcp.contains("host") || !tcp["host"].is_string()) {
            throw std::runtime_error("TransportConfig: missing or invalid 'tcp.host' in " + path);
        }
        if (!tcp.contains("port") || !tcp["port"].is_number_integer()) {
            throw std::runtime_error("TransportConfig: missing or invalid 'tcp.port' in " + path);
        }

        cfg.host = tcp["host"].get<std::string>();
        cfg.port = tcp["port"].get<int32_t>();

        if (cfg.port <= 0 || cfg.port > 65535) {
            throw std::runtime_error("TransportConfig: 'tcp.port' out of range (1..65535) in " + path);
        }
    } else {
        throw std::runtime_error("TransportConfig: unsupported kind '" + cfg.kind + "' in " + path);
    }

    return cfg;
}
