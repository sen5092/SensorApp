// Sensor.cpp
#include "Sensor.hpp"

#include "IDataSource.hpp"
#include "ITransport.hpp"
// (Only needed if your header uses SensorConfig fields directly in the ctor)
#include "ConfigTypes.hpp"

#include <nlohmann/json.hpp>
#include <chrono>
#include <string>
#include <unordered_map>
#include <stdexcept>

using json = nlohmann::json;

// ----- ctor -----
Sensor::Sensor(const SensorConfig& cfg,
               IDataSource& source,
               ITransport& transport)
    : config_(cfg),
      sensorId_(cfg.sensorId),
      intervalSeconds_(cfg.intervalSeconds),
      datasource_(source),
      transport_(transport)
{
    if (sensorId_.empty()) {
        throw std::invalid_argument("Sensor: sensorId must not be empty");
    }
    if (intervalSeconds_ <= 0) {
        throw std::invalid_argument("Sensor: intervalSeconds must be > 0");
    }
}

// ----- connect/close -----
void Sensor::connect() {
    transport_.connect();
}

void Sensor::close() noexcept {
    transport_.close();
}

void Sensor::run(std::atomic<bool>& running) {
    while (running) {
        runOnce();
        std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds_));
    }
}

// ----- one tick: read -> json -> send -----
void Sensor::runOnce() {
    // 1) get current readings
    const auto values = datasource_.readAll();

    // 2) build payload
    const std::string payload = buildJsonPayload(values);

    // 3) send (blocking)
    transport_.sendString(payload);
}

// ----- payload builder (minimal, line-delimited JSON) -----
std::string Sensor::buildJsonPayload(const std::unordered_map<std::string, double>& readingsMap) const
{
    json payload;

    // identity
    payload["sensor_id"] = sensorId_;

    // optional static metadata
    if (!config_.metadata.empty()) {
        payload["metadata"] = config_.metadata;
    }

    // timestamp (ms since epoch)
    const auto now   = std::chrono::system_clock::now();
    const auto epoch = std::chrono::time_point_cast<std::chrono::milliseconds>(now)
                         .time_since_epoch()
                         .count();
    payload["timestamp_ms"] = epoch;

    // helpers
    auto roundToDecimals = [](double value, int decimals) {
        const double scale = std::pow(10.0, decimals);
        return std::round(value * scale) / scale;
    };

    auto inferUnitFromReadingName = [this](std::string_view readingName) -> std::string {
        if (auto it = config_.units.find(std::string(readingName)); it != config_.units.end()) {
            return it->second;
        }
        // sensible defaults for image-sensor fields
        if (readingName.find("width")     != std::string_view::npos ||
            readingName.find("height")    != std::string_view::npos) return "pixels";
        if (readingName.find("channels")  != std::string_view::npos) return "count";
        if (readingName.find("bytes")     != std::string_view::npos ||
            readingName.find("size")      != std::string_view::npos) return "bytes";
        if (readingName.find("brightness")!= std::string_view::npos ||
            readingName.find("luma")      != std::string_view::npos) return "intensity";
        return "unknown";
    };

    // Readings object
    json readingsJson = json::object();
    for (const auto& [readingName, readingValue] : readingsMap) {
        const double roundedValue = roundToDecimals(readingValue, 3);

        json readingJsonObject;
        readingJsonObject["value"] = roundedValue;
        readingJsonObject["unit"]  = inferUnitFromReadingName(readingName);

        readingsJson[readingName] = readingJsonObject;
    }

     if (!readingsJson.empty()) {
        payload["readings"] = readingsJson;
    }

    std::string out = payload.dump();
    out.push_back('\n');
    return out;
}


