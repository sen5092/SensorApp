//
//  main.cpp
//  PracticeC++
//
//  Created by Scott Novak on 7/13/22.
//

#include <iostream>
using namespace std;


#include "TcpSocket.hpp"
#include <exception>
#include <string>
#include "ConfigLoader.hpp"
#include "ConfigTypes.hpp"

#ifdef SIMULATION
  #include "SimulationDataSource.hpp"
#else
  #include "HardwareDataSource.hpp"
#endif

// Default config paths (next to the executable after CMake copy)
static const char* kDefaultSensorCfg    = "config/sensor_config.json";
static const char* kDefaultTransportCfg = "config/transport_config.json";

#ifdef SIMULATION
  static const char* kDefaultDataSourceCfg = "config/simulation_datasource_config.json";
#endif


static std::string envOrDefault(const char* name, const char* defval) {
    if (const char* v = std::getenv(name)) return std::string(v);
    return std::string(defval);
}



int main(int argc, const char* argv[]) {

    std::cout << "Starting Sensor Application (" 
    << (SIMULATION ? "SIMULATION" : "HARDWARE") 
    << " mode)...\n";


    const std::string sensorCfgPath     = envOrDefault("SENSOR_CONFIG",      kDefaultSensorCfg);
    const std::string transportCfgPath  = envOrDefault("TRANSPORT_CONFIG",   kDefaultTransportCfg);
    //const std::string dsSelectorPath    = envOrDefault("DATASOURCE_SELECTOR",kDefaultDataSourceSel);

    

#ifdef SIMULATION
    const std::string dataSourceCfgPath= envOrDefault("DATASOURCE_CONFIG", kDefaultDataSourceCfg);
    SimulationDataSource ds(dataSourceCfgPath);
#else
    HardwareDataSource ds(dataSourceCfgPath);  // if your HW impl takes a config
#endif

    ConfigLoader loader;
    const auto sensorCfg   = loader.loadSensorConfig(sensorCfgPath);
    const auto transportCfg= loader.loadTransportConfig(transportCfgPath);
    //const auto dsSel       = loader.loadSensorDataSourceSelector(dsSelectorPath);

    //auto dataSource = SensorDataSourceFactory::make(dsSel);
    //auto transport  = TransportFactory::make(transportCfg);

    //Sensor sensor{sensorCfg, *dataSource, *transport};


    // --------------------------- //
    // MockSensorDataSource sensor("config/mock_sensor_generator_config.json");
    // cout << "Temperator: " << sensor.generate("temperature") << '\n';

    // cout << "All metrics:\n";
    // for (const auto& [name, value] : sensor.generateAll()) {
    //     cout << "  " << name << ": " << value << '\n';
    // }


    TcpSocket cli{"127.0.0.1", 8080};
    cli.connect();
    cli.sendString("{\"hello\":\"world\"}\n");
    cli.close();

    return 0;
}   
