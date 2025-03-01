#include "middleWhere.hpp"
#include <iostream>

// Initialize static members
std::atomic<bool> KeyboardMiddleware::blockKeys(false);
HHOOK KeyboardMiddleware::keyboardHook = NULL;
std::vector<KeyboardMiddleware::KeyConfig> KeyboardMiddleware::keyConfigs;
bool KeyboardMiddleware::shouldExit = false;
std::atomic<int> KeyboardMiddleware::targetCounter(0);
std::mutex KeyboardMiddleware::counterMutex;
std::function<void(int)> KeyboardMiddleware::sendToHardwareCallback;
std::function<bool()> KeyboardMiddleware::receiveFromHardwareCallback;

void KeyboardMiddleware::LogMessage(const std::string& message) {
    Logger::log(Logger::LogLevel::MAIN, message);
}

void KeyboardMiddleware::SendResponseToApplication(WORD key) {
    std::lock_guard<std::mutex> lock(counterMutex);
    auto keyConfig = std::find_if(keyConfigs.begin(), keyConfigs.end(),
        [key](const KeyConfig& config) { return config.key == key; });

    if (keyConfig != keyConfigs.end()) {
        if (sendToHardwareCallback) {
            sendToHardwareCallback(keyConfig->targetCounter);
        }
        
        if (receiveFromHardwareCallback && receiveFromHardwareCallback()) {
            LogMessage("Hardware verification successful");
            blockKeys = false;
        } else {
            LogMessage("Hardware verification failed");
        }
    }
}

LRESULT CALLBACK KeyboardMiddleware::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode != HC_ACTION) {
        return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
    }

    if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
        KBDLLHOOKSTRUCT* kbStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        WORD key = kbStruct->vkCode;

        auto keyConfig = std::find_if(keyConfigs.begin(), keyConfigs.end(),
            [key](const KeyConfig& config) { return config.key == key; });

        if (keyConfig != keyConfigs.end()) {
            if (blockKeys) {
                LogMessage("Key blocked: " + std::to_string(key));
                return 1;
            }

            blockKeys = true;
            std::thread([key]() {
                SendResponseToApplication(key);
            }).detach();

            return 1;
        }
    }

    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

bool KeyboardMiddleware::Initialize() {
    LogMessage("Initializing keyboard middleware...");
    
    keyboardHook = SetWindowsHookEx(
        WH_KEYBOARD_LL,
        LowLevelKeyboardProc,
        GetModuleHandle(NULL),
        0
    );

    if (!keyboardHook) {
        LogMessage("Failed to initialize keyboard hook");
        return false;
    }

    LogMessage("Keyboard middleware initialized successfully");
    return true;
}

void KeyboardMiddleware::RegisterKey(WORD key, int targetCount) {
    keyConfigs.emplace_back(key, targetCount);
    LogMessage("Registered key: " + std::to_string(key) + 
              " with target count: " + std::to_string(targetCount));
}

void KeyboardMiddleware::SetTargetCounter(int counter) {
    targetCounter.store(counter);
    LogMessage("Set target counter to: " + std::to_string(counter));
}

void KeyboardMiddleware::RegisterHardwareCallbacks(
    std::function<void(int)> sendCallback,
    std::function<bool()> receiveCallback
) {
    sendToHardwareCallback = sendCallback;
    receiveFromHardwareCallback = receiveCallback;
    LogMessage("Hardware callbacks registered");
}

void KeyboardMiddleware::Cleanup() {
    if (keyboardHook != NULL) {
        UnhookWindowsHookEx(keyboardHook);
        keyboardHook = NULL;
        LogMessage("Keyboard hook cleaned up");
    }
    shouldExit = true;
    LogMessage("Middleware cleanup complete");
}