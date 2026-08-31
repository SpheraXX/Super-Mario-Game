#include "Model/Core/LogManager.h"
#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <sstream>

namespace model {

namespace {
std::string resolveLogPath(const std::string& filename) {
    if (std::filesystem::exists("CMakeLists.txt")) {
        return filename;
    }
    if (std::filesystem::exists("../../CMakeLists.txt")) {
        return "../../" + filename;
    }
    if (std::filesystem::exists("../CMakeLists.txt")) {
        return "../" + filename;
    }
    return filename;
}
}

LogManager& LogManager::instance() {
    static LogManager singleton;
    return singleton;
}

LogManager::LogManager() {
    logFilePath = resolveLogPath("log.txt");
    logFile.open(logFilePath, std::ios::out | std::ios::app);
}

LogManager::~LogManager() {
    if (logFile.is_open()) {
        logFile.flush();
        logFile.close();
    }
}

void LogManager::setLogFile(const std::string& filepath) {
    std::lock_guard<std::mutex> lock(logMutex);
    if (logFile.is_open()) {
        logFile.flush();
        logFile.close();
    }
    logFilePath = filepath;
    logFile.open(logFilePath, std::ios::out | std::ios::app);
}

void LogManager::setLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(logMutex);
    minLevel = level;
}

std::string LogManager::levelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO";
        case LogLevel::Warning: return "WARN";
        case LogLevel::Error:   return "ERROR";
        default:                return "UNKNOWN";
    }
}

std::string LogManager::currentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    std::time_t timeNow = std::chrono::system_clock::to_time_t(now);
    std::tm tmNow{};
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tmNow, &timeNow);
#else
    localtime_r(&timeNow, &tmNow);
#endif
    std::ostringstream ss;
    ss << std::put_time(&tmNow, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void LogManager::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    if (level < minLevel) return;

    std::string formatted = "[" + currentTimestamp() + "] [" + levelToString(level) + "] " + message + "\n";

    if (level == LogLevel::Error || level == LogLevel::Warning) {
        std::cerr << formatted;
    } else {
        std::cout << formatted;
    }

    if (logFile.is_open()) {
        logFile << formatted;
        logFile.flush();
    } else {
        // Try opening in case working directory was set after initialization
        logFile.open(logFilePath, std::ios::out | std::ios::app);
        if (logFile.is_open()) {
            logFile << formatted;
            logFile.flush();
        }
    }
}

void LogManager::debug(const std::string& message) {
    log(LogLevel::Debug, message);
}

void LogManager::info(const std::string& message) {
    log(LogLevel::Info, message);
}

void LogManager::warning(const std::string& message) {
    log(LogLevel::Warning, message);
}

void LogManager::error(const std::string& message) {
    log(LogLevel::Error, message);
}

void LogManager::flush() {
    std::lock_guard<std::mutex> lock(logMutex);
    if (logFile.is_open()) {
        logFile.flush();
    }
    std::cout.flush();
    std::cerr.flush();
}

} // namespace model
