#pragma once
#include "Logger.hpp"
#include <windows.h>
#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <vector>

class SerialMonitor {
private:
    HANDLE serialHandle;
    bool connected;
    std::string portName;
    std::atomic<bool> shouldRun;
    std::thread monitorThread;
    std::function<void(const std::string&)> dataCallback;

public:
    SerialMonitor(const std::string& port = "COM7");
    ~SerialMonitor();
    
    bool connect();
    void disconnect();
    bool isConnected() const { return connected; }
    
    bool sendCommand(const std::string& cmd);
    std::string receiveData(int timeout = 1000);
    
    // Simon game-specific functions
    bool sendSimonGameLength(int length);
    bool verifySimonGameSuccess(int timeout = 5000);
    
    // Monitoring with callback
    void startMonitoring(std::function<void(const std::string&)> callback = nullptr);
    void stopMonitoring();

private:
    void monitorTask();
    void logMessage(const std::string& level, const std::string& message);
};