#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use std::os::raw::{c_char, c_int};
use std::cell::RefCell;
use std::sync::{Mutex, Once};
use std::collections::HashMap;

use simon_game::ffi::simon_error_t;

pub type serial_monitor_t = *mut std::ffi::c_void;
pub type keyboard_middleware_t = *mut std::ffi::c_void;

struct MockState {
    serial_connected: bool,
    simon_game_success: bool, 
    registered_keys: HashMap<i32, i32>,
    last_simon_length: i32,
    callbacks_registered: bool,
}

static INIT: Once = Once::new();
static mut MOCK_STATE: Option<Mutex<MockState>> = None;

pub type simon_send_callback_t = Option<unsafe extern "C" fn(counter: c_int)>;
pub type simon_receive_callback_t = Option<unsafe extern "C" fn() -> c_int>;

fn get_mock_state() -> &'static Mutex<MockState> {
    unsafe {
        INIT.call_once(|| {
            MOCK_STATE = Some(Mutex::new(MockState {
                serial_connected: false,
                simon_game_success: true,
                registered_keys: HashMap::new(),
                last_simon_length: 0,
                callbacks_registered: false,
            }));
        });
        MOCK_STATE.as_ref().unwrap()
    }
}

pub fn reset_mock_state() {
    let state = get_mock_state();
    let mut state = state.lock().unwrap();
    state.serial_connected = false;
    state.simon_game_success = true;
    state.registered_keys.clear();
    state.last_simon_length = 0;
    state.callbacks_registered = false;
}

pub fn set_simon_game_success(success: bool) {
    let state = get_mock_state();
    let mut state = state.lock().unwrap();
    state.simon_game_success = success;
}
#[no_mangle]
pub unsafe extern "C" fn sm_create(_port_name: *const c_char) -> serial_monitor_t {
    1 as serial_monitor_t
}

#[no_mangle]
pub unsafe extern "C" fn sm_destroy(_handle: serial_monitor_t) {
}

#[no_mangle]
pub unsafe extern "C" fn sm_connect(_handle: serial_monitor_t) -> simon_error_t {
    let state = get_mock_state();
    let mut state = state.lock().unwrap();
    
    if _handle.is_null() {
        return simon_error_t::SIMON_ERROR_NULL_HANDLE;
    }
    
    state.serial_connected = true;
    simon_error_t::SIMON_SUCCESS
}

#[no_mangle]
pub unsafe extern "C" fn sm_disconnect(_handle: serial_monitor_t) -> simon_error_t {
    let state = get_mock_state();
    let mut state = state.lock().unwrap();
    
    if _handle.is_null() {
        return simon_error_t::SIMON_ERROR_NULL_HANDLE;
    }
    
    state.serial_connected = false;
    simon_error_t::SIMON_SUCCESS
}

#[no_mangle]
pub unsafe extern "C" fn sm_send_command(_handle: serial_monitor_t, _command: *const c_char) -> simon_error_t {
    let state = get_mock_state();
    let state = state.lock().unwrap();
    
    if _handle.is_null() {
        return simon_error_t::SIMON_ERROR_NULL_HANDLE;
    }
    
    if !state.serial_connected {
        return simon_error_t::SIMON_ERROR_CONNECTION_FAILED;
    }
    
    simon_error_t::SIMON_SUCCESS
}

#[no_mangle]
pub unsafe extern "C" fn sm_send_simon_game_length(_handle: serial_monitor_t, length: c_int) -> simon_error_t {
    let state = get_mock_state();
    let mut state = state.lock().unwrap();
    
    if _handle.is_null() {
        return simon_error_t::SIMON_ERROR_NULL_HANDLE;
    }
    
    if !state.serial_connected {
        return simon_error_t::SIMON_ERROR_CONNECTION_FAILED;
    }
    
    if length <= 0 {
        return simon_error_t::SIMON_ERROR_INVALID_PARAMETER;
    }
    
    state.last_simon_length = length;
    simon_error_t::SIMON_SUCCESS
}

#[no_mangle]
pub unsafe extern "C" fn sm_verify_simon_game_success(_handle: serial_monitor_t, _timeout_ms: c_int) -> c_int {
    let state = get_mock_state();
    let state = state.lock().unwrap();
    
    if _handle.is_null() || !state.serial_connected {
        return 0;
    }
    
    if state.simon_game_success {
        1
    } else {
        0
    }
}

#[no_mangle]
pub unsafe extern "C" fn sm_is_connected(_handle: serial_monitor_t) -> c_int {
    let state = get_mock_state();
    let state = state.lock().unwrap();
    
    if _handle.is_null() {
        return 0;
    }
    
    if state.serial_connected {
        1
    } else {
        0
    }
}

#[no_mangle]
pub unsafe extern "C" fn km_create() -> keyboard_middleware_t {
    2 as keyboard_middleware_t
}

#[no_mangle]
pub unsafe extern "C" fn km_destroy(_handle: keyboard_middleware_t) {
    // Nothing to do bru I eepy
}

#[no_mangle]
pub unsafe extern "C" fn km_initialize(_handle: keyboard_middleware_t) -> simon_error_t {
    if _handle.is_null() {
        return simon_error_t::SIMON_ERROR_NULL_HANDLE;
    }
    
    simon_error_t::SIMON_SUCCESS
}

#[no_mangle]
pub unsafe extern "C" fn km_register_key(_handle: keyboard_middleware_t, key_code: c_int, target_count: c_int) -> simon_error_t {
    let state = get_mock_state();
    let mut state = state.lock().unwrap();
    
    if _handle.is_null() {
        return simon_error_t::SIMON_ERROR_NULL_HANDLE;
    }
    
    if target_count <= 0 {
        return simon_error_t::SIMON_ERROR_INVALID_PARAMETER;
    }
    
    state.registered_keys.insert(key_code, target_count);
    simon_error_t::SIMON_SUCCESS
}

#[no_mangle]
pub unsafe extern "C" fn km_register_callbacks(
    _handle: keyboard_middleware_t,
    _send_callback: simon_send_callback_t,
    _receive_callback: simon_receive_callback_t,
) -> simon_error_t {
    let state = get_mock_state();
    let mut state = state.lock().unwrap();
    
    if _handle.is_null() {
        return simon_error_t::SIMON_ERROR_NULL_HANDLE;
    }
    
    state.callbacks_registered = true;
    simon_error_t::SIMON_SUCCESS
}

#[no_mangle]
pub unsafe extern "C" fn km_cleanup(_handle: keyboard_middleware_t) -> simon_error_t {
    let state = get_mock_state();
    let mut state = state.lock().unwrap();
    
    if _handle.is_null() {
        return simon_error_t::SIMON_ERROR_NULL_HANDLE;
    }
    
    state.registered_keys.clear();
    state.callbacks_registered = false;
    simon_error_t::SIMON_SUCCESS
}