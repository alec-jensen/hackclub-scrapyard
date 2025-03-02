#include "middleWhere.hpp"
#include "Logger.hpp"
#include <iostream>
#include <chrono>
#include <thread>

class MockHardware {
public:
    static void simulateProgress(int target) {
        LOG_DEBUG("Starting hardware simulation with target: " + std::to_string(target));
        for(int i = 0; i < target; i++) {
            std::cout << "Progress: " << i + 1 << "/" << target << "\r" << std::flush;
            Sleep(100);
        }
        LOG_DEBUG("Hardware simulation complete for target: " + std::to_string(target));
        std::cout << "\nComplete!" << std::endl;
    }
};

void testSendToHardware(int counter) {
    LOG_INFO("Sending counter to hardware: " + std::to_string(counter));
    MockHardware::simulateProgress(counter);
}

bool testReceiveFromHardware() {
    LOG_INFO("Receiving verification from hardware");
    std::cout << "Hardware verification complete" << std::endl;
    return true;
}

void setupConsole() {
    AllocConsole();
    
    FILE* fpstdout = freopen("CONOUT$", "w", stdout);
    if (!fpstdout)
        MessageBoxA(NULL, "Failed to redirect stdout", "Error", MB_OK | MB_ICONERROR);
    
    FILE* fpstderr = freopen("CONOUT$", "w", stderr);
    if (!fpstderr)
        MessageBoxA(NULL, "Failed to redirect stderr", "Error", MB_OK | MB_ICONERROR);
    
    SetConsoleTitleA("Keyboard Middleware Console");
    
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
    

    if (!Logger::initialize("keyboard_middleware.log", Logger::LogLevel::DEBUG)) {
        std::cerr << "Failed to initialize logger!" << std::endl;
    } else {
        LOG_INFO("Logger initialized successfully");
    }
    
    LOG_MAIN("Application starting...");
    
    if (!KeyboardMiddleware::Initialize()) {
        LOG_ERROR("Middleware initialization failed!");
        MessageBoxA(NULL, "Middleware initialization failed!", "Error", MB_OK | MB_ICONERROR);
        Logger::shutdown();
        return 1;
    }

    KeyboardMiddleware::RegisterKey('W', 5); 
    KeyboardMiddleware::RegisterKey('A', 3); 
    KeyboardMiddleware::RegisterKey('S', 7);  
    KeyboardMiddleware::RegisterKey('D', 4); 
    KeyboardMiddleware::RegisterKey(VK_SPACE, 10);

    KeyboardMiddleware::RegisterHardwareCallbacks(testSendToHardware, testReceiveFromHardware);
    
    LOG_MAIN("Application ready - waiting for keyboard events");

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    KeyboardMiddleware::Cleanup();
    
    LOG_MAIN("Application shutting down");
    Logger::shutdown();
    
    return 0;
}