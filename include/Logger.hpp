/**
 * @file Logger.hpp
 * @brief Simple thread-safe singleton logger for console and file output.
 *
 * The Logger provides timestamped log messages across multiple severity levels
 * (info, debug, warning, error). Thread-safe via std::mutex to ensure messages
 * from different threads do not interleave. Supports optional log file output
 * in addition to standard output.
 *
 * @note All log methods are safe to call from any thread.
 */

#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <mutex>
#include <memory>
#include <ctime>
#include <array>
#include <cstdint>

enum class LogLevel : std::uint8_t {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    static Logger& instance() {
        static Logger loggerInstance;
        return loggerInstance;
    }

        // Prevent copying and moving — modern style (public)
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    void setLogFile(const std::string& filename) {
        const std::lock_guard<std::mutex> lock(mutex_);
        if (file_.is_open()) {
            file_.close();
        }
        file_.open(filename, std::ios::out | std::ios::app);
    }

    void log(LogLevel level, const std::string& message) {
        const std::lock_guard<std::mutex> lock(mutex_);
        const std::string timestamp = getTimestamp();
        const std::string levelStr = levelToString(level);

        if (file_.is_open()) {
            file_ << "[" << timestamp << "] [" << levelStr << "] " << message << '\n';
        }

        // Also print to console
        std::cout << "[" << timestamp << "] [" << levelStr << "] " << message << '\n';
    }

    void debug(const std::string& msg)   { log(LogLevel::DEBUG, msg); }
    void info(const std::string& msg)    { log(LogLevel::INFO, msg); }
    void warning(const std::string& msg) { log(LogLevel::WARNING, msg); }
    void error(const std::string& msg)   { log(LogLevel::ERROR, msg); }

private:
    Logger() = default;
    ~Logger() {
        if (file_.is_open()) {
            file_.close();
        }
    }

    static std::string getTimestamp() {
        constexpr std::size_t kTimestampBufSize = 20; // YYYY-MM-DD HH:MM:SS + '\0'
        const std::time_t now = std::time(nullptr);
        std::array<char, kTimestampBufSize> buf{}; // safer than char[20]

        const std::tm* local = std::localtime(&now);
        if (local != nullptr && std::strftime(buf.data(), buf.size(), "%Y-%m-%d %H:%M:%S", local) != 0) {
            return {buf.data()};
        }
        return {"unknown-time"};
    }

    static std::string levelToString(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG:   return "DEBUG";
            case LogLevel::INFO:    return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR:   return "ERROR";
        }
        return "UNKNOWN";
    }

    std::ofstream file_;
    std::mutex mutex_;
};
