/**
 * @file main.cpp
 * @brief Entry point for the SensorApp program.
 *
 * This file implements the main runtime loop for SensorApp. It initializes
 * configuration, constructs the appropriate data source (hardware or simulation),
 * sets up the transport mechanism (e.g., UDP or TCP), and launches a background
 * thread to handle periodic sensor readings and data transmission.
 *
 * Key features include:
 *  - Embedded-style infinite main loop that supervises system state.
 *  - Graceful shutdown via signal handling (`SIGINT`, `SIGTERM`) using a shared atomic flag.
 *  - Optional run duration limit via the `RUN_DURATION_SECONDS` environment variable.
 *  - Heartbeat logging from the main thread to indicate liveness.
 *  - Modular architecture using dependency injection (DataSource, Transport).
 *
 * All components log their actions to both stdout and a log file ("sensor.log") using a custom logger.
 * This setup mirrors production-grade daemon behavior in an embedded or system-level service.
 *
 * Environment Variables:
 *  - SENSOR_CONFIG: path to the sensor configuration JSON file.
 *  - TRANSPORT_CONFIG: path to the transport configuration JSON file.
 *  - RUN_DURATION_SECONDS: optional max run time; 0 means run indefinitely.
 *  - SIMULATION_DATASOURCE_CONFIG: path to simulation-specific config (if running in sim mode).
 *
 * @author Scott Novak
 * @date Created October 1, 2025
 */


#include "ConfigLoader.hpp"
#include "Logger.hpp"
#include "Sensor.hpp"
#include "TransportFactory.hpp"
#include "HardwareDataSource.hpp"

#include <atomic>
#include <csignal>
#include <chrono>
#include <cstdlib> // std::getenv
#include <exception>
#include <string>
#include <string_view>
#include <thread>
#include <memory>
#include <utility>

#ifdef USE_CAMERA
#include "OpenCvCamera.hpp"
#else
#include "MockCamera.hpp"
#endif


namespace {

    // Shared flag to signal all threads to stop running
    std::atomic<bool> running = true;   // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

    // Signal handler for graceful shutdown
    void handleSignal(int signal)
    {
        std::string name;
        switch (signal) {
            case SIGINT:  name = "SIGINT (Ctrl-C)"; break;
            case SIGTERM: name = "SIGTERM"; break;
            default:      name = "Unknown"; break;
        }

        Logger::instance().info("Received termination signal: " + name);
        running = false;  // <-- Tell all threads to stop
    }

}

namespace {
    constexpr std::string_view kDefaultSensorCfgFile = "config/sensor_config.json";
    constexpr std::string_view kDefaultTransportCfgFile = "config/transport_config.json";
    constexpr int kDefaultRunDurationSecs = 0;    // 0 = run indefinitely
    constexpr int kHeartbeatIntervalSecs = 60;    // log heartbeat every k seconds

    // Runtime: controlled by RUN_DURATION_SECONDS env var.
    // If zero (the default) run indefinitely; otherwise run for that many seconds.
    constexpr const char* kDefaultSensorEnv = "SENSOR_CONFIG";
    constexpr const char* kDefaultTransportEnv = "TRANSPORT_CONFIG";
    constexpr const char* kRunDurationEnv = "RUN_DURATION_SECONDS";
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

        // Setup signal handlers for graceful shutdown
        std::signal(SIGINT, handleSignal);
        std::signal(SIGTERM, handleSignal);

        // Initialize logger file
        Logger::instance().setLogFile("sensor.log");

        // ----- Main setup -----
         // 1. Load config
        const std::string sensorCfgPath = envOrDefault(kDefaultSensorEnv, kDefaultSensorCfgFile);
        const std::string transportCfgPath = envOrDefault(kDefaultTransportEnv, kDefaultTransportCfgFile);
        const auto sensorCfg = ConfigLoader::loadSensorConfig(sensorCfgPath);
        const auto transportCfg = ConfigLoader::loadTransportConfig(transportCfgPath);

#ifdef USE_CAMERA

    // Try to open the default camera (index 0)
    // If it fails, log an error and exit.
    auto camera = std::make_shared<OpenCvCamera>();
    if(camera->open(0)){
        Logger::instance().info("Camera opened successfully.");
    } else {
        Logger::instance().error("Failed to open camera.");
        return EXIT_FAILURE;
    }

    Logger::instance().info("Camera opened successfully.");

    auto dataSource = std::make_unique<HardwareDataSource>(camera);

#else
    auto camera = std::make_shared<MockCamera>();
    if(camera->open(0)){
        Logger::instance().info("MockCamera opened successfully.");
    } else {
        Logger::instance().error("Failed to open MockCamera.");
        return EXIT_FAILURE;
    }
    auto dataSource = std::make_unique<HardwareDataSource>(camera);
#endif

        // 2. Create transport
        auto transport = TransportFactory::make(transportCfg);

        // 3. Create sensor object and start sensor thread
        Sensor sensor{sensorCfg, std::move(dataSource), std::move(transport)};
        std::thread sensorThread([&sensor]() {
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
        const int runDuration = envOrDefaultInt(kRunDurationEnv, kDefaultRunDurationSecs);
        auto startTime = std::chrono::steady_clock::now();
        Logger::instance().info("Sensor running. Press Ctrl-C to stop."
            + (runDuration > 0 ? (" Will auto-stop after " + std::to_string(runDuration) + " seconds.") : ""));

        while (running) {

            static int tickCount = 0;
            if (++tickCount % kHeartbeatIntervalSecs == 0) {  // every ~k seconds
                Logger::instance().info("Main loop heartbeat: system running normally.");
            }

            if (runDuration > 0 &&
                std::chrono::steady_clock::now() - startTime > std::chrono::seconds(runDuration)) {
                Logger::instance().info("Run duration reached, stopping...");
                running = false;
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }


        // Wait for sensor thread to exit
        // Make sure the thread has finished before exite
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
