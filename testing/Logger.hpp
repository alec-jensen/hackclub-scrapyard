#pragma once
#include <string>
#include <iostream>
#include <windows.h>
#include <iomanip>
#include <ctime>
#include <sstream>

class Logger {
public:
    enum class LogLevel {
        MAIN,
        WARNING,
        ERROR_LEVEL
    };

    static void log(LogLevel level, const std::string& message) {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        
        const char* levelStr;
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        
        switch (level) {
            case LogLevel::MAIN:
                SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
                levelStr = "[MAIN]";
                break;
            case LogLevel::WARNING:
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
                levelStr = "[WARNING]";
                break;
            case LogLevel::ERROR_LEVEL:
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
                levelStr = "[ERROR]";
                break;
            default:
                levelStr = "[INFO]";
                break;
        }

        std::stringstream ss;
        ss << "[" << std::setfill('0') << std::setw(2) << tm.tm_hour << "."
           << std::setfill('0') << std::setw(2) << tm.tm_min << "."
           << std::setfill('0') << std::setw(2) << tm.tm_sec << "] "
           << levelStr << ": " << message;

        std::string logMessage = ss.str();
        OutputDebugStringA((logMessage + "\n").c_str());
        std::cout << logMessage << std::endl;
        
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }
};