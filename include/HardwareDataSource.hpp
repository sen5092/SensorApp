

#pragma once

#include "IDataSource.hpp"

#include <string>
#include <unordered_map>
#include <random>

class HardwareDataSource : public IDataSource {
    public:

        // --- Construction ---
        HardwareDataSource(const std::string &configPath);

        // --- Data Generation ---
        double generate(const std::string &metricName);

        std::unordered_map<std::string, double> readAll() override;

};