// ConfigTypes.hpp
#pragma once

#include <string>
#include <cstdint>
#include <unordered_map>

// ---------- Sensor (identity + timing) ----------
struct SensorConfig {
    std::string sensorId;                         // e.g., "temp-01"
    int32_t     intervalSeconds{1};               // send cadence
    // Optional extras (safe to leave empty):
    std::unordered_map<std::string, std::string> units;     // metric -> unit (e.g., "temperature"->"F")
    std::unordered_map<std::string, std::string> metadata;  // free-form tags (location, model, etc.)
};

// ---------- Transport (how bytes leave the device) ----------
struct TransportConfig {
    std::string kind;  // e.g., "tcp"
    std::string host;  // for tcp
    int32_t     port{0};
};

// ---------- Data generation (what values to produce) ----------
// Per-metric rule: either fixed OR ranged (with optional bad outliers).
struct MetricRule {
    bool   hasFixed{false};
    double fixed{0.0};

    bool   hasRange{false};
    double min{0.0};
    double max{0.0};
    double badProbability{0.0};  // chance to emit an out-of-range value
};

// All metric rules keyed by metric name (e.g., "temperature", "humidity").
struct DataSourceConfig {
    std::unordered_map<std::string, MetricRule> metrics;
};

struct DataSourceSelector {
    std::string kind;        // "mock" | "hardware"
    std::string configPath;  // path to that kind's own config
};
  