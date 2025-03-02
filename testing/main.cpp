#include "middleWhere.hpp"
#include "Logger.hpp"
#include "SerialCommunication.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <map>

// Global serial communication object
SerialCommunication serialComm;

// Define the key to pattern mapping
std::map<WORD, std::vector<int>> keyPatterns = {
    {'W', {1, 2, 3, 2, 1}},      // Example pattern for W key
    {'A', {3, 1, 3, 1}},         // Example pattern for A key
    {'S', {2, 4, 2, 1, 3, 4}},   // Example pattern for S key
    {'D', {4, 3, 2, 1}},         // Example pattern for D key
    {VK_SPACE, {1, 2, 3, 4, 4, 3, 2, 1}} // Example pattern for SPACE key
};

void sendToHardware(int counter) {
    LOG_INFO("Sending pattern to hardware with complexity: " + std::to_string(counter));
    
    WORD lastPressedKey = 0;
    
    // Find which key was last pressed (this is a simple approach - you might need to enhance this)
    for (const auto& keyConfig : keyPatterns) {
        if (keyConfig.second.size() == counter) {
            lastPressedKey = keyConfig.first;
            break;
        }
    }
    
    // If key pattern found, send it to the hardware
    if (lastPressedKey != 0 && keyPatterns.find(lastPressedKey) != keyPatterns.end()) {
        LOG_INFO("Sending pattern for key: " + std::to_string(lastPressedKey));
        serialComm.sendPattern(keyPatterns[lastPressedKey]);
    } else {
        LOG_WARNING("No pattern defined for the pressed key with counter: " + std::to_string(counter));
    }
}

bool receiveFromHardware() {
    LOG_INFO("Waiting for hardware to verify pattern completion");
    
    // Wait a brief moment for the user to interact with the hardware
    Sleep(500);
    
    // Check if the pattern was completed successfully
    bool success = serialComm.verifyPatternCompleted();
    
    if (success) {
        LOG_MAIN("Pattern successfully completed on hardware!");
    } else {
        LOG_WARNING("Pattern verification failed or timed out");
    }
    
    return success;
}

void setupConsole() {
    AllocConsole();
    
    FILE* fpstdout = freopen("CONOUT$", "w", stdout);
    if (!fpstdout)
        MessageBoxA(NULL, "Failed to redirect stdout", "Error", MB_OK | MB_ICONERROR);
    
    FILE* fpstderr = freopen("CONOUT$", "w", stderr);
    if (!fpstderr)
        MessageBoxA(NULL, "Failed to redirect stderr", "Error", MB_OK | MB_ICONERROR);
    
    SetConsoleTitleA("Simon Game Keyboard Middleware");
    
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
            COORD bufferSize;
            bufferSize.X = csbi.dwSize.X;
            bufferSize.Y = 1000;
            SetConsoleScreenBufferSize(hConsole, bufferSize);
        }
    }
    
    LOG_INFO("Console initialized");
}

int WINAPI WinMain([[maybe_unused]] HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, 
    [[maybe_unused]] LPSTR lpCmdLine, [[maybe_unused]] int nCmdShow) {
    
    setupConsole();
    
    if (!Logger::initialize("simon_game_middleware.log", Logger::LogLevel::DEBUG)) {
        std::cerr << "Failed to initialize logger!" << std::endl;
        return 1;
    }
    
    LOG_MAIN("Simon Game Application starting...");
    
    // Try to connect to the hardware
    if (!serialComm.connect()) {
        LOG_ERROR("Failed to connect to Simon game hardware! Check connections and port settings.");
        MessageBoxA(NULL, "Failed to connect to Simon game hardware! Check connections and port settings.", 
                    "Connection Error", MB_OK | MB_ICONERROR);
        Logger::shutdown();
        return 1;
    }
    
    if (!KeyboardMiddleware::Initialize()) {
        LOG_ERROR("Middleware initialization failed!");
        MessageBoxA(NULL, "Middleware initialization failed!", "Error", MB_OK | MB_ICONERROR);
        Logger::shutdown();
        return 1;
    }

    // Register keys with their pattern complexity
    KeyboardMiddleware::RegisterKey('W', keyPatterns['W'].size()); 
    KeyboardMiddleware::RegisterKey('A', keyPatterns['A'].size()); 
    KeyboardMiddleware::RegisterKey('S', keyPatterns['S'].size());  
    KeyboardMiddleware::RegisterKey('D', keyPatterns['D'].size()); 
    KeyboardMiddleware::RegisterKey(VK_SPACE, keyPatterns[VK_SPACE].size());

    // Register our hardware callbacks
    KeyboardMiddleware::RegisterHardwareCallbacks(sendToHardware, receiveFromHardware);
    
    LOG_MAIN("Application ready - waiting for keyboard events");
    std::cout << "Press a registered key (W, A, S, D or SPACE) to send a pattern to the Simon game." << std::endl;

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    KeyboardMiddleware::Cleanup();
    serialComm.disconnect();
    
    LOG_MAIN("Application shutting down");
    Logger::shutdown();
    
    return 0;
}