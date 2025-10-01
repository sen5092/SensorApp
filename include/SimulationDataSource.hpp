#include "IDataSource.hpp"
#include <string>
#include <unordered_map>
#include <random>
class SimulationDataSource : public IDataSource{
    public:
        // --- Construction ---
        SimulationDataSource(const std::string &configPath);
    
        // --- Data Generation ---
        double generate(const std::string &metricName);

        std::unordered_map<std::string, double> readAll() override;
    
    private:
        // --- Internal Data Structure ---
        struct Limits {
            bool hasFixed = false;
            double fixedValue = 0.0;
    
            bool hasRange = false;
            double min = 0.0;
            double max = 0.0;
            double badProbability = 0.0;
        };
    
        // --- State ---
        std::unordered_map<std::string, Limits> metricLimits;
        std::mt19937 rng;  // random engine
    
        // --- Helpers ---
        double generateValue(const Limits &limits);
    };
    