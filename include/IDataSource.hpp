// ISensorDataSource {.hpp
#pragma once

#include <string>
#include <unordered_map>

// Abstract base class (interface) for any data generator.
// Could be a real hardware generator, or a mock/test one.
class IDataSource {
public:
    virtual ~IDataSource () = default; 

    // Core contract: produce the latest set of values.
    // Each metric is identified by a string name.
    // Returns a map of metric name -> numeric value.
    virtual std::unordered_map<std::string, double> readAll() = 0;
};
