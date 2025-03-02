#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use std::os::raw::{c_char, c_int};

pub type serial_monitor_t = *mut std::ffi::c_void;

pub type keyboard_middleware_t = *mut std::ffi::c_void;

#[repr(C)]
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum simon_error_t {
    SIMON_SUCCESS = 0,
    SIMON_ERROR_NULL_HANDLE = -1,
    SIMON_ERROR_CONNECTION_FAILED = -2,
    SIMON_ERROR_PORT_UNAVAILABLE = -3,
    SIMON_ERROR_INVALID_PARAMETER = -4,
    SIMON_ERROR_HOOK_FAILED = -5,
    SIMON_ERROR_TIMEOUT = -6,
    SIMON_ERROR_UNKNOWN = -99,
}

pub type simon_send_callback_t = Option<unsafe extern "C" fn(counter: c_int)>;
pub type simon_receive_callback_t = Option<unsafe extern "C" fn() -> c_int>;

extern "C" {
    pub fn sm_create(port_name: *const c_char) -> serial_monitor_t;
    pub fn sm_destroy(handle: serial_monitor_t);
    pub fn sm_connect(handle: serial_monitor_t) -> simon_error_t;
    pub fn sm_disconnect(handle: serial_monitor_t) -> simon_error_t;
    pub fn sm_send_command(handle: serial_monitor_t, command: *const c_char) -> simon_error_t;
    pub fn sm_send_simon_game_length(handle: serial_monitor_t, length: c_int) -> simon_error_t;
    pub fn sm_verify_simon_game_success(handle: serial_monitor_t, timeout_ms: c_int) -> c_int;
    pub fn sm_is_connected(handle: serial_monitor_t) -> c_int;

    pub fn km_create() -> keyboard_middleware_t;
    pub fn km_destroy(handle: keyboard_middleware_t);
    pub fn km_initialize(handle: keyboard_middleware_t) -> simon_error_t;
    pub fn km_register_key(handle: keyboard_middleware_t, key_code: c_int, target_count: c_int) -> simon_error_t;
    pub fn km_cleanup(handle: keyboard_middleware_t) -> simon_error_t;
    pub fn km_register_callbacks(
        handle: keyboard_middleware_t,
        send_callback: simon_send_callback_t,
        receive_callback: simon_receive_callback_t,
    ) -> simon_error_t;
}