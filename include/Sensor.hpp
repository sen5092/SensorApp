/**
 * @file Sensor.hpp
 * @brief Core sensor component responsible for reading and transmitting data.
 *
 * The Sensor orchestrates data acquisition via an IDataSource implementation
 * and transmits results through an ITransport (e.g., TCP, UDP). It manages the
 * main sensor loop, timing intervals, and graceful shutdown signaling.
 *
 * @note Thread-safe startup and shutdown behavior is expected from the caller.
 */

#pragma once

#include <memory>
#include <string>
#include <cstdint>
#include "IDataSource.hpp"
#include "ITransport.hpp"
#include "TcpSocket.hpp"
#include "ConfigTypes.hpp"
#include <atomic>
#include <thread>

// Sensor: reads values from an IDataGenerator at a fixed interval
// and sends them (as JSON text) to a collector via TcpClient.
class Sensor {
public:
    // Construct with path to sensor_config.json and a data generator
    Sensor(const SensorConfig& config, IDataSource& datasource, ITransport& transport);

    // Load config, create TcpClient (but don't connect yet)
    void loadConfig();

    // Open TCP connection to collector (blocking)
    void connect();

    // Do one cycle: read→serialize→send
    void runOnce();

    void run(std::atomic<bool>& running);

    // Close TCP connection (safe to call multiple times)
    void close() noexcept;

private:
    // Helpers (implementation detail)
    [[nodiscard]] std::string buildJsonPayload(const std::unordered_map<std::string, double>& readingsMap) const;

    // Config-derived state
    SensorConfig config_;
    std::string configPath_;
    std::string sensorId_;
    int32_t     intervalSeconds_ = 1;

    // Dependencies / runtime state
    IDataSource& datasource_;   // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
    ITransport&  transport_;    // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
    bool         loaded_ = false;
};
