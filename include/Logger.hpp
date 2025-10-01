// Logger.h
#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <mutex>
#include <memory>
#include <ctime>

enum class LogLevel {
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

    void setLogFile(const std::string& filename) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (file_.is_open()) {
            file_.close();
        }
        file_.open(filename, std::ios::out | std::ios::app);
    }

    void log(LogLevel level, const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string timestamp = getTimestamp();
        std::string levelStr = levelToString(level);

        if (file_.is_open()) {
            file_ << "[" << timestamp << "] [" << levelStr << "] " << message << std::endl;
        }

        // Also print to console
        std::cout << "[" << timestamp << "] [" << levelStr << "] " << message << std::endl;
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

    std::string getTimestamp() {
        std::time_t now = std::time(nullptr);
        char buf[20];
        if (std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now))) {
            return buf;
        }
        return "unknown-time";
    }

    std::string levelToString(LogLevel level) {
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
