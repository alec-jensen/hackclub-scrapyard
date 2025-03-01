#include <windows.h>
#include <vector>
#include <atomic>
#include <thread>
#include <chrono>
#include <algorithm>
#include <iostream>
#include <ctime>
#include <string>

class KeyboardBlocker {
private:
    static std::atomic<bool> blockKeys;
    static HHOOK keyboardHook;
    static std::vector<WORD> restrictedKeys;
    static const int DELAY_MS = 5000; // 5 second delay
    static bool shouldExit;

    static void LogMessage(const std::string& message) {
        time_t now = time(0);
        char* dt = ctime(&now);
        std::cout << "[" << dt << "] " << message << std::endl;
    }
	static void SendFinishedAction(WORD key) {
	}
protected:
    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
        if (nCode == HC_ACTION) {
            KBDLLHOOKSTRUCT* hookStruct = (KBDLLHOOKSTRUCT*)lParam;
            
            // Check for Ctrl+X
            if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && hookStruct->vkCode == 'X') {
                LogMessage("Ctrl+X detected - Exiting application");
                shouldExit = true;
                PostQuitMessage(0);
                return 1;
            }

            // Check if key press event
            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
                // Check if the key is in our restricted list
                if (std::find(restrictedKeys.begin(), restrictedKeys.end(), 
                    hookStruct->vkCode) != restrictedKeys.end()) {
                    
                    if (blockKeys) {
                        LogMessage("Blocking key: " + std::to_string(hookStruct->vkCode));
                        return 1; // Block the key
                    }
                    
                    // Key is allowed, trigger the delay
                    blockKeys = true;
                    LogMessage("Key pressed: " + std::to_string(hookStruct->vkCode) + " - Starting delay");
                    WORD capturedKey = hookStruct->vkCode;
                    
                    std::thread([capturedKey]{
                        Sleep(DELAY_MS);
						LogMessage("Sending Key: " + std::to_string(capturedKey));
                        SimulateKeyPress(capturedKey);
                        blockKeys = false;
                    }).detach();
                    
                    return 1; // Block the original key press
                }
            }
        }
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    }

public:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        if (uMsg == WM_DESTROY) {
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    static bool Initialize() {
        // Initialize restricted keys
        restrictedKeys = {
            'W', 'A', 'S', 'D',    // WASD keys
            VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT, // Arrow keys
            VK_SPACE,              // Spacebar
            VK_RETURN             // Enter key
        };

        blockKeys = false;
        shouldExit = false;
        
        // Create console for logging
        AllocConsole();
        FILE* dummy;
        freopen_s(&dummy, "CONOUT$", "w", stdout);
        
        LogMessage("Initializing keyboard blocker...");
        LogMessage("Press Ctrl+X to exit");
        
        keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
        return (keyboardHook != NULL);
    }

    static void Cleanup() {
        if (keyboardHook != NULL) {
            UnhookWindowsHookEx(keyboardHook);
            keyboardHook = NULL;
        }
        LogMessage("Cleaning up and exiting...");
    }
};

// Initialize static members
std::atomic<bool> KeyboardBlocker::blockKeys(false);
HHOOK KeyboardBlocker::keyboardHook = NULL;
std::vector<WORD> KeyboardBlocker::restrictedKeys;
bool KeyboardBlocker::shouldExit = false;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Register window class
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = KeyboardBlocker::WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "KeyboardBlockerClass";
    RegisterClassEx(&wc);

    // Create minimal window (can be hidden with WS_POPUP)
    HWND hwnd = CreateWindow(
        "KeyboardBlockerClass", "Key Blocker",
        WS_POPUP,
        0, 0, 1, 1,
        NULL, NULL, hInstance, NULL
    );

    if (!KeyboardBlocker::Initialize()) {
        MessageBox(NULL, "Failed to initialize keyboard blocker", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, SW_HIDE); // Hide the window

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    KeyboardBlocker::Cleanup();
    return 0;
}