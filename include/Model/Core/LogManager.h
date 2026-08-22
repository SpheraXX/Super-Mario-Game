#ifndef MODEL_CORE_LOGMANAGER_H
#define MODEL_CORE_LOGMANAGER_H

#include <string>
#include <fstream>
#include <mutex>
#include <iostream>

namespace model {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

class LogManager {
public:
    static LogManager& instance();

    void setLogFile(const std::string& filepath);
    void setLogLevel(LogLevel level);
    LogLevel getLogLevel() const { return minLevel; }

    void log(LogLevel level, const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);

    void flush();

private:
    LogManager();
    ~LogManager();

    LogManager(const LogManager&) = delete;
    LogManager& operator=(const LogManager&) = delete;

    std::string levelToString(LogLevel level) const;
    std::string currentTimestamp() const;

    LogLevel minLevel = LogLevel::Info;
    std::ofstream logFile;
    std::string logFilePath = "log.txt";
    std::mutex logMutex;
};

} // namespace model

#endif // MODEL_CORE_LOGMANAGER_H
