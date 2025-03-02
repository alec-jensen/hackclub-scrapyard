#pragma once
#include "Logger.hpp"
#include <windows.h>
#include <vector>
#include <atomic>
#include <thread>
#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <mutex>

class KeyboardMiddleware {
    struct KeyConfig {
        WORD key;
        int targetCounter;
        KeyConfig(WORD k, int t) : key(k), targetCounter(t) {}
    };

private:
    static std::atomic<bool> blockKeys;
    static HHOOK keyboardHook;
    static std::vector<KeyConfig> keyConfigs;
    static bool shouldExit;
    static std::function<void(int)> sendToHardwareCallback;
    static std::function<bool()> receiveFromHardwareCallback;
    static std::atomic<int> targetCounter;
    static std::mutex counterMutex;

    static void LogMessage(const std::string& message);
    static void SendResponseToApplication(WORD key);
    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

public:
    static bool Initialize();
    static void RegisterKey(WORD key, int targetCount);
    static void SetTargetCounter(int counter);
    static void RegisterHardwareCallbacks(
        std::function<void(int)> sendCallback,
        std::function<bool()> receiveCallback
    );
    static void Cleanup();
};