#include "middleWhere.hpp"
#include "Logger.hpp"
#include <iostream>
#include <chrono>
#include <thread>

// Mock hardware simulation
class MockHardware {
public:
    static void simulateProgress(int target) {
        AllocConsole();
        freopen("CONOUT$", "w", stdout);
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

int WINAPI WinMain([[maybe_unused]] HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, 
    [[maybe_unused]] LPSTR lpCmdLine, [[maybe_unused]] int nCmdShow) {
    if (!KeyboardMiddleware::Initialize()) {
        // Using MessageBoxA for ANSI strings
        MessageBoxA(NULL, "Middleware initialization failed!", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Register each key with its own target counter
    KeyboardMiddleware::RegisterKey('W', 5);  // W key needs 5 counts
    KeyboardMiddleware::RegisterKey('A', 3);  // A key needs 3 counts
    KeyboardMiddleware::RegisterKey('S', 7);  // S key needs 7 counts
    KeyboardMiddleware::RegisterKey('D', 4);  // D key needs 4 countsy
    KeyboardMiddleware::RegisterKey(VK_SPACE, 10);  // Space key needs 10 counts

    KeyboardMiddleware::RegisterHardwareCallbacks(testSendToHardware, testReceiveFromHardware);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    KeyboardMiddleware::Cleanup();
    return 0;
}