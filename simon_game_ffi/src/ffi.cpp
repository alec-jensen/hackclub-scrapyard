#include "simon_game.h"
#include "SerialMonitor.hpp"
#include "middleWhere.hpp"
#include "Logger.hpp"
#include <map>
#include <functional>

// Structure to hold the actual SerialMonitor instance
struct SerialMonitorHandle {
    SerialMonitor monitor;
    
    explicit SerialMonitorHandle(const std::string& port) : monitor(port) {}
};

// Structure to hold the middleware state
struct KeyboardMiddlewareHandle {
    bool initialized;
    KeyboardMiddlewareHandle() : initialized(false) {}
};

// Store callback pointers for use in C++ context
static simon_send_callback_t g_send_callback = nullptr;
static simon_receive_callback_t g_receive_callback = nullptr;

// C++ wrapper for send callback
void cpp_send_callback_wrapper(int counter) {
    if (g_send_callback) {
        g_send_callback(counter);
    }
}

// C++ wrapper for receive callback
bool cpp_receive_callback_wrapper() {
    if (g_receive_callback) {
        return g_receive_callback() != 0;
    }
    return false;
}

// SerialMonitor implementation
extern "C" {

serial_monitor_t sm_create(const char* port_name) {
    try {
        std::string port = port_name ? port_name : "COM6";
        return new SerialMonitorHandle(port);
    } catch (...) {
        return nullptr;
    }
}

void sm_destroy(serial_monitor_t handle) {
    if (handle) {
        delete static_cast<SerialMonitorHandle*>(handle);
    }
}

simon_error_t sm_connect(serial_monitor_t handle) {
    if (!handle) return SIMON_ERROR_NULL_HANDLE;
    
    try {
        SerialMonitorHandle* h = static_cast<SerialMonitorHandle*>(handle);
        if (h->monitor.connect()) {
            return SIMON_SUCCESS;
        } else {
            return SIMON_ERROR_CONNECTION_FAILED;
        }
    } catch (...) {
        return SIMON_ERROR_UNKNOWN;
    }
}

simon_error_t sm_disconnect(serial_monitor_t handle) {
    if (!handle) return SIMON_ERROR_NULL_HANDLE;
    
    try {
        SerialMonitorHandle* h = static_cast<SerialMonitorHandle*>(handle);
        h->monitor.disconnect();
        return SIMON_SUCCESS;
    } catch (...) {
        return SIMON_ERROR_UNKNOWN;
    }
}

simon_error_t sm_send_command(serial_monitor_t handle, const char* command) {
    if (!handle) return SIMON_ERROR_NULL_HANDLE;
    if (!command) return SIMON_ERROR_INVALID_PARAMETER;
    
    try {
        SerialMonitorHandle* h = static_cast<SerialMonitorHandle*>(handle);
        if (h->monitor.sendCommand(command)) {
            return SIMON_SUCCESS;
        } else {
            return SIMON_ERROR_CONNECTION_FAILED;
        }
    } catch (...) {
        return SIMON_ERROR_UNKNOWN;
    }
}

simon_error_t sm_send_simon_game_length(serial_monitor_t handle, int length) {
    if (!handle) return SIMON_ERROR_NULL_HANDLE;
    if (length <= 0) return SIMON_ERROR_INVALID_PARAMETER;
    
    try {
        SerialMonitorHandle* h = static_cast<SerialMonitorHandle*>(handle);
        if (h->monitor.sendSimonGameLength(length)) {
            return SIMON_SUCCESS;
        } else {
            return SIMON_ERROR_CONNECTION_FAILED;
        }
    } catch (...) {
        return SIMON_ERROR_UNKNOWN;
    }
}

int sm_verify_simon_game_success(serial_monitor_t handle, int timeout_ms) {
    if (!handle) return 0;
    
    try {
        SerialMonitorHandle* h = static_cast<SerialMonitorHandle*>(handle);
        return h->monitor.verifySimonGameSuccess(timeout_ms) ? 1 : 0;
    } catch (...) {
        return 0;
    }
}

int sm_is_connected(serial_monitor_t handle) {
    if (!handle) return 0;
    
    try {
        SerialMonitorHandle* h = static_cast<SerialMonitorHandle*>(handle);
        return h->monitor.isConnected() ? 1 : 0;
    } catch (...) {
        return 0;
    }
}

// KeyboardMiddleware implementation
keyboard_middleware_t km_create() {
    try {
        return new KeyboardMiddlewareHandle();
    } catch (...) {
        return nullptr;
    }
}

void km_destroy(keyboard_middleware_t handle) {
    if (handle) {
        delete static_cast<KeyboardMiddlewareHandle*>(handle);
    }
}

simon_error_t km_initialize(keyboard_middleware_t handle) {
    if (!handle) return SIMON_ERROR_NULL_HANDLE;
    
    try {
        KeyboardMiddlewareHandle* h = static_cast<KeyboardMiddlewareHandle*>(handle);
        if (KeyboardMiddleware::Initialize()) {
            h->initialized = true;
            return SIMON_SUCCESS;
        } else {
            return SIMON_ERROR_HOOK_FAILED;
        }
    } catch (...) {
        return SIMON_ERROR_UNKNOWN;
    }
}

simon_error_t km_register_key(keyboard_middleware_t handle, int key_code, int target_count) {
    if (!handle) return SIMON_ERROR_NULL_HANDLE;
    KeyboardMiddlewareHandle* h = static_cast<KeyboardMiddlewareHandle*>(handle);
    
    if (!h->initialized) return SIMON_ERROR_HOOK_FAILED;
    if (target_count <= 0) return SIMON_ERROR_INVALID_PARAMETER;
    
    try {
        KeyboardMiddleware::RegisterKey(static_cast<WORD>(key_code), target_count);
        return SIMON_SUCCESS;
    } catch (...) {
        return SIMON_ERROR_UNKNOWN;
    }
}

simon_error_t km_register_callbacks(
    keyboard_middleware_t handle,
    simon_send_callback_t send_callback,
    simon_receive_callback_t receive_callback
) {
    if (!handle) return SIMON_ERROR_NULL_HANDLE;
    KeyboardMiddlewareHandle* h = static_cast<KeyboardMiddlewareHandle*>(handle);
    
    if (!h->initialized) return SIMON_ERROR_HOOK_FAILED;
    
    try {
        g_send_callback = send_callback;
        g_receive_callback = receive_callback;
        
        KeyboardMiddleware::RegisterHardwareCallbacks(
            cpp_send_callback_wrapper,
            cpp_receive_callback_wrapper
        );
        return SIMON_SUCCESS;
    } catch (...) {
        return SIMON_ERROR_UNKNOWN;
    }
}

simon_error_t km_cleanup(keyboard_middleware_t handle) {
    if (!handle) return SIMON_ERROR_NULL_HANDLE;
    KeyboardMiddlewareHandle* h = static_cast<KeyboardMiddlewareHandle*>(handle);
    
    try {
        if (h->initialized) {
            KeyboardMiddleware::Cleanup();
            h->initialized = false;
        }
        g_send_callback = nullptr;
        g_receive_callback = nullptr;
        return SIMON_SUCCESS;
    } catch (...) {
        return SIMON_ERROR_UNKNOWN;
    }
}

} // extern "C"