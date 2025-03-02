#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>

// Simple logger function
void log(const std::string& level, const std::string& message) {
    std::cout << "[" << level << "]: " << message << std::endl;
}

class SerialMonitor {
private:
    HANDLE serialHandle;
    bool connected;
    std::string portName;
    std::atomic<bool> shouldRun;
    std::thread monitorThread;

public:
    SerialMonitor(const std::string& port = "COM3") 
        : serialHandle(INVALID_HANDLE_VALUE), connected(false), portName(port), shouldRun(false) {
    }
    
    ~SerialMonitor() {
        stopMonitoring();
        disconnect();
    }
    
    bool connect() {
        log("INFO", "Attempting to connect to " + portName);
        
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
            log("ERROR", "Failed to open serial port: " + portName + ", error: " + std::to_string(GetLastError()));
            return false;
        }
        
        // Configure serial port parameters
        DCB dcbSerialParams = {0};
        dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
        
        if (!GetCommState(serialHandle, &dcbSerialParams)) {
            log("ERROR", "Failed to get serial port state");
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
            log("ERROR", "Failed to set serial port state");
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
            log("ERROR", "Failed to set serial timeouts");
            CloseHandle(serialHandle);
            serialHandle = INVALID_HANDLE_VALUE;
            return false;
        }
        
        connected = true;
        log("INFO", "Successfully connected to " + portName);
        return true;
    }
    
    void disconnect() {
        if (serialHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(serialHandle);
            serialHandle = INVALID_HANDLE_VALUE;
            connected = false;
            log("INFO", "Disconnected from serial port: " + portName);
        }
    }
    
    bool sendCommand(const std::string& cmd) {
        if (!connected) {
            log("ERROR", "Cannot send command - not connected to serial port");
            return false;
        }
        
        DWORD bytesWritten = 0;
        std::string cmdWithNewline = cmd + "\r\n";
        
        log("DEBUG", "Sending command: " + cmd);
        
        if (!WriteFile(serialHandle, cmdWithNewline.c_str(), cmdWithNewline.size(), &bytesWritten, NULL)) {
            log("ERROR", "Failed to write to serial port");
            return false;
        }
        
        return true;
    }
    
    std::string receiveData(int timeout = 1000) {
        if (!connected) {
            log("ERROR", "Cannot receive data - not connected to serial port");
            return "";
        }
        
        char buffer[256] = {0};
        DWORD bytesRead = 0;
        std::string response;
        DWORD startTime = GetTickCount();
        
        while (GetTickCount() - startTime < (DWORD)timeout) {
            if (!ReadFile(serialHandle, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
                log("ERROR", "Failed to read from serial port");
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
    
    void startMonitoring() {
        if (!connected) {
            log("ERROR", "Cannot start monitoring - not connected");
            return;
        }
        
        shouldRun = true;
        monitorThread = std::thread(&SerialMonitor::monitorTask, this);
        log("INFO", "Started serial monitoring");
    }
    
    void stopMonitoring() {
        shouldRun = false;
        if (monitorThread.joinable()) {
            monitorThread.join();
            log("INFO", "Stopped serial monitoring");
        }
    }
    
private:
    void monitorTask() {
        log("INFO", "Monitor thread started");
        while (shouldRun) {
            std::string data = receiveData(100);
            if (!data.empty()) {
                log("DATA", data);
				if (data.find("False") != std::string::npos) {
					log("FAILED", "Received 'False' from serial port");
				}
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));1
        }
        log("INFO", "Monitor thread ended");
    }
};

int main() {
    std::cout << "===== Serial Monitor Test =====\n" << std::endl;
    
    // Get port name from user
    std::string portName;
    std::cout << "Enter COM port (e.g., COM3): ";
    std::getline(std::cin, portName);
    
    if (portName.empty()) {
        portName = "COM3";  // Default
    }
    
    // Create and use serial monitor
    SerialMonitor monitor(portName);
    
    if (!monitor.connect()) {
        std::cout << "Failed to connect. Press any key to exit..." << std::endl;
        std::cin.get();
        return 1;
    }
    
    // Start monitoring immediately
    monitor.startMonitoring();
    
    std::cout << "\nConnection successful! Now monitoring " << portName << "\n" << std::endl;
    std::cout << "Any text you type will be sent to the serial port." << std::endl;
    std::cout << "Type 'exit' to quit the program." << std::endl;
    std::cout << std::endl;
    
    std::string input;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);
        
        if (input == "exit") {
            break;
        }
        else {
            monitor.sendCommand(input);
        }
    }
    
    return 0;
}