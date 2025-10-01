#include "HardwareDataSource.hpp"

// Stub for now – replace with actual hardware drivers later
std::unordered_map<std::string, double> HardwareDataSource::readAll() {
    return {
        {"temperature", 42.0},
        {"pressure", 101.3}
    };
}
