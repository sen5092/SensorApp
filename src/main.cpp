//
//  main.cpp
//  PracticeC++
//
//  Created by Scott Novak on 7/13/22.
//

#include <iostream>
using namespace std;


#include "TcpSocket.hpp"
#include "Logger.hpp"
#include "Sensor.hpp"
#include "TransportFactory.hpp"
#include "UdpSocket.hpp"
#include "UdpTransport.hpp"
#include "TcpTransport.hpp"
#include <cstdlib>  // std::getenv
#include <exception>
#include <string>
#include "ConfigLoader.hpp"
#include "ConfigTypes.hpp"
#include <thread>
#include <atomic>
#include <chrono>
#include <string_view>

#ifdef USE_OPENCV
  #include "HardwareDataSource.hpp"
#else
  #include "SimulationDataSource.hpp"
#endif

// Default config paths (next to the executable after CMake copy)
//static const char* DEFAULT_SENSOR_CFG    = "config/sensor_config.json";
//static const char* DEFAULT_TRANSPORT_CFG = "config/transport_config.json";

namespace {
    constexpr std::string_view DEFAULT_SENSOR_CFG = "config/sensor_config.json";
    constexpr std::string_view DEFAULT_TRANSPORT_CFG = "config/transport_config.json";
}


#ifdef SIMULATION
  static const char* kDefaultDataSourceCfg = "config/simulation_datasource_config.json";
#endif

namespace {
  std::string envOrDefault(const char* name, const std::string_view defval) {
    if (const char* envvar = std::getenv(name)) {
      return envvar;
    }
    return std::string(defval);
  }
}



int main(int /*argc*/, const char* /*argv*/[]) {


    const std::string sensorCfgPath     = envOrDefault("SENSOR_CONFIG",      DEFAULT_SENSOR_CFG);
    const std::string transportCfgPath  = envOrDefault("TRANSPORT_CONFIG",   DEFAULT_TRANSPORT_CFG);
    //const std::string dsSelectorPath    = envOrDefault("DATASOURCE_SELECTOR",kDefaultDataSourceSel);

    

#ifdef USE_OPENCV
  HardwareDataSource datasource;
#else
    const std::string dataSourceCfgPath= envOrDefault("DATASOURCE_CONFIG", kDefaultDataSourceCfg);
    SimulationDataSource ds(dataSourceCfgPath);
#endif

    const ConfigLoader loader;
    const auto sensorCfg = loader.loadSensorConfig(sensorCfgPath);
    const auto transportCfg = loader.loadTransportConfig(transportCfgPath);
    //const auto dsSel       = loader.loadSensorDataSourceSelector(dsSelectorPath);

    //auto dataSource = DataSourceFactory::make(dsSel);
    auto transport  = TransportFactory::make(transportCfg);

    Sensor sensor{sensorCfg, datasource, *transport};
    sensor.connect();
    sensor.runOnce();
    sensor.close();

    // Start sensor thread
    std::atomic<bool> running(true);
    const std::thread sensorThread([&] {
        sensor.connect();
        sensor.run(running);
        sensor.close();
    });


    // --------------------------- //
    // MockSensorDataSource sensor("config/mock_sensor_generator_config.json");
    // cout << "Temperator: " << sensor.generate("temperature") << '\n';

    // cout << "All metrics:\n";
    // for (const auto& [name, value] : sensor.generateAll()) {
    //     cout << "  " << name << ": " << value << '\n';
    // }


    //TcpSocket cli{"127.0.0.1", 8080};
    //cli.connect();
    //cli.sendString("{\"hello\":\"world\"}\n");
    //cli.close();

        // Let the program run for 10 seconds

    std::this_thread::sleep_for(std::chrono::seconds(10));
    running = false;

    // Wait for sensor thread to exit
    //sensorThread.join();

    std::cout << "Program finished after 10 seconds.\n";

    return 0;
}   
