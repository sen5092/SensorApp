// DataSource.h
#pragma once

#include <string>
#include <unordered_map>

class DataSource {
public:
    DataSource();  // default ctor picks mode internally

    std::unordered_map<std::string, double> readAll();

private:
    enum class Mode { Hardware, Mock };
    Mode mode_;
    std::string configFilePath_;

    std::unordered_map<std::string, double> readFromHardware();
    std::unordered_map<std::string, double> readFromJson();
};
