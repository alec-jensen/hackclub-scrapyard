#ifndef SIMON_GAME_H
#define SIMON_GAME_H

#ifdef __cplusplus
extern "C" {
#endif

// Opaque handle types
typedef struct SerialMonitorHandle* serial_monitor_t;
typedef struct KeyboardMiddlewareHandle* keyboard_middleware_t;

// Error codes
typedef enum {
    SIMON_SUCCESS = 0,
    SIMON_ERROR_NULL_HANDLE = -1,
    SIMON_ERROR_CONNECTION_FAILED = -2,
    SIMON_ERROR_PORT_UNAVAILABLE = -3,
    SIMON_ERROR_INVALID_PARAMETER = -4,
    SIMON_ERROR_HOOK_FAILED = -5,
    SIMON_ERROR_TIMEOUT = -6,
    SIMON_ERROR_UNKNOWN = -99
} simon_error_t;

// SerialMonitor functions
serial_monitor_t sm_create(const char* port_name);
void sm_destroy(serial_monitor_t handle);
simon_error_t sm_connect(serial_monitor_t handle);
simon_error_t sm_disconnect(serial_monitor_t handle);
simon_error_t sm_send_command(serial_monitor_t handle, const char* command);
simon_error_t sm_send_simon_game_length(serial_monitor_t handle, int length);
int sm_verify_simon_game_success(serial_monitor_t handle, int timeout_ms);
int sm_is_connected(serial_monitor_t handle);

// KeyboardMiddleware functions
keyboard_middleware_t km_create();
void km_destroy(keyboard_middleware_t handle);
simon_error_t km_initialize(keyboard_middleware_t handle);
simon_error_t km_register_key(keyboard_middleware_t handle, int key_code, int target_count);
simon_error_t km_cleanup(keyboard_middleware_t handle);

// Callback type definitions
typedef void (*simon_send_callback_t)(int counter);
typedef int (*simon_receive_callback_t)(void);

// Register callbacks
simon_error_t km_register_callbacks(
    keyboard_middleware_t handle,
    simon_send_callback_t send_callback,
    simon_receive_callback_t receive_callback
);

#ifdef __cplusplus
}
#endif

#endif // SIMON_GAME_H