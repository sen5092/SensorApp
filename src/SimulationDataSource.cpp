#include "SimulationDataSource.hpp"   // header for the class
#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>         // JSON library
#include "Logger.hpp"

using json = nlohmann::json;

// -----------------
// Constructor
// -----------------
SimulationDataSource::SimulationDataSource(const std::string &configPath) {

    std::ifstream file(configPath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config file: " + configPath);
    }

    Logger::instance().debug("Simulation data source read from " + configPath);

    json config;
    file >> config;

    if (!config.contains("limits")) {
        throw std::runtime_error("Config missing 'limits' field");
    }

    // Parse each metric entry
    for (auto &[metricName, values] : config["limits"].items()) {
        Limits limits;

        // Case 1: fixed value
        if (values.contains("fixed")) {
            limits.hasFixed = true;
            limits.fixedValue = values["fixed"].get<double>();
        }

        // Case 2: range
        if (values.contains("min") && values.contains("max")) {
            limits.hasRange = true;
            limits.min = values["min"].get<double>();
            limits.max = values["max"].get<double>();
            limits.badProbability = values.value("bad_probability", 0.0);
        }

        metricLimits[metricName] = limits;
    }

    // Seed random generator
    std::random_device rd;
    rng.seed(rd());
}

// -----------------
// Generate for one metric
// -----------------
double SimulationDataSource::generate(const std::string &metricName) {
    auto it = metricLimits.find(metricName);
    if (it == metricLimits.end()) {
        throw std::runtime_error("Metric not found: " + metricName);
    }
    return generateValue(it->second);
}

// -----------------
// Generate for all metrics
// -----------------
std::unordered_map<std::string, double> SimulationDataSource::readAll() {
    std::unordered_map<std::string, double> results;
    for (auto &pair : metricLimits) {
        results[pair.first] = generateValue(pair.second);
    }
    return results;
}

// -----------------
// Internal helper
// -----------------
double SimulationDataSource::generateValue(const Limits &limits) {
    // Case 1: Fixed
    if (limits.hasFixed) {
        return limits.fixedValue;
    }

    // Case 2: Range
    if (limits.hasRange) {
        std::uniform_real_distribution<double> probDist(0.0, 1.0);
        double roll = probDist(rng);

        if (roll < limits.badProbability) {
            // Bad data case
            std::bernoulli_distribution sideDist(0.5);
            if (sideDist(rng)) {
                return limits.min - 10.0;  // example: below range
            } else {
                return limits.max + 10.0;  // example: above range
            }
        }

        // Normal case
        std::uniform_real_distribution<double> dist(limits.min, limits.max);
        return dist(rng);
    }

    // Case 3: Misconfigured metric
    throw std::runtime_error("Metric misconfigured: no fixed or range values");
}
