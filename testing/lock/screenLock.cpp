#include <Windows.h>    // For Windows-specific APIs (BlockInput, console functions)
#include <iostream>     // For console output (cout, endl)
#include <thread>       // For thread sleep functionality
#include <chrono>       // For time units (seconds)
#include "screenLock.h" // Include our library header

// Implementation of the screen lock function to be exported
// This function will:
// 1. Create a console window to show status
// 2. Block all keyboard and mouse input
// 3. Show a countdown for the specified number of seconds
// 4. Restore normal input
// 5. Return 1 for success, 0 for failure
extern "C" __declspec(dllexport) int lock_screen_for_seconds(int seconds) {
    // Validate input - ensure we don't lock for too long or with negative values
    if (seconds <= 0) {
        return 0;  // Invalid duration
    }
    
    if (seconds > 3600) {  // Limit to 1 hour for safety
        seconds = 3600;
    }
    
    // Create console window to show status
    // AllocConsole creates a new console for the current process
    AllocConsole();
    FILE* stream;
    // Redirect stdout to the console
    freopen_s(&stream, "CONOUT$", "w", stdout);
    
    std::cout << "=== SCREEN LOCK ACTIVATED ===" << std::endl;
    std::cout << "All input will be blocked for " << seconds << " seconds" << std::endl;
    
    // Block input (keyboard and mouse)
    // BlockInput(TRUE) prevents keyboard and mouse input from reaching applications
    // NOTE: Requires admin privileges to work
    if (!BlockInput(TRUE)) {
        std::cout << "Failed to lock input. Error code: " << GetLastError() << std::endl;
        BlockInput(FALSE); // Make sure input is restored in case of failure
        FreeConsole();
        return 0;
    }
    
    // Countdown timer
    // Loop for the specified number of seconds, updating the console each second
    for (int i = seconds; i > 0; i--) {
        std::cout << "Seconds remaining: " << i << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    // Unblock input - restore normal functionality
    BlockInput(FALSE);
    
    std::cout << "=== SCREEN LOCK DEACTIVATED ===" << std::endl;
    std::cout << "Input control returned" << std::endl;
    std::cout << "Press any key to close this window..." << std::endl;
    
    // Wait for a keypress before closing
    std::cin.get();
    
    // Clean up console when done
    FreeConsole();
    return 1;
}

// Function to check if the current process has administrator privileges
// This is important because BlockInput requires admin rights to function
extern "C" __declspec(dllexport) int has_admin_privileges() {
    BOOL isAdmin = FALSE;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    PSID adminGroup;
    
    // Attempts to create a SID for the administrator group
    isAdmin = AllocateAndInitializeSid(
        &ntAuthority,
        2,  // Two subauthorities
        SECURITY_BUILTIN_DOMAIN_RID,   // First subauthority
        DOMAIN_ALIAS_RID_ADMINS,       // Second subauthority (Administrators group)
        0, 0, 0, 0, 0, 0,              // Remaining subauthorities (unused)
        &adminGroup);
    
    if (isAdmin) {
        // Check if the current process token is a member of the admin group
        if (!CheckTokenMembership(NULL, adminGroup, &isAdmin)) {
            isAdmin = FALSE;
        }
        // Free the SID when done
        FreeSid(adminGroup);
    }
    
    // Return 1 for admin, 0 for non-admin
    return isAdmin ? 1 : 0;
}