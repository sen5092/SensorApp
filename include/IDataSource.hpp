/**
 * @file IDataSource.hpp
 * @brief Interface for data acquisition sources used by the Sensor.
 *
 * Abstract base class defining a minimal interface for obtaining sensor data,
 * such as from hardware (e.g., camera, serial, etc.) or simulated inputs.
 * Implementations must provide a method to fetch a data sample or frame.
 *
 * @see HardwareDataSource
 * @see SimulationDataSource
 */

#pragma once

#include <string>
#include <unordered_map>

// Abstract base class (interface) for any data generator.
// Could be a real hardware generator, or a mock/test one.
class IDataSource {
public:
    virtual ~IDataSource () = default;

    IDataSource() = default;
    IDataSource(const IDataSource&) = delete;
    IDataSource& operator=(const IDataSource&) = delete;
    IDataSource(IDataSource&&) = delete;
    IDataSource& operator=(IDataSource&&) = delete;

    // Core contract: produce the latest set of values.
    // Each metric is identified by a string name.
    // Returns a map of metric name -> numeric value.
    virtual std::unordered_map<std::string, double> readAll() = 0;
};
