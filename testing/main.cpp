// Example usage in main application
#include "KeyboardMiddleware.h"

// Define your hardware communication functions
void SendToHardware(int counter) {
    // Implement your hardware communication here
}

bool ReceiveFromHardware() {
    // Implement your hardware response handling here
    return true; // or false based on hardware response
}

int main() {
    // Initialize with restricted keys
    std::vector<WORD> restrictedKeys = {'W', 'A', 'S', 'D', VK_SPACE};
    KeyboardMiddleware::Initialize(restrictedKeys);

    // Register hardware callbacks
    KeyboardMiddleware::RegisterHardwareCallbacks(SendToHardware, ReceiveFromHardware);

    // Set initial target counter
    KeyboardMiddleware::SetTargetCounter(100);

    // Your main application loop here
    // ...

    // Cleanup on exit
    KeyboardMiddleware::Cleanup();
    return 0;
}