#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <windows.h>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <mutex>
#include <filesystem>
#include <thread>
#include <atomic>

class Logger {
public:
    enum class LogLevel {
        DEBUG,
        INFO,
        MAIN,
        WARNING,
        ERROR_LEVEL,
        CRITICAL
    };

private:
    static std::ofstream logFile;
    static std::mutex logMutex;
    static std::atomic<bool> fileLoggingEnabled;
    static std::atomic<LogLevel> minLogLevel;
    static std::string logFilePath;
    static constexpr size_t MAX_LOG_SIZE = 10 * 1024 * 1024; 
    
    static void rotateLogFileIfNeeded();
    static std::string getCurrentTimestamp();
    static std::string getLevelString(LogLevel level);
    static void setConsoleColor(LogLevel level);

public:
    static bool initialize(const std::string& filePath = "debug.log", LogLevel level = LogLevel::INFO);
    static void setLogLevel(LogLevel level);
    static void log(LogLevel level, const std::string& message, const char* file = nullptr, int line = -1);
    static void shutdown();
};

#define LOG_DEBUG(message)    Logger::log(Logger::LogLevel::DEBUG, message, __FILE__, __LINE__)
#define LOG_INFO(message)     Logger::log(Logger::LogLevel::INFO, message, __FILE__, __LINE__)
#define LOG_MAIN(message)     Logger::log(Logger::LogLevel::MAIN, message, __FILE__, __LINE__)
#define LOG_WARNING(message)  Logger::log(Logger::LogLevel::WARNING, message, __FILE__, __LINE__)
#define LOG_ERROR(message)    Logger::log(Logger::LogLevel::ERROR_LEVEL, message, __FILE__, __LINE__)
#define LOG_CRITICAL(message) Logger::log(Logger::LogLevel::CRITICAL, message, __FILE__, __LINE__)