/**
 * @file ConfigLoader.hpp
 * @brief Declares ConfigLoader for parsing and validating JSON configuration files.
 *
 * ConfigLoader reads application settings (e.g., transport, sensors) from JSON.
 * It performs general validation (types, ranges) while protocol-specific checks
 * are handled in the transport layer.
 */

#pragma once

#include <string>
#include "ConfigTypes.hpp"

class ConfigLoader {
public:
    [[nodiscard]] static SensorConfig    loadSensorConfig(const std::string& path);
    static TransportConfig loadTransportConfig(const std::string& path);
};
