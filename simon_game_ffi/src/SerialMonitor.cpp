#include "SerialMonitor.hpp"
#include <iostream>
#include <sstream>

SerialMonitor::SerialMonitor(const std::string& port) 
    : serialHandle(INVALID_HANDLE_VALUE), connected(false), portName(port), shouldRun(false) {
}

SerialMonitor::~SerialMonitor() {
    stopMonitoring();
    disconnect();
}

void SerialMonitor::logMessage(const std::string& level, const std::string& message) {
    if (level == "ERROR") {
        LOG_ERROR(message);
    } else if (level == "WARNING") {
        LOG_WARNING(message);
    } else if (level == "DEBUG") {
        LOG_DEBUG(message);
    } else {
        LOG_INFO(message);
    }
}

bool SerialMonitor::connect() {
    logMessage("INFO", "Attempting to connect to " + portName);
    
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
        logMessage("ERROR", "Failed to open serial port: " + portName + 
                  ", error: " + std::to_string(GetLastError()));
        return false;
    }
    
    // Configure serial port parameters
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    
    if (!GetCommState(serialHandle, &dcbSerialParams)) {
        logMessage("ERROR", "Failed to get serial port state");
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
        logMessage("ERROR", "Failed to set serial port state");
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
        logMessage("ERROR", "Failed to set serial timeouts");
        CloseHandle(serialHandle);
        serialHandle = INVALID_HANDLE_VALUE;
        return false;
    }
    
    connected = true;
    logMessage("INFO", "Successfully connected to " + portName);
    return true;
}

void SerialMonitor::disconnect() {
    if (serialHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(serialHandle);
        serialHandle = INVALID_HANDLE_VALUE;
        connected = false;
        logMessage("INFO", "Disconnected from serial port: " + portName);
    }
}

bool SerialMonitor::sendCommand(const std::string& cmd) {
    if (!connected) {
        logMessage("ERROR", "Cannot send command - not connected to serial port");
        return false;
    }
    
    DWORD bytesWritten = 0;
    std::string cmdWithNewline = cmd + "\r\n";
    
    logMessage("DEBUG", "Sending command: " + cmd);
    
    if (!WriteFile(serialHandle, cmdWithNewline.c_str(), cmdWithNewline.size(), &bytesWritten, NULL)) {
        logMessage("ERROR", "Failed to write to serial port");
        return false;
    }
    
    return bytesWritten == cmdWithNewline.size();
}

std::string SerialMonitor::receiveData(int timeout) {
    if (!connected) {
        logMessage("ERROR", "Cannot receive data - not connected to serial port");
        return "";
    }
    
    char buffer[256] = {0};
    DWORD bytesRead = 0;
    std::string response;
    DWORD startTime = GetTickCount();
    
    while (GetTickCount() - startTime < (DWORD)timeout) {
        if (!ReadFile(serialHandle, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
            logMessage("ERROR", "Failed to read from serial port");
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
    
    return response;
}

bool SerialMonitor::sendSimonGameLength(int length) {
    std::string lengthStr = std::to_string(length);
    return sendCommand(lengthStr);
}

bool SerialMonitor::verifySimonGameSuccess(int timeout) {
    std::string response = receiveData(timeout);
    
    // Look for "True" in the response
    size_t truePos = response.find("True");
    if (truePos != std::string::npos) {
        logMessage("INFO", "Simon game completed successfully");
        return true;
    }
    
    logMessage("WARNING", "Simon game failed or timed out");
    return false;
}

void SerialMonitor::startMonitoring(std::function<void(const std::string&)> callback) {
    if (!connected) {
        logMessage("ERROR", "Cannot start monitoring - not connected");
        return;
    }
    
    dataCallback = callback;
    shouldRun = true;
    monitorThread = std::thread(&SerialMonitor::monitorTask, this);
    logMessage("INFO", "Started serial monitoring");
}

void SerialMonitor::stopMonitoring() {
    shouldRun = false;
    if (monitorThread.joinable()) {
        monitorThread.join();
        logMessage("INFO", "Stopped serial monitoring");
    }
}

void SerialMonitor::monitorTask() {
    logMessage("INFO", "Monitor thread started");
    while (shouldRun) {
        std::string data = receiveData(100);
        if (!data.empty()) {
            logMessage("DATA", data);
            
            if (dataCallback) {
                dataCallback(data);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    logMessage("INFO", "Monitor thread ended");
}