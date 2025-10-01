// ConfigLoader.hpp
#pragma once

#include <string>
#include "ConfigTypes.hpp"

class ConfigLoader {
public:
    SensorConfig    loadSensorConfig(const std::string& path) const;
    TransportConfig loadTransportConfig(const std::string& path) const;
    DataSourceConfig loadDataGenerationConfig(const std::string& path) const;
    DataSourceSelector loadSensorDataSourceSelector(const std::string& path) const;

};
