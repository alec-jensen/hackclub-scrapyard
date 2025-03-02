// SerialCommunication.hpp
#pragma once
#include "Logger.hpp"
#include <windows.h>
#include <string>
#include <vector>

class SerialCommunication {
private:
    HANDLE serialHandle;
    bool connected;
    std::string portName;

public:
    SerialCommunication(const std::string& port = "COM3");
    ~SerialCommunication();
    
    bool connect();
    void disconnect();
    bool isConnected() const { return connected; }
    
    bool sendCommand(const std::string& cmd);
    bool sendPattern(const std::vector<int>& pattern);
    std::string receiveResponse(int timeout = 2000);
    bool verifyPatternCompleted();
};