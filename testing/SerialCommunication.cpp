// SerialCommunication.cpp
#include "SerialCommunication.hpp"
#include <sstream>

SerialCommunication::SerialCommunication(const std::string& port) 
    : serialHandle(INVALID_HANDLE_VALUE), connected(false), portName(port) {
}

SerialCommunication::~SerialCommunication() {
    disconnect();
}

bool SerialCommunication::connect() {
    LOG_INFO("Attempting to connect to " + portName);
    
    // Convert port name to wide string for Windows API
    std::wstring wPortName(portName.begin(), portName.end());
    
    serialHandle = CreateFileW(
        wPortName.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    
    if (serialHandle == INVALID_HANDLE_VALUE) {
        LOG_ERROR("Failed to open serial port: " + portName + ", error: " + std::to_string(GetLastError()));
        return false;
    }
    
    // Configure serial port parameters
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    
    if (!GetCommState(serialHandle, &dcbSerialParams)) {
        LOG_ERROR("Failed to get serial port state, error: " + std::to_string(GetLastError()));
        CloseHandle(serialHandle);
        serialHandle = INVALID_HANDLE_VALUE;
        return false;
    }
    
    // Configure baud rate and other serial parameters
    dcbSerialParams.BaudRate = CBR_115200;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    
    if (!SetCommState(serialHandle, &dcbSerialParams)) {
        LOG_ERROR("Failed to set serial port state, error: " + std::to_string(GetLastError()));
        CloseHandle(serialHandle);
        serialHandle = INVALID_HANDLE_VALUE;
        return false;
    }
    
    // Set timeouts
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    
    if (!SetCommTimeouts(serialHandle, &timeouts)) {
        LOG_ERROR("Failed to set serial timeouts, error: " + std::to_string(GetLastError()));
        CloseHandle(serialHandle);
        serialHandle = INVALID_HANDLE_VALUE;
        return false;
    }
    
    connected = true;
    LOG_MAIN("Successfully connected to " + portName);
    return true;
}

void SerialCommunication::disconnect() {
    if (serialHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(serialHandle);
        serialHandle = INVALID_HANDLE_VALUE;
        connected = false;
        LOG_INFO("Disconnected from serial port: " + portName);
    }
}

bool SerialCommunication::sendCommand(const std::string& cmd) {
    if (!connected) {
        LOG_ERROR("Cannot send command - not connected to serial port");
        return false;
    }
    
    DWORD bytesWritten = 0;
    std::string cmdWithNewline = cmd + "\r\n";
    
    LOG_DEBUG("Sending command: " + cmd);
    
    if (!WriteFile(serialHandle, cmdWithNewline.c_str(), cmdWithNewline.size(), &bytesWritten, NULL)) {
        LOG_ERROR("Failed to write to serial port, error: " + std::to_string(GetLastError()));
        return false;
    }
    
    if (bytesWritten != cmdWithNewline.size()) {
        LOG_WARNING("Incomplete write to serial port");
    }
    
    return true;
}

bool SerialCommunication::sendPattern(const std::vector<int>& pattern) {
    std::stringstream ss;
    ss << "PATTERN:";
    
    for (size_t i = 0; i < pattern.size(); ++i) {
        ss << pattern[i];
        if (i < pattern.size() - 1) {
            ss << ",";
        }
    }
    
    return sendCommand(ss.str());
}

std::string SerialCommunication::receiveResponse(int timeout) {
    if (!connected) {
        LOG_ERROR("Cannot receive response - not connected to serial port");
        return "";
    }
    
    char buffer[256] = {0};
    DWORD bytesRead = 0;
    std::string response;
    DWORD startTime = GetTickCount();
    
    while (GetTickCount() - startTime < (DWORD)timeout) {
        if (!ReadFile(serialHandle, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
            LOG_ERROR("Failed to read from serial port, error: " + std::to_string(GetLastError()));
            return "";
        }
        
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            response += buffer;
            
            // Check if we have a complete response
            if (response.find("\n") != std::string::npos) {
                break;
            }
        }
        
        // Short sleep to prevent CPU thrashing
        Sleep(10);
    }
    
    LOG_DEBUG("Received response: " + response);
    return response;
}

bool SerialCommunication::verifyPatternCompleted() {
    sendCommand("CHECK");
    std::string response = receiveResponse();
    
    // Trim whitespace and newlines
    response.erase(0, response.find_first_not_of(" \t\r\n"));
    response.erase(response.find_last_not_of(" \t\r\n") + 1);
    
    LOG_INFO("Pattern verification response: " + response);
    
    return response == "SUCCESS";
}