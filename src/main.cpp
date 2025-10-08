//
//  main.cpp
//  PracticeC++
//
//  Created by Scott Novak on 7/13/22.
//


#include "ConfigLoader.hpp"
#include "Logger.hpp"
#include "Sensor.hpp"
#include "TransportFactory.hpp"
#include "ITransport.hpp"

#include <atomic>
#include <chrono>
#include <cstdlib> // std::getenv
#include <exception>
#include <string>
#include <string_view>
#include <thread>
#include <memory>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>

#ifdef USE_OPENCV
#include "HardwareDataSource.hpp"
#else
#include "SimulationDataSource.hpp"
#endif

// Default config paths (next to the executable after CMake copy)
// static const char* DEFAULT_SENSOR_CFG    = "config/sensor_config.json";
// static const char* DEFAULT_TRANSPORT_CFG = "config/transport_config.json";

namespace {
    constexpr std::string_view DEFAULT_SENSOR_CFG = "config/sensor_config.json";
    constexpr std::string_view DEFAULT_TRANSPORT_CFG = "config/transport_config.json";

    // Runtime: controlled by RUN_DURATION_SECONDS env var.
    // If zero (the default) run indefinitely; otherwise run for that many seconds.
    constexpr std::string_view kRunDurationEnv = "RUN_DURATION_SECONDS";

#ifndef USE_OPENCV
    constexpr std::string_view DEFAULT_SIMULATION_CFG = "config/simulation_datasource_config.json";
#endif

} // namespace


namespace {
    std::string envOrDefault(const char* name, const std::string_view defval) {
        if (const char* envvar = std::getenv(name)) {
            return envvar;
        }
        return std::string(defval);
    }

    int envOrDefaultInt(const char* name, int defval) {
        if (const char* env = std::getenv(name)) {
            return std::atoi(env);  // returns 0 if invalid or not a number
        }
        return defval;
    }



} // namespace

int main(int /*argc*/, const char* /*argv*/[]) {    // NOLINT(bugprone-exception-escape)

    try {

        Logger::instance().info("Sensor starting up...");


         // 1. Load config
        const std::string sensorCfgPath = envOrDefault("SENSOR_CONFIG", DEFAULT_SENSOR_CFG);
        const std::string transportCfgPath = envOrDefault("TRANSPORT_CONFIG", DEFAULT_TRANSPORT_CFG);
        const auto sensorCfg = ConfigLoader::loadSensorConfig(sensorCfgPath);
        const auto transportCfg = ConfigLoader::loadTransportConfig(transportCfgPath);

#ifdef USE_OPENCV
    HardwareDataSource datasource;
    if (!datasource.ensureCameraAuthorized()) {
        return EXIT_FAILURE;
    }
#else
    const std::string simDataCfgPath = envOrDefault("SIMULATION_DATASOURCE_CONFIG", DEFAULT_SIMULATION_CFG);
    SimulationDataSource datasource(simDataCfgPath);
#endif

        // 2. Create transport
        std::unique_ptr<ITransport> transport = TransportFactory::make(transportCfg);

        // 3. Start sensor thread
        Sensor sensor{sensorCfg, datasource, *transport};

        std::atomic<bool> running(true);
        std::thread sensorThread([&sensor, &running]() {
            try {
                sensor.connect();
                sensor.run(running);
                sensor.close();
            } catch (const std::exception& e) {
                Logger::instance().error(std::string("Sensor thread uncaught exception: ") + e.what());
                // stop main loop if something unrecoverable happened
                running = false;
                try {
                    Logger::instance().warning("Sensor attempting to close after exception...");
                    sensor.close();
                } catch (const std::exception& ex) {
                    Logger::instance().error(std::string("Sensor close failed after exception: ") + ex.what());
                }
            }
        });

        // 4. Main thread: wait for termination signal (Ctrl-C) or optional timeout
        //const int runDuration = envOrDefaultInt(kRunDurationEnv.data(), 12);
        std::this_thread::sleep_for(std::chrono::seconds(10));
        running = false;

        // Wait for sensor thread to exit
        // Make sure the thread has finished before exit
        if (sensorThread.joinable()) {
            sensorThread.join();
        }

        Logger::instance().info("Sensor shutting down...");

    }
    catch (const std::exception& ex) {
        Logger::instance().error(std::string("Fatal error ocured: ") + ex.what());
        return EXIT_FAILURE;
    }
    catch (...) {
        Logger::instance().error("Unknown fatal error ocurred.");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
