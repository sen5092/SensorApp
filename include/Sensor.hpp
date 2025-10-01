// Sensor.hpp
#pragma once

#include <memory>
#include <string>
#include <cstdint>
#include "IDataSource.hpp"
#include "ITransport.hpp"
#include "TcpSocket.hpp"

// Sensor: reads values from an IDataGenerator at a fixed interval
// and sends them (as JSON text) to a collector via TcpClient.
class Sensor {
public:
    // Construct with path to sensor_config.json and a data generator
    Sensor(std::string configPath, std::shared_ptr<ITransport> transport, std::shared_ptr<IDataGatherer> generator);

    // Load config, create TcpClient (but don't connect yet)
    void loadConfig();

    // Open TCP connection to collector (blocking)
    void connect();

    // Do one cycle: read→serialize→send
    void runOnce();

    // Close TCP connection (safe to call multiple times)
    void close() noexcept;

private:
    // Helpers (implementation detail)
    std::string buildJsonPayload(const std::unordered_map<std::string, double>& values) const;

    // Config-derived state
    std::string configPath_;
    std::string sensorId_;
    int32_t     intervalSeconds_{1};

    // Dependencies / runtime state
    std::shared_ptr<IDataGenerator> generator_;
    std::shared_ptr<ITransport>     transport_;
    bool                            loaded_{false};
};
