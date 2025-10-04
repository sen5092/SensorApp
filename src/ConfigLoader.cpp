/**
 * @file ConfigLoader.cpp
 * @brief Implements ConfigLoader for reading and validating JSON configs.
 *
 * Contains the method definitions for parsing JSON fields and enforcing
 * general constraints such as valid port ranges.
 */

#include "ConfigLoader.hpp"
#include "ConfigTypes.hpp"
#include "NetworkConstants.hpp"

#include "Logger.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <cstdint>
#include <stdexcept>
#include <unordered_map>

using json = nlohmann::json;

// --- small helper: read a JSON file into a json object ---
namespace {

    json readJsonFile(const std::string& path) {

        std::ifstream input(path);
        if (!input.is_open()) {
            throw std::runtime_error("ConfigLoader: cannot open file: " + path);
        }

        Logger::instance().debug("Reading from JSON file at " + path);

        json jsonObject;
        input >> jsonObject;
        return jsonObject;
    }

    // Helper to read a string->string map if the field exists
    void readStringMapIfPresent(
        const json& jsonConfig,
        const char* fieldName,
        std::unordered_map<std::string, std::string>& outputMap)
    {
        if (jsonConfig.contains(fieldName)) {
            const auto& fieldValue = jsonConfig.at(fieldName);
            if (!fieldValue.is_object()) {
                throw std::runtime_error(
                    std::string("ConfigLoader: '") + fieldName + "' must be an object");
            }

            outputMap.clear();
            for (const auto& [entryKey, entryValue] : fieldValue.items()) {
                if (!entryValue.is_string()) {
                    throw std::runtime_error(
                        std::string("ConfigLoader: '") + fieldName + "." + entryKey +
                        "' must be a string");
                }
                outputMap.emplace(entryKey, entryValue.get<std::string>());
            }
        }
    }

} // namespace

// ---------------- Sensor ----------------

SensorConfig ConfigLoader::loadSensorConfig(const std::string& path) {
    const json jsonObject = readJsonFile(path);

    SensorConfig cfg;

    // sensor_id (required, string)
    if (!jsonObject.contains("sensor_id") || !jsonObject["sensor_id"].is_string()) {
        throw std::runtime_error("SensorConfig: missing or invalid 'sensor_id' in " + path);
    }
    cfg.sensorId = jsonObject["sensor_id"].get<std::string>();

    // interval_seconds (optional int, default 1)
    if (jsonObject.contains("interval_seconds")) {
        if (!jsonObject["interval_seconds"].is_number_integer()) {
            throw std::runtime_error("SensorConfig: 'interval_seconds' must be an integer in " +
                                     path);
        }

        if (jsonObject["interval_seconds"] <= 0) {
            throw std::runtime_error("SensorConfig: 'interval_seconds' must be > 0 in " + path);
        }

        cfg.intervalSeconds = jsonObject["interval_seconds"].get<int32_t>();
    } else {
        cfg.intervalSeconds = 1;
    }

    // Optional maps
    readStringMapIfPresent(jsonObject, "units", cfg.units);
    readStringMapIfPresent(jsonObject, "metadata", cfg.metadata);

    return cfg;
}

// ---------------- Transport ----------------

TransportConfig ConfigLoader::loadTransportConfig(const std::string& path) {

    const json jsonObject = readJsonFile(path);

    if (!jsonObject.contains("kind") || !jsonObject["kind"].is_string()) {
        throw std::runtime_error("TransportConfig: missing or invalid 'kind' in " + path);
    }

    TransportConfig cfg;
    cfg.kind = jsonObject["kind"].get<std::string>();

    if (cfg.kind == "tcp") {
        if (!jsonObject.contains("tcp") || !jsonObject["tcp"].is_object()) {
            throw std::runtime_error("TransportConfig: missing 'tcp' object for kind='tcp' in " +
                                     path);
        }
        const auto& tcp = jsonObject["tcp"];

        if (!tcp.contains("host") || !tcp["host"].is_string()) {
            throw std::runtime_error("TransportConfig: missing or invalid 'tcp.host' in " + path);
        }
        if (!tcp.contains("port") || !tcp["port"].is_number_integer()) {
            throw std::runtime_error("TransportConfig: missing or invalid 'tcp.port' in " + path);
        }
        if (!isValidPortRange(tcp["port"])) {
            throw std::runtime_error("TransportConfig: 'tcp.port' out of range (1..65535) in " +
                                     path);
        }

        cfg.host = tcp["host"].get<std::string>();
        cfg.port = tcp["port"].get<u_int16_t>();


    } else if (cfg.kind == "udp") {

        if (!jsonObject.contains("udp") || !jsonObject["udp"].is_object()) {
            throw std::runtime_error("TransportConfig: missing 'udp' object for kind='udp' in " +
                                     path);
        }
        const auto& udp = jsonObject["udp"];

        if (!udp.contains("host") || !udp["host"].is_string()) {
            throw std::runtime_error("TransportConfig: missing or invalid 'udp.host' in " + path);
        }
        if (!udp.contains("port") || !udp["port"].is_number_integer()) {
            throw std::runtime_error("TransportConfig: missing or invalid 'udp.port' in " + path);
        }
        if (!isValidPortRange(udp["port"])) {
            throw std::runtime_error("TransportConfig: 'udp.port' out of range (1..65535) in " + path);
        }

        cfg.host = udp["host"].get<std::string>();
        cfg.port = udp["port"].get<uint16_t>();

    } else {
        throw std::runtime_error("TransportConfig: unsupported kind '" + cfg.kind + "' in " + path);
    }

    return cfg;
}
