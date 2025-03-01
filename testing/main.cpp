#include "middleWhere.hpp"
#include "Logger.hpp"
#include <iostream>
#include <chrono>
#include <thread>

// Mock hardware simulation
class MockHardware {
public:
    static void simulateProgress(int target) {
        std::cout << "Target received: " << target << std::endl;
        for(int i = 0; i < target; i++) {
            std::cout << "Progress: " << i + 1 << "/" << target << "\r" << std::flush;
            Sleep(100);
        }
        std::cout << "\nComplete!" << std::endl;
    }
};

// Test callbacks
void testSendToHardware(int counter) {
    MockHardware::simulateProgress(counter);
}

bool testReceiveFromHardware() {
    std::cout << "Hardware verification complete" << std::endl;
    return true;
}

// Setup console for logging
void setupConsole() {
    // Allocate a console for this application
    AllocConsole();
    
    // Redirect standard output to console
    FILE* fpstdout = freopen("CONOUT$", "w", stdout);
    if (!fpstdout)
        MessageBoxA(NULL, "Failed to redirect stdout", "Error", MB_OK | MB_ICONERROR);
    
    // Redirect standard error to console
    FILE* fpstderr = freopen("CONOUT$", "w", stderr);
    if (!fpstderr)
        MessageBoxA(NULL, "Failed to redirect stderr", "Error", MB_OK | MB_ICONERROR);
    
    // Set console title (fixed from L"..." to "...")
    SetConsoleTitleA("Keyboard Middleware Console");
    
    // Optional: Set console buffer size for more visible history
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
            COORD bufferSize;
            bufferSize.X = csbi.dwSize.X;
            bufferSize.Y = 1000; // Larger scrollback buffer
            SetConsoleScreenBufferSize(hConsole, bufferSize);
        }
    }
    
    std::cout << "Console initialized" << std::endl;
}

int WINAPI WinMain([[maybe_unused]] HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, 
    [[maybe_unused]] LPSTR lpCmdLine, [[maybe_unused]] int nCmdShow) {
    
    // Setup console at application start
    setupConsole();
    
    Logger::log(Logger::LogLevel::MAIN, "Application starting...");
    
    if (!KeyboardMiddleware::Initialize()) {
        Logger::log(Logger::LogLevel::ERROR_LEVEL, "Middleware initialization failed!");
        MessageBoxA(NULL, "Middleware initialization failed!", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Register each key with its own target counter
    KeyboardMiddleware::RegisterKey('W', 5);  // W key needs 5 counts
    KeyboardMiddleware::RegisterKey('A', 3);  // A key needs 3 counts
    KeyboardMiddleware::RegisterKey('S', 7);  // S key needs 7 counts
    KeyboardMiddleware::RegisterKey('D', 4);  // D key needs 4 counts
    KeyboardMiddleware::RegisterKey(VK_SPACE, 10);  // Space key needs 10 counts

    KeyboardMiddleware::RegisterHardwareCallbacks(testSendToHardware, testReceiveFromHardware);
    
    Logger::log(Logger::LogLevel::MAIN, "Application ready - waiting for keyboard events");

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    KeyboardMiddleware::Cleanup();
    return 0;
}