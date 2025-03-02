#pragma once

// This header defines the public API for the screen lock library
// The functions are declared with extern "C" to prevent C++ name mangling
// allowing them to be called from C code or FFI interfaces

#ifdef __cplusplus
extern "C" {  // Use C linkage when compiled with C++
#endif

// Function that locks the screen for a specified number of seconds
// @param seconds - Number of seconds to lock the screen
// This function blocks all keyboard and mouse input using Windows BlockInput API
// Displays a countdown in a console window
// Returns 1 for success, 0 for failure
// NOTE: Requires administrator privileges to work properly
__declspec(dllexport) int lock_screen_for_seconds(int seconds);

// Function to check if admin privileges are available
// BlockInput API requires administrator privileges to work
// Returns 1 if the current process has admin privileges, 0 if not
__declspec(dllexport) int has_admin_privileges();

#ifdef __cplusplus
}  // End of extern "C" block
#endif