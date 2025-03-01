#include <windows.h>
#include <vector>
#include <atomic>
#include <thread>
#include <functional>
#include <memory>
#include <queue>
#include <mutex>

/**
 * @brief Middleware class that handles keyboard interception and hardware communication
 * This class acts as a bridge between the main application and hardware device
 */
class KeyboardMiddleware {
private:
    // Core state management
    static std::atomic<bool> blockKeys;
    static HHOOK keyboardHook;
    static std::vector<WORD> restrictedKeys;
    static bool shouldExit;
    
    // Hardware interaction callbacks
    static std::function<void(int)> sendToHardwareCallback;
    static std::function<bool()> receiveFromHardwareCallback;
    
    // Target counter management
    static std::atomic<int> targetCounter;
    static std::mutex counterMutex;

    /**
     * @brief Logs messages with timestamps (can be replaced with custom logging)
     * @param message The message to log
     */
    static void LogMessage(const std::string& message);

protected:
    /**
     * @brief Windows keyboard hook procedure
     * Intercepts keyboard events and processes them based on restrictions
     */
    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
        if (nCode != HC_ACTION) return CallNextHookEx(NULL, nCode, wParam, lParam);

        KBDLLHOOKSTRUCT* hookStruct = (KBDLLHOOKSTRUCT*)lParam;
        
        // Handle key press events
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            if (std::find(restrictedKeys.begin(), restrictedKeys.end(), 
                hookStruct->vkCode) != restrictedKeys.end()) {
                
                if (blockKeys) {
                    LogMessage("Blocking key: " + std::to_string(hookStruct->vkCode));
                    return 1;
                }
                
                // Capture key and process through hardware
                blockKeys = true;
                WORD capturedKey = hookStruct->vkCode;
                
                std::thread([capturedKey]{
                    // Send counter to hardware and wait for response
                    if (sendToHardwareCallback) {
                        int currentTarget;
                        {
                            std::lock_guard<std::mutex> lock(counterMutex);
                            currentTarget = targetCounter;
                        }
                        
                        sendToHardwareCallback(currentTarget);
                        
                        // Wait for hardware response
                        if (receiveFromHardwareCallback && receiveFromHardwareCallback()) {
                            SimulateKeyPress(capturedKey);
                        }
                    }
                    
                    blockKeys = false;
                }).detach();
                
                return 1; // Block original key press
            }
        }
        
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    }

public:
    /**
     * @brief Initializes the middleware with specific keys to block
     * @param keys Vector of virtual key codes to restrict
     * @return true if initialization successful
     */
    static bool Initialize(const std::vector<WORD>& keys = {}) {
        restrictedKeys = keys;
        blockKeys = false;
        shouldExit = false;
        keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
        return (keyboardHook != NULL);
    }

    /**
     * @brief Sets the target counter for hardware verification
     * @param counter The target value hardware needs to reach
     */
    static void SetTargetCounter(int counter) {
        std::lock_guard<std::mutex> lock(counterMutex);
        targetCounter = counter;
    }

    /**
     * @brief Registers callbacks for hardware communication
     * @param sendCallback Function to send data to hardware
     * @param receiveCallback Function to receive response from hardware
     */
    static void RegisterHardwareCallbacks(
        std::function<void(int)> sendCallback,
        std::function<bool()> receiveCallback
    ) {
        sendToHardwareCallback = sendCallback;
        receiveFromHardwareCallback = receiveCallback;
    }

    /**
     * @brief Cleanup resources and shutdown middleware
     */
    static void Cleanup() {
        if (keyboardHook != NULL) {
            UnhookWindowsHookEx(keyboardHook);
            keyboardHook = NULL;
        }
    }
};

// Initialize static members
std::atomic<bool> KeyboardMiddleware::blockKeys(false);
HHOOK KeyboardMiddleware::keyboardHook = NULL;
std::vector<WORD> KeyboardMiddleware::restrictedKeys;
bool KeyboardMiddleware::shouldExit = false;
std::atomic<int> KeyboardMiddleware::targetCounter(0);
std::mutex KeyboardMiddleware::counterMutex;
std::function<void(int)> KeyboardMiddleware::sendToHardwareCallback;
std::function<bool()> KeyboardMiddleware::receiveFromHardwareCallback;